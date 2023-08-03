#pragma once

#include <map>
#include <utility>
#include "Camera.h"
#include "Shader.h"
#include "Light.h"
#include "GLFW/glfw3.h"
#include "Visual/VisualGrid.h"
#include "Visual/VisualLine.h"
#include "Visual/VisualCube.h"
#include "Visual/VisualSphere.h"
#include "Visual/VisualPlane.h"
#include "Screen.h"


class Renderer
{
private:
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;

        glm::vec3 target;

        Transform() = default;
        Transform(glm::vec3 _position, glm::vec3 _rotation, glm::vec3 _scale, glm::vec3 _target = glm::vec3(0.0f)) : position(_position), rotation(_rotation), scale(_scale), target(_target) {}
    };

    std::unique_ptr<Screen> main_screen;
    std::shared_ptr<Camera> main_camera;
    std::unique_ptr<Shader::Material> shadow_mapper_material;

    std::unique_ptr<VisualGrid> main_grid;

    std::unique_ptr<VisualLine> main_x_line;
    std::unique_ptr<VisualLine> main_y_line;
    std::unique_ptr<VisualLine> main_z_line;

    std::shared_ptr<std::vector<Light>> lights;
    std::unique_ptr<VisualCube> main_light_cube;
    std::unique_ptr<VisualCube> world_cube;
    std::unique_ptr<VisualPlane> ground_plane;

    std::unique_ptr<VisualSphere> tennis_ball;

    std::vector<VisualCube> net_cubes;

    std::vector<VisualCube> letter_cubes;

    std::shared_ptr<VisualCube> augusto_racket_cube;
    std::vector<Shader::Material> augusto_racket_materials;

    std::vector<Transform> rackets;
    std::vector<Transform> default_rackets;

    std::vector<Transform> cameras;

    int viewport_width, viewport_height;

    bool shadow_mode = true;
    bool light_mode = true;
    int racket_render_mode = GL_TRIANGLES;
    int selected_player = 2;

    GLuint shadow_map_fbo = 0;
    GLuint shadow_map_texture = 0;

public:
    Renderer(int _initialWidth, int _initialHeight);

    void Init();
    void Render(GLFWwindow *_window, double _deltaTime);

    void DrawOneNet(const glm::vec3 &_position, const glm::vec3 &_rotation, const glm::vec3 &_scale, const glm::mat4& _viewProjection, const glm::vec3& _eyePosition, const Shader::Material *_materialOverride = nullptr);
    void DrawOneRacket(const glm::vec3 &_position, const glm::vec3 &_rotation, const glm::vec3 &_scale, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride = nullptr);

    void DrawOneA(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride = nullptr);

    void ResizeCallback(GLFWwindow *_window, int _displayWidth, int _displayHeight);
    void InputCallback(GLFWwindow *_window, double _deltaTime);
};
