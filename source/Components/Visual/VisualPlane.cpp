#include "VisualPlane.h"

#include <utility>
#include "Utility/Transform.hpp"

VisualPlane::VisualPlane(glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, Shader::Material _material) : VisualObject(_position, _rotation, _scale, std::move(_material))
{
    // quad vertices with their uvs
    vertices = {
        -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };

    indices = {
        0, 1, 2,
        0, 2, 3
    };

    VisualObject::SetupGlBuffersVerticesNormalsUvsWithIndices();
}

void VisualPlane::Draw(const glm::mat4 &_viewProjection, const glm::vec3 &_cameraPosition, int _renderMode, const Shader::Material *material)
{
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    model_matrix = Transforms::RotateDegrees(model_matrix, rotation);
    model_matrix = glm::scale(model_matrix, scale);

    DrawFromMatrix(_viewProjection, _cameraPosition, model_matrix, _renderMode, material);
}

void VisualPlane::DrawFromMatrix(const glm::mat4 &_viewProjection, const glm::vec3 &_cameraPosition, const glm::mat4 &_transformMatrix, int _renderMode, const Shader::Material *_material)
{
    // bind the vertex array to draw
    glBindVertexArray(vertex_array_o);

    const Shader::Material *current_material = &material;

    // set the material to use on this frame
    if (_material != nullptr)
        current_material = _material;

    current_material->shader->Use();
    current_material->shader->SetModelMatrix(_transformMatrix);
    current_material->shader->SetViewProjectionMatrix(_viewProjection);

    current_material->shader->SetVec3("u_color", current_material->color);
    current_material->shader->SetFloat("u_alpha", current_material->alpha);

    current_material->shader->SetVec3("u_cam_pos", _cameraPosition);

    current_material->shader->SetVec3("u_light_pos", current_material->main_light->GetPosition());
    current_material->shader->SetVec3("u_light_color", current_material->main_light->GetColor());

    current_material->shader->SetFloat("u_ambient_strength", current_material->main_light->ambient_strength);
    current_material->shader->SetFloat("u_specular_strength", current_material->main_light->specular_strength);
    current_material->shader->SetInt("u_shininess", current_material->shininess);

    current_material->shader->SetFloat("u_shadows_influence", 1.0f - (float)current_material->main_light->project_shadows);
    current_material->shader->SetMat4("u_light_view_projection", current_material->main_light->GetViewProjection());
    current_material->shader->SetTexture("u_depth_texture", 0);

    current_material->texture->Use(GL_TEXTURE1);

    current_material->shader->SetFloatFast("u_texture_influence", current_material->texture_influence);
    current_material->shader->SetTexture("u_texture", 1);

    glLineWidth(current_material->line_thickness);
    glPointSize(current_material->point_size);

    // draw vertices according to their indices
    glDrawElements(_renderMode, indices.size(), GL_UNSIGNED_INT, nullptr);
    current_material->texture->Clear();
}
