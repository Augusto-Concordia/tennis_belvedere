#include "Renderer.h"
#include "Utility/Input.hpp"
#include "Utility/Transform.hpp"

Renderer::Renderer(int _initialWidth, int _initialHeight)
{
    viewport_width = _initialWidth;
    viewport_height = _initialHeight;

    main_camera = std::make_unique<Camera>(glm::vec3(0.0f, 35.0f, 35.0f), glm::vec3(0.0f), viewport_width, viewport_height);

    lights = std::make_shared<std::vector<Light>>();
    lights->emplace_back(glm::vec3(2.0f, 14.0f, 2.0f), glm::vec3(0.99f, 0.95f, 0.78f), 0.1f, 0.4f, 100.0f, 50.0f, Light::Type::POINT);
    lights->emplace_back(glm::vec3(30.0f, 10.0f, 0.0f), glm::vec3(0.09f, 0.95f, 0.08f), 0.2f, 0.4f, 300.0f, 50.0f, Light::Type::SPOT);
    lights->emplace_back(glm::vec3(-30.0f, 10.0f, 0.0f), glm::vec3(0.99f, 0.05f, 0.08f), 0.2f, 0.4f, 300.0f, 50.0f, Light::Type::SPOT);
    lights->emplace_back(glm::vec3(0.0f, 34.0f, 36.0f), glm::vec3(0.09f, 0.05f, 0.78f), 0.2f, 0.4f, 400.0f, 40.0f, Light::Type::SPOT);

    auto grid_shader = Shader::Library::CreateShader("shaders/grid/grid.vert", "shaders/grid/grid.frag");
    auto unlit_shader = Shader::Library::CreateShader("shaders/unlit/unlit.vert", "shaders/unlit/unlit.frag");
    auto lit_shader = Shader::Library::CreateShader("shaders/lit/lit.vert", "shaders/lit/lit.frag");

    auto shadow_mapper_shader = Shader::Library::CreateShader("shaders/shadows/shadow_mapper.vert", "shaders/shadows/shadow_mapper.frag");
    auto screen_shader = Shader::Library::CreateShader("shaders/screen/screen.vert", "shaders/screen/screen.frag");

    shadow_mapper_material = std::make_unique<Shader::Material>();
    shadow_mapper_material->shader = shadow_mapper_shader;

    Shader::Material main_light_cube_material = {
        .shader = unlit_shader,
        .color = lights->at(0).GetColor(),
        .lights = lights,
    };
    main_light_cube = std::make_unique<VisualCube>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), main_light_cube_material);

    Shader::Material screen_material = {
        .shader = screen_shader,
    };
    main_screen = std::make_unique<Screen>(screen_material);

    // default material
    Shader::Material default_s_material = {
        .shader = lit_shader,
        .color = glm::vec3(1.0f),
        .lights = lights,
    };

    // grid
    Shader::Material grid_s_material = {
        .shader = grid_shader,
        .color = glm::vec3(0.3f),
        .alpha = 0.9f,
    };
    main_grid = std::make_unique<VisualGrid>(78, 36, 1.0f, glm::vec3(0.0f), glm::vec3(90.0f, 0.0f, 0.0f), grid_s_material);

    // axis lines
    Shader::Material x_line_s_material = {
        .shader = unlit_shader,
        .line_thickness = 3.0f,
        .color = glm::vec3(1.0f, 0.0f, 0.0f),
    };

    Shader::Material y_line_s_material = {
        .shader = unlit_shader,
        .line_thickness = 3.0f,
        .color = glm::vec3(0.0f, 1.0f, 0.0f),
    };

    Shader::Material z_line_s_material = {
        .shader = unlit_shader,
        .line_thickness = 3.0f,
        .color = glm::vec3(0.0f, 0.0f, 1.0f),
    };

    // this is a quick way to make the axis lines avoid having depth fighting issues
    main_x_line = std::make_unique<VisualLine>(glm::vec3(0.01f), glm::vec3(5.01f, 0.01f, 0.01f), x_line_s_material);
    main_y_line = std::make_unique<VisualLine>(glm::vec3(0.01f), glm::vec3(0.01f, 5.01f, 0.01f), y_line_s_material);
    main_z_line = std::make_unique<VisualLine>(glm::vec3(0.01f), glm::vec3(0.01f, 0.01f, 5.01f), z_line_s_material);

    // world cube - rgb(176, 212, 228)
    Shader::Material world_s_material = {
        .shader = unlit_shader,
        .color = glm::vec3(0.69f, 0.83f, 0.89f),
        .texture = Texture::Library::CreateTexture("assets/sky.jpg"),
        .texture_influence = 1.0f,
    };
    world_cube = std::make_unique<VisualCube>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(200.0f), glm::vec3(0.0f), world_s_material);

    Shader::Material world_t_material = {
        .shader = lit_shader,
        .lights = lights,
        .texture = Texture::Library::CreateTexture("assets/clay_texture.jpg"),
        .texture_influence = 1.0f,
        .shininess = 1,
    };

    ground_plane = std::make_unique<VisualPlane>(glm::vec3(0.0f, -0.1f, 0.0f), glm::vec3(0.0f), glm::vec3(42.0f, 20.0f, 20.0f), world_t_material);

    Shader::Material world_tennisfuzz_material = {
        .shader = lit_shader,
        .lights = lights,
        .texture = Texture::Library::CreateTexture("assets/fuzz.jpg"),
        .texture_influence = 1.0f,
        .shininess = 1,
    };

    tennis_ball = std::make_unique<VisualSphere>(1.0f, 3, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.1f), world_tennisfuzz_material);

    // cube transform point offset (i.e. to scale it from the bottom-up)
    auto bottom_y_transform_offset = glm::vec3(0.0f, 0.5f, 0.0f);

    // net
    net_cubes = std::vector<VisualCube>(3);

    Shader::Material netpost_s_material = {
        .shader = lit_shader,
        .color = glm::vec3(0.51f, 0.53f, 0.53f),
        .lights = lights,
        .texture = Texture::Library::CreateTexture("assets/metal.jpg"),
        .texture_influence = 1.0f,
        .texture_tiling = glm::vec2(1.0f, 1.0f / 8.0f),
        .shininess = 4,
    };
    net_cubes[0] = VisualCube(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), bottom_y_transform_offset, netpost_s_material); // net post

    Shader::Material net_s_material = {
        .shader = lit_shader,
        .color = glm::vec3(0.96f, 0.96f, 0.96f),
        .lights = lights,
        .shininess = 128,
    };
    net_cubes[1] = VisualCube(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), bottom_y_transform_offset, net_s_material); // net

    Shader::Material top_net_s_material = {
            .shader = lit_shader,
            .color = glm::vec3(0.96f, 0.96f, 0.96f),
            .lights = lights,
            .texture = Texture::Library::CreateTexture("assets/fabric.jpg"),
            .texture_influence = 1.0f,
            .texture_tiling = 1.0f / glm::vec2(36.0f, 0.2f),
            .shininess = 128,
    };
    net_cubes[2] = VisualCube(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), bottom_y_transform_offset, top_net_s_material); // top net

    // letters
    letter_cubes = std::vector<VisualCube>(4);

    Shader::Material a_s_material = {
        .shader = lit_shader,
        .color = glm::vec3(0.15f, 0.92f, 0.17f),
        .alpha = 0.7f,
        .lights = lights,
        .shininess = 4,
    };
    letter_cubes[0] = VisualCube(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), bottom_y_transform_offset, a_s_material); // letter a

    const auto racket_line_thickness = 2.0f;
    const auto racket_point_size = 3.0f;

    // augusto racket cube + materials
    augusto_racket_cube = std::make_shared<VisualCube>(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), bottom_y_transform_offset, default_s_material);
    augusto_racket_materials = std::vector<Shader::Material>();

    Shader::Material skin_material = {
        .shader = lit_shader,
        .line_thickness = racket_line_thickness,
        .point_size = racket_point_size,
        .color = glm::vec3(0.58f, 0.38f, 0.24f),
        .lights = lights,
        .texture = Texture::Library::CreateTexture("assets/tattoo.jpg"),
        .texture_influence = 0.9f,
        .texture_tiling = 1.0f / glm::vec2(1.0f, 5.0f),
        .shininess = 2,
    };
    augusto_racket_materials.push_back(skin_material); // skin

    Shader::Material black_metal_material = {
            .shader = lit_shader,
            .line_thickness = racket_line_thickness,
            .point_size = racket_point_size,
            .color = glm::vec3(0.2f),
            .lights = lights,
            .texture = Texture::Library::CreateTexture("assets/rust.jpg"),
            .texture_influence = 0.7f,
            .shininess = 64,
    };
    augusto_racket_materials.push_back(black_metal_material); // racket handle (black metal)

    Shader::Material blue_metal_material = {
            .shader = lit_shader,
            .line_thickness = racket_line_thickness,
            .point_size = racket_point_size,
            .color = glm::vec3(0.1f, 0.2f, 0.9f),
            .lights = lights,
            .texture = Texture::Library::CreateTexture("assets/rust.jpg"),
            .texture_influence = 0.7f,
            .shininess = 64,
    };
    augusto_racket_materials.push_back(blue_metal_material); // racket piece (blue metal)

    Shader::Material green_metal_material = {
            .shader = lit_shader,
            .line_thickness = racket_line_thickness,
            .point_size = racket_point_size,
            .color = glm::vec3(0.1f, 0.9f, 0.2f),
            .lights = lights,
            .texture = Texture::Library::CreateTexture("assets/rust.jpg"),
            .texture_influence = 0.7f,
            .shininess = 64,
    };
    augusto_racket_materials.push_back(green_metal_material); // racket piece (green metal)

    Shader::Material white_plastic_material = {
            .shader = lit_shader,
            .line_thickness = racket_line_thickness,
            .point_size = racket_point_size,
            .color = glm::vec3(0.94f),
            .alpha = 0.95f,
            .lights = lights,
            .shininess = 128,
    };
    augusto_racket_materials.push_back(white_plastic_material); // racket net (white plastic)

    // racket positions
    rackets = std::vector<Transform>(3);
    default_rackets = std::vector<Transform>(3);
    rackets[0] = default_rackets[0] = Transform(
        glm::vec3(-10.0f, 0.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.8f));

    rackets[1] = default_rackets[1] = Transform(
        glm::vec3(10.0f, 0.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.8f));

    rackets[2] = default_rackets[2] = Transform(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(0.8f));

    // camera points of view
    cameras = std::vector<Transform>(3);
    cameras[0] = Transform(
        glm::vec3(-10.0f, 10.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        rackets[1].position);

    cameras[1] = Transform(
        glm::vec3(10.0f, 10.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        rackets[0].position);

    cameras[2] = Transform(
        glm::vec3(0.0f, 35.0f, 35.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        rackets[2].position);

    main_camera->SetPosition(cameras[selected_player].position);
    main_camera->SetTarget(cameras[selected_player].target);
}

void Renderer::Init() {
    // initializes the shadow map framebuffer
    glGenFramebuffers(1, &shadow_map_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);

    // initializes the shadow map depth texture
    glGenTextures(1, &shadow_map_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_map_texture);

    // sets the texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // sets the border color to white, so that the shadow map is white outside the light's view (i.e. no shadow)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    // the following 2 blocks mitigate shadow map artifacts, coming from: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    // when sampling outside, we don't want a repeating pattern
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, Light::LIGHTMAP_SIZE, Light::LIGHTMAP_SIZE, (GLint)lights->size());

    // binds the shadow map depth texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map_texture, 0);

    // disable color draw & read buffer for this framebuffer
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    // checks if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR -> Framebuffer is not complete! (" << glCheckFramebufferStatus(GL_FRAMEBUFFER) << "): ";

        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cerr << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
                break;
            default:
                std::cerr << "Unknown error" << std::endl;
                break;
        }
    }

    // cleanup the framebuffer bind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::Render(GLFWwindow *_window, const double _deltaTime)
{
    // processes input
    InputCallback(_window, _deltaTime);

    // moves "flashlight"
    lights->at(3).SetPosition(main_camera->GetPosition() + main_camera->GetCamRight() * 2.0f);
    lights->at(3).SetTarget(main_camera->GetPosition() + main_camera->GetCamForward() * -10.0f);

    // SHADOW MAP PASS

    // binds the shadow map framebuffer and the depth texture to draw on it
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_fbo);
    glViewport(0, 0, Light::LIGHTMAP_SIZE, Light::LIGHTMAP_SIZE);

    for (int i = 0; i < lights->size(); ++i) {
        const auto& light = lights->at(i);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map_texture, 0, i);

        // clears the depth canvas to black
        glClear(GL_DEPTH_BUFFER_BIT);

        if (shadow_mode && light_mode) {
            // draws the net
            DrawOneNet(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), light.GetViewProjection(), light.GetPosition(), shadow_mapper_material.get());

            // draws the rackets
            DrawOneRacket(rackets[0].position, rackets[0].rotation, rackets[0].scale, light.GetViewProjection(), light.GetPosition(), 0, shadow_mapper_material.get());
            DrawOneRacket(rackets[1].position, rackets[1].rotation + glm::vec3(0.0f, 180.0f, 0.0f), rackets[1].scale, light.GetViewProjection(), light.GetPosition(), 1, shadow_mapper_material.get());

            ground_plane->Draw(light.GetViewProjection(), light.GetPosition(), GL_TRIANGLES, shadow_mapper_material.get());
        }
    }

    // unbind the shadow map texture & framebuffer
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // COLOR PASS

    // resets the viewport to the window size
    glViewport(0, 0, viewport_width, viewport_height);

    // activates the shadow map depth texture & binds it to the first texture unit, so that it can be used by the lit shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_map_texture);

    // clears the color & depth canvas to black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draws the world cube
    world_cube->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());

    // draws the main light cube
    /*for (const auto& light: *lights) {
        main_light_cube->position = light.GetPosition();
        main_light_cube->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());
    }*/

    // draws the main grid
    main_grid->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());

    // draws the coordinate axis
    main_x_line->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());
    main_y_line->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());
    main_z_line->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());

    // draws the net
    DrawOneNet(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), main_camera->GetViewProjection(), main_camera->GetPosition());

    // draws the rackets
    DrawOneRacket(rackets[0].position, rackets[0].rotation, rackets[0].scale, main_camera->GetViewProjection(), main_camera->GetPosition(), 0);
    DrawOneRacket(rackets[1].position, rackets[1].rotation + glm::vec3(0.0f, 180.0f, 0.0f), rackets[1].scale, main_camera->GetViewProjection(), main_camera->GetPosition(), 1);

    ground_plane->Draw(main_camera->GetViewProjection(), main_camera->GetPosition());

    // can be used for post-processing effects
    //main_screen->Draw();
}

void Renderer::DrawOneNet(const glm::vec3 &_position, const glm::vec3 &_rotation, const glm::vec3 &_scale, const glm::mat4& _viewProjection, const glm::vec3& _eyePosition, const Shader::Material *_materialOverride)
{
    glm::mat4 world_transform_matrix = glm::mat4(1.0f);
    // global transforms
    world_transform_matrix = glm::translate(world_transform_matrix, _position);
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, _rotation);
    world_transform_matrix = glm::scale(world_transform_matrix, _scale);

    auto scale_factor = glm::vec3(0.0f);

    // first net post
    scale_factor = glm::vec3(1.0f, 8.0f, 1.0f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, -18.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    net_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    // horizontal net
    auto h_net_count = 7;
    scale_factor = glm::vec3(0.2f, 36.0f, 0.2f);
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(-90.0f, 0.0f, 0.0f));

    for (int i = 0; i < h_net_count - 1; ++i)
    {
        world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, -1.0f));
        world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
        net_cubes[1].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
        world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
    }

    // first horizontal net (top)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, -1.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    net_cubes[2].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    // vertical net
    auto v_net_count = 36;
    scale_factor = glm::vec3(0.2f, (float)(h_net_count - 1), 0.2f);

    // undo any movement from the horizontal net
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, (float)(h_net_count - 1)));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(90.0f, 0.0f, 0.0f));

    // first half of the vertical net
    for (int i = 0; i < v_net_count / 2; ++i)
    {
        world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, 1.0f));
        world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
        net_cubes[1].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
        world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
    }

    // second net post
    scale_factor = glm::vec3(1.0f, 8.0f, 1.0f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, -1.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    net_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0f));

    // rest of the vertical net
    scale_factor = glm::vec3(0.2f, (float)(h_net_count - 1), 0.2f);
    for (int i = v_net_count / 2; i < v_net_count; ++i)
    {
        world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 0.0f, 1.0f));
        world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
        net_cubes[1].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
        world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
    }

    // third net post
    scale_factor = glm::vec3(1.0f, 8.0f, 1.0f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, -1.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    net_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
}

void Renderer::DrawOneRacket(const glm::vec3 &_position, const glm::vec3 &_rotation, const glm::vec3 &_scale, const glm::mat4& _viewProjection, const glm::vec3& _eyePosition, int _player, const Shader::Material *_materialOverride)
{
    glm::mat4 world_transform_matrix = glm::mat4(1.0f);
    // global transforms
    world_transform_matrix = glm::translate(world_transform_matrix, _position);
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, _rotation);
    world_transform_matrix = glm::scale(world_transform_matrix, _scale);

    glm::mat4 secondary_transform_matrix = world_transform_matrix;

    // letters
    //player's letter
    switch (_player) {
        case 0:
            DrawOneP(secondary_transform_matrix, _viewProjection, _eyePosition, _materialOverride);

            secondary_transform_matrix = glm::scale(secondary_transform_matrix, glm::vec3(0.9f));
            secondary_transform_matrix = glm::translate(secondary_transform_matrix, glm::vec3(0.0f, 2.5f, -2.0f));

            DrawOneI(secondary_transform_matrix, _viewProjection, _eyePosition, _materialOverride);
            break;
        case 1:
            secondary_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 180.0f, 0.0f));
            DrawOneN(secondary_transform_matrix, _viewProjection, _eyePosition, _materialOverride);

            secondary_transform_matrix = glm::scale(secondary_transform_matrix, glm::vec3(0.9f));
            secondary_transform_matrix = glm::translate(secondary_transform_matrix, glm::vec3(0.0f, 2.5f, -2.0f));

            DrawOneH(secondary_transform_matrix, _viewProjection, _eyePosition, _materialOverride);
            break;
    }

    // tennis ball
    glm::mat4 third_transform_matrix = world_transform_matrix;
    third_transform_matrix = glm::translate(third_transform_matrix, glm::vec3(1.0f, 14.0f, -3.0f));
    third_transform_matrix = glm::scale(third_transform_matrix, glm::vec3(0.7f));
    tennis_ball->DrawFromMatrix(_viewProjection, _eyePosition, third_transform_matrix, racket_render_mode, _materialOverride);

    // forearm (skin)
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(45.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(1.0f, 5.0f, 1.0f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                           nullptr ? &augusto_racket_materials[0] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(1.0f, 0.2f, 1.0f));

    // arm (skin)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 5.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(-45.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(1.0f, 4.0f, 1.0f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[0] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(1.0f, 0.25f, 1.0f));

    // racket handle (black plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 4.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 4.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[1] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 0.25f, 2.0f));

    // racket angled bottom left (blue plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 4.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(-60.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 2.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[2] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 0.5f, 2.0f));

    // racket vertical left (green plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 2.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(60.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 3.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[3] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 1.0f / 3.0f, 2.0f));

    // racket angled top left (blue plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 3.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(60.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 1.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[2] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 1.0f, 2.0f));

    // racket horizontal top (green plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(30.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 1.6f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[3] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 1.0f / 1.6f, 2.0f));

    // racket angled top right (blue plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.6f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(30.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 1.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[2] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 1.0f, 2.0f));

    // racket vertical right (green plastic)
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(60.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 3.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[3] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 1.0f / 3.0f, 2.0f));

    // racket horizontal bottom (blue plastic)
    auto horizontal_bottom_scale = glm::vec3(0.4f, 3.2f, 0.4f);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 3.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(90.0f, 0.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, horizontal_bottom_scale);
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[2] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / horizontal_bottom_scale);

    // racket net vertical (white plastic)

    // vertical net parts
    int number_of_same_nets_v = 4;
    auto net_first_v_translate = glm::vec3(-0.65f, 0.0f, 0.0f);
    auto net_v_translate = glm::vec3(-0.5f, 0.0f, 0.0f);
    auto net_v_scale = glm::vec3(0.1f, 3.55f, 0.1f);
    auto full_v_translate = net_first_v_translate + net_v_translate * (float)number_of_same_nets_v;

    // correct orientation for the vertical nets
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(90.0f, -90.0f, 0.0f));

    // part 1
    // done separately because it has a different offset (for aesthetic purposes)
    world_transform_matrix = glm::translate(world_transform_matrix, net_first_v_translate);
    world_transform_matrix = glm::scale(world_transform_matrix, net_v_scale);
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[4] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / net_v_scale);

    // the rest of the net parts
    for (int i = 0; i < number_of_same_nets_v; ++i)
    {
        world_transform_matrix = glm::translate(world_transform_matrix, net_v_translate);
        world_transform_matrix = glm::scale(world_transform_matrix, net_v_scale);
        augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                       nullptr ? &augusto_racket_materials[4] : _materialOverride);
        world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / net_v_scale);
    }

    // horizontal net parts
    int number_of_same_nets_h = 4;
    auto net_first_h_translate = glm::vec3(-0.6f, 0.0f, 0.0f);
    auto net_h_translate = glm::vec3(-0.5f, 0.0f, 0.0f);
    auto net_h_scale = glm::vec3(0.1f, 3.05f, 0.1f);
    auto full_h_translate = net_first_h_translate + net_h_translate * (float)number_of_same_nets_h;

    // correctly place and rotate the horizontal nets
    // the reason why it's a weird combination of y and z, is because we're always in relative space,
    // so depending on the current piece we're drawing, the orientation won't be the same
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-horizontal_bottom_scale.z - full_v_translate.y, 0.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));

    // part 1
    // done separately because it has a different offset (for aesthetic purposes)
    world_transform_matrix = glm::translate(world_transform_matrix, net_first_h_translate);
    world_transform_matrix = glm::scale(world_transform_matrix, net_h_scale);
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[4] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / net_h_scale);

    // the rest of the net parts
    for (int i = 0; i < number_of_same_nets_h; ++i)
    {
        world_transform_matrix = glm::translate(world_transform_matrix, net_h_translate);
        world_transform_matrix = glm::scale(world_transform_matrix, net_h_scale);
        augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                       nullptr ? &augusto_racket_materials[4] : _materialOverride);
        world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / net_h_scale);
    }

    // racket angled bottom right (blue plastic)
    // first we undo any transformations done for the net parts
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-full_v_translate.x, horizontal_bottom_scale.y, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 150.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(0.5f, 2.0f, 0.5f));
    augusto_racket_cube->DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, racket_render_mode, _materialOverride ==
                                                                                                                   nullptr ? &augusto_racket_materials[2] : _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(2.0f, 0.5f, 2.0f));
}

// augusto letter A
void Renderer::DrawOneA(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride)
{
    auto scale_factor = glm::vec3(0.75f, 0.75f, 0.75f); // scale for one cube

    // base transform
    // rotating first, because we want to keep the original center of rotation
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 90.0f, 0.0f));
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-scale_factor.x * 1.5f, 20.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, glm::vec3(scale_factor));

    // long left A vertical cubes
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    // short top A horizontal cubes
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(1.0f, 0.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    // long right A vertical cubes
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(1.0f, -1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, -1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, -1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, -1.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    // short middle A horizontal cubes

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-1.0f, 2.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);

    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-1.0f, 0.0f, 0.0f));
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
}

void Renderer::DrawOneP(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection, const glm::vec3& _eyePosition, const Shader::Material *_materialOverride) {
    //long P vertical
    auto scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 20.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //short top P horizontal
    scale_factor = glm::vec3(0.5f, 3.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 5.0f, 0.0f)); //translate to the end of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //short right P vertical
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 3.0f, 0.0f)); //translate to the end of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //short bottom P horizontal
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 3.0f, 0.0f)); //translate to the end of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
}

void Renderer::DrawOneI(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride) {
    //short I bottom horizontal
    auto scale_factor = glm::vec3(0.5f, 3.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-1.5f, 20.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //long I vertical
    scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 1.5f, 0.0f)); //translate to the middle of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, -90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //short I top horizontal
    scale_factor = glm::vec3(0.5f, 3.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-1.5f, 5.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
}

void Renderer::DrawOneN(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride) {
    //long N vertical
    auto scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 20.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //long N diagonal
    scale_factor = glm::vec3(0.5f, 7.07f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 5.0f, 0.0f)); //translate to the end of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, -135.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //long N vertical
    scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 7.07f, 0.0f)); //translate to the end of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 135.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
}

void Renderer::DrawOneH(glm::mat4 world_transform_matrix, const glm::mat4& _viewProjection,const glm::vec3& _eyePosition, const Shader::Material *_materialOverride) {
    //long H left vertical
    auto scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(-1.5f, 20.0f, 0.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //short H middle horizontal
    scale_factor = glm::vec3(0.5f, 3.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(0.0f, 2.5f, 0.0f)); //translate to the middle of the previous cube
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, 90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);

    //long H right vertical
    scale_factor = glm::vec3(0.5f, 5.0f, 0.5f);
    world_transform_matrix = glm::translate(world_transform_matrix, glm::vec3(2.5f, 3.0f, 0.0f));
    world_transform_matrix = Transforms::RotateDegrees(world_transform_matrix, glm::vec3(0.0f, 0.0f, -90.0f));
    world_transform_matrix = glm::scale(world_transform_matrix, scale_factor);
    letter_cubes[0].DrawFromMatrix(_viewProjection, _eyePosition, world_transform_matrix, GL_TRIANGLES, _materialOverride);
    world_transform_matrix = glm::scale(world_transform_matrix, 1.0f / scale_factor);
}

void Renderer::ResizeCallback(GLFWwindow *_window, int _displayWidth, int _displayHeight)
{
    viewport_width = _displayWidth;
    viewport_height = _displayHeight;

    main_camera->SetViewportSize((float)viewport_width, (float)viewport_height);
}

void Renderer::InputCallback(GLFWwindow *_window, const double _deltaTime)
{
    // exit
    if (Input::IsKeyReleased(_window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(_window, true);
    }

    if (Input::IsKeyPressed(_window, GLFW_KEY_1))
    {
        selected_player = 0;

        main_camera->SetPosition(cameras[selected_player].position);
        main_camera->SetTarget(cameras[selected_player].target);
    }
    else if (Input::IsKeyPressed(_window, GLFW_KEY_2))
    {
        selected_player = 1;

        main_camera->SetPosition(cameras[selected_player].position);
        main_camera->SetTarget(cameras[selected_player].target);
    }
    else if (Input::IsKeyPressed(_window, GLFW_KEY_3))
    {
        selected_player = 2;

        main_camera->SetPosition(cameras[selected_player].position);
        main_camera->SetTarget(cameras[selected_player].target);
    }

    // loop through the camera positions
    if (Input::IsKeyReleased(_window, GLFW_KEY_M))
    {
        selected_player = (selected_player + 1) % 3;

        main_camera->SetPosition(cameras[selected_player].position);
        main_camera->SetTarget(cameras[selected_player].target);
    }

    // reset the camera position
    if (Input::IsKeyPressed(_window, GLFW_KEY_R))
    {
        selected_player = 2;

        main_camera->SetPosition(cameras[selected_player].position);
        main_camera->SetTarget(cameras[selected_player].target);
    }

    if (Input::IsKeyReleased(_window, GLFW_KEY_L)) {
        light_mode = !light_mode;

        for (auto& light: *lights) {
            light.SetRange(light_mode ? 300.0f : 0.0f);
        }
    }

    // keyboard triggers
    //shadows
    if (Input::IsKeyReleased(_window, GLFW_KEY_B))
    {
        shadow_mode = !shadow_mode;

        for (auto& light: *lights) {
            light.project_shadows = shadow_mode;
        }
    }

    // model transforms
    // translation
    if (Input::IsKeyReleased(_window, GLFW_KEY_TAB))
    {
        rackets[selected_player].position = default_rackets[selected_player].position;
        rackets[selected_player].rotation = default_rackets[selected_player].rotation;
        rackets[selected_player].scale = default_rackets[selected_player].scale;
    }

    if (Input::IsKeyPressed(_window, GLFW_KEY_W) && Input::IsKeyPressed(_window, GLFW_KEY_LEFT_SHIFT))
    {
        rackets[selected_player].position += glm::vec3(-1.0f, 0.0f, 0.0f) * (float)_deltaTime * 10.0f;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_S) && Input::IsKeyPressed(_window, GLFW_KEY_LEFT_SHIFT))
    {
        rackets[selected_player].position += glm::vec3(1.0f, 0.0f, 0.0f) * (float)_deltaTime * 10.0f;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_A) && Input::IsKeyPressed(_window, GLFW_KEY_LEFT_SHIFT))
    {
        rackets[selected_player].position += glm::vec3(0.0f, -1.0f, 0.0f) * (float)_deltaTime * 10.0f;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_D) && Input::IsKeyPressed(_window, GLFW_KEY_LEFT_SHIFT))
    {
        rackets[selected_player].position += glm::vec3(0.0f, 1.0f, 0.0f) * (float)_deltaTime * 10.0f;
    }

    // rotation
    if (Input::IsKeyPressed(_window, GLFW_KEY_Q))
    {
        rackets[selected_player].rotation += glm::vec3(-20.0f, 0.0f, 0.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_E))
    {
        rackets[selected_player].rotation += glm::vec3(20.0f, 0.0f, 0.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_W))
    {
        rackets[selected_player].rotation += glm::vec3(0.0f, 0.0f, 20.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_S))
    {
        rackets[selected_player].rotation += glm::vec3(0.0f, 0.0f, -20.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_A))
    {
        rackets[selected_player].rotation += glm::vec3(0.0f, 20.0f, 0.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_D))
    {
        rackets[selected_player].rotation += glm::vec3(0.0f, -20.0f, 0.0f) * (float)_deltaTime;
    }

    // scale
    if (Input::IsKeyPressed(_window, GLFW_KEY_U))
    {
        rackets[selected_player].scale += glm::vec3(1.0f) * (float)_deltaTime;
    }
    if (Input::IsKeyPressed(_window, GLFW_KEY_J))
    {
        rackets[selected_player].scale += glm::vec3(-1.0f) * (float)_deltaTime;
    }

    // only the main camera can be moved
    if (selected_player == 2) {
        // camera translates (side to side and zoom forwards & back)
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_7) /* || Input::IsKeyPressed(_window, GLFW_KEY_Y)*/) {
            main_camera->OneAxisMove(Camera::Translation::UP, (float) _deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_9) /* || Input::IsKeyPressed(_window, GLFW_KEY_I)*/) {
            main_camera->OneAxisMove(Camera::Translation::DOWN, (float) _deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_4) /* || Input::IsKeyPressed(_window, GLFW_KEY_H)*/) {
            main_camera->OneAxisMove(Camera::Translation::LEFT, (float) _deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_6) /* || Input::IsKeyPressed(_window, GLFW_KEY_K)*/) {
            main_camera->OneAxisMove(Camera::Translation::RIGHT, (float) _deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_8) /*|| Input::IsKeyPressed(_window, GLFW_KEY_U)*/) {
            main_camera->OneAxisMove(Camera::Translation::FORWARD, (float) _deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_KP_2) /*|| Input::IsKeyPressed(_window, GLFW_KEY_J)*/) {
            main_camera->OneAxisMove(Camera::Translation::BACKWARD, (float) _deltaTime);
        }

        // camera orbit
        if (Input::IsKeyPressed(_window, GLFW_KEY_UP))
        {
            main_camera->OneAxisOrbit(Camera::Orbitation::ORBIT_UP, (float)_deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_DOWN))
        {
            main_camera->OneAxisOrbit(Camera::Orbitation::ORBIT_DOWN, (float)_deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_RIGHT))
        {
            main_camera->OneAxisOrbit(Camera::Orbitation::ORBIT_RIGHT, (float)_deltaTime);
        }
        if (Input::IsKeyPressed(_window, GLFW_KEY_LEFT))
        {
            main_camera->OneAxisOrbit(Camera::Orbitation::ORBIT_LEFT, (float)_deltaTime);
        }
    }

    // mouse triggers
    // forwards & back in camera local coordinates
    if (Input::IsMouseButtonPressed(_window, GLFW_MOUSE_BUTTON_LEFT))
    {
        main_camera->OneAxisMove(Camera::Translation::CAMERA_FORWARD, (float)(_deltaTime * Input::cursor_delta_y));
    }

    // tilt control
    if (Input::IsMouseButtonPressed(_window, GLFW_MOUSE_BUTTON_MIDDLE))
    {
        main_camera->OneAxisRotate(Camera::Rotation::POSITIVE_PITCH, (float)(_deltaTime * Input::cursor_delta_y));
    }

    // pan control
    if (Input::IsMouseButtonPressed(_window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        main_camera->OneAxisRotate(Camera::Rotation::POSITIVE_YAW, (float)(_deltaTime * Input::cursor_delta_x));
    }

    // camera rotation reset
    if (Input::IsKeyPressed(_window, GLFW_KEY_HOME) || Input::IsKeyPressed(_window, GLFW_KEY_KP_5))
    {
        main_camera->SetDefaultPositionAndTarget();
    }
}
