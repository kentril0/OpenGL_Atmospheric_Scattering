/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file application.cpp
 * @brief Main application abstraction
 *********************************************************/

#include "pch.hpp"
#include "application.hpp"

// --------------------------------------------------------------------------
// Static variables
// --------------------------------------------------------------------------

#define PLANET_RADIUS 6360e3

// --------------------------------------------------------------------------
Application::Application(GLFWwindow* w, size_t initial_width, size_t initial_height) 
  : m_window(w),
    m_width(initial_width), 
    m_height(initial_height),
    m_state(STATE_MODIFY)
{
    LOG_INFO("Screen Dimensions: " << m_width << " x " << m_height);

    // "Show" the cursor
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // --------------------------------------------------------------------------
    // TODO initialize TODO
    // --------------------------------------------------------------------------
    m_camera = std::make_unique<Camera>(float(m_width) / float(m_height), 
                                        glm::vec3(0.0f, 1.0f, 0.0f));
                                        //glm::vec3(0.0f, (PLANET_RADIUS + 30), 0.0f));
    
    std::shared_ptr<Shader> shSkybox = std::make_shared<Shader>(
                                         "shaders/draw_skybox.vs", 
                                         "shaders/draw_skybox.fs");
    m_skybox = std::make_unique<Skybox>(shSkybox, 
                                        std::initializer_list<const char*>{
                                         "images/skybox/right.jpg",
                                         "images/skybox/left.jpg",
                                         "images/skybox/top.jpg",
                                         "images/skybox/bottom.jpg",
                                         "images/skybox/front.jpg",
                                         "images/skybox/back.jpg"});

    m_atmosphereProgram = std::make_unique<Shader>(
                                         "shaders/draw_atmosphere.vs", 
                                         "shaders/draw_atmosphere.fs");

    m_drawMeshProgram = std::make_unique<Shader>(
                                         "shaders/draw_mesh.vs", 
                                         "shaders/draw_mesh.fs");
    // Load meshes
    meshes = Mesh::from_file("objects/sphere.obj");



    // --------------------------------------------------------------------------
    // Register callbacks
    // --------------------------------------------------------------------------
    m_callbackMap = {
        // Key,             Action,     State
        { {KEY_TOGGLE_MENU, GLFW_PRESS, STATE_MODIFY}, {&Application::set_state_freefly} },
        { {KEY_TOGGLE_MENU, GLFW_PRESS, STATE_FREEFLY}, {&Application::set_state_modify, 
                                                         &Application::camera_reset} },
        // Camera
        { {KEY_CAM_FORWARD, GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_forward} },
        { {KEY_CAM_FORWARD, GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_forward} },
        { {KEY_CAM_BACKWARD,GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_backward} },
        { {KEY_CAM_BACKWARD,GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_backward} },
        { {KEY_CAM_RIGHT,   GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_right} },
        { {KEY_CAM_RIGHT,   GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_right} },
        { {KEY_CAM_LEFT,    GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_left} },
        { {KEY_CAM_LEFT,    GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_left} }
    };

    // --------------------------------------------------------------------------
    // Setup OpenGL states
    // --------------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    set_vsync(true);


    // --------------------------------------------------------------------------
    // Get current timestamp - prepare for main loop
    // --------------------------------------------------------------------------
    m_lastFrame = glfwGetTime();
    m_framestamp = m_lastFrame;
}

Application::~Application()
{

}

void Application::loop()
{
    // --------------------------------------------------------------------------
    // Calculate delta time
    // --------------------------------------------------------------------------
    double currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    // Frametime and FPS counter, updates once per 1 second
    if (currentFrame - m_framestamp > 1.0f)
    {
        m_framestamp += 1.0f;
        m_frames = 0;
    }

    update();

    render();
}

void Application::render()
{
    // --------------------------------------------------------------------------
    // Clear and reset
    // --------------------------------------------------------------------------
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_width, m_height);

    // --------------------------------------------------------------------------
    // Sart the Dear ImGUI frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // --------------------------------------------------------------------------
    // Draw the scene
    // --------------------------------------------------------------------------
    // Get projection and view matrices defined by the camera
    glm::mat4 proj = m_camera->proj_matrix();
    glm::mat4 view = m_camera->view_matrix();
    m_projView = proj * view;

    // TODO render smth
    //m_drawMeshProgram->use();
    //m_drawMeshProgram->set_mat4("MVP",  m_projView);
    //for (auto& mesh : meshes)
    //{
    //    mesh->draw();
    //}
    //LOG_INFO("Done");

    //glDisable(GL_DEPTH_TEST);
    //glm::mat4 model(1.0);
    glm::mat4 model = glm::scale(glm::mat4(1.0f), 
                                 glm::vec3(3.0f, 
                                           3.0f, 
                                           3.0f));

    //glm::mat4 model = glm::scale(glm::mat4(1.0f), 
    //                             glm::vec3(PLANET_RADIUS + 60e3, 
    //                                       PLANET_RADIUS + 60e3, 
    //                                       PLANET_RADIUS + 60e3));

    m_atmosphereProgram->use();

    m_atmosphereProgram->set_mat4("M", model);
    m_atmosphereProgram->set_mat4("MVP", proj * view * model);
    
    // TODO should not change
    m_atmosphereProgram->set_vec3("viewPos", m_camera->position());
    // Direction of the sun
    m_atmosphereProgram->set_vec3("sunPos", glm::vec3(0, -1500, 0));

    m_atmosphereProgram->set_int("viewSamples", 16);
    m_atmosphereProgram->set_int("lightSamples", 8);

    m_atmosphereProgram->set_float("I_sun", 20.f);
    m_atmosphereProgram->set_float("R_e", 6360e3);
    m_atmosphereProgram->set_float("R_a", 6420e3);

    // Rayleight scattering coefficient
    const glm::vec3 beta_R(3.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel implementation
    const glm::vec3 beta_R1(5.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel web 
    const glm::vec3 beta_R2(5.5e-3f, 15.0e-3f, 22.4e-3f);    // sky_fragment.glsl
    m_atmosphereProgram->set_vec3("beta_R", beta_R);
    m_atmosphereProgram->set_float("beta_M", 21e-6f);
    m_atmosphereProgram->set_float("H_R", 7994);
    m_atmosphereProgram->set_float("H_M", 1200);
    m_atmosphereProgram->set_float("g", 0.76f);

    /// Issues a draw call
    meshes[0]->draw();

    // TODO delete later
    // Render skybox as last
    //m_skybox->render(view, proj);

    // --------------------------------------------------------------------------
    // ImGUI render
    // --------------------------------------------------------------------------
    
    // By default GUI is shown
    {
        show_interface();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    // --------------------------------------------------------------------------

    m_frames++;
}

void Application::update()
{
    m_camera->update(m_deltaTime);
}


void Application::on_resize(GLFWwindow *window, int width, int height)
{
    m_width = width;
    m_height = height;
    // TODO
    // glViewport?? glViewport(0, 0, width, height);
    DERR("SCREEN RESIZE");
}

void Application::on_mouse_move(GLFWwindow *window, double x, double y) 
{ 
    if (m_state == STATE_FREEFLY)
        m_camera->on_mouse_move(x, y);
}

void Application::on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{

}

void Application::on_key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    m_key = key;
    m_keyAction = action;

    call_registered(key, action);
}

/**@brief ImGui: adds "(?)" with hover one the same line as the prev obj */
static void HelpMarker(const char* desc)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Application::show_interface()
{
    if (m_state == STATE_MODIFY)
    {
        if (!ImGui::Begin("Application Controls", NULL))
        {
            ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Configuration"))
        {
            static bool vsync = true;
            if (ImGui::Checkbox(" Vertical sync", &vsync))
                set_vsync(vsync);
        }

        if (ImGui::CollapsingHeader("Camera Settings"))
        {
            // TODO help (?)
            static float fov = glm::radians(m_camera->field_of_view());
            static float nearC = m_camera->near_plane_dist();
            static float farC = m_camera->far_plane_dist();

            glm::vec3 pos = m_camera->position();
            float pitch = m_camera->pitch();
            float yaw = m_camera->yaw();

            if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
                m_camera->set_position(pos);
            if (ImGui::SliderFloat("Pitch angle", &pitch, -89.f, 89.f, "%.0f deg"))
                m_camera->set_pitch(pitch);
            if (ImGui::SliderFloat("Yaw angle", &yaw, 0.f, 360.f, "%.0f deg"))
                m_camera->set_yaw(yaw);
            if (ImGui::SliderAngle("Field of view", &fov, 0.f, 180.f))
                m_camera->set_field_of_view(fov);
            if (ImGui::SliderFloat("Near plane", &nearC, 0.f, 10.f))
                m_camera->set_near_plane_dist(nearC);
            if (ImGui::SliderFloat("Far plane", &farC, 100.f, 1000.f))
                m_camera->set_far_plane_dist(farC);

            // TODO camera preset positions relative to terrain size
            ImGui::Text("Position Presets");
            ImGui::Separator();
            if (ImGui::Button("From Top")) { cameraSetPresetTop(); }
            ImGui::SameLine();
            if (ImGui::Button("From Front")) { cameraSetPresetFront(); }
            ImGui::SameLine();
            if (ImGui::Button("From Side")) { cameraSetPresetSideWays(); }

            ImGui::NewLine();
        }
        if (ImGui::CollapsingHeader("Atmosphere Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
                static int dim = 4;
                static bool autoUpdate = false;
                static bool falloff = true;
                static float tileScale = 2.;
                static float heightScale = 2.;

                if (ImGui::Checkbox(" Falloff Map", &falloff))
                    falloff = false;

                // TODO ? with  [vertices^2]
                ImGui::SliderInt("Terrain size", &dim, 4, 512);
                ImGui::SliderFloat("Tile scale", &tileScale, 0.01f, 10.f);
                ImGui::SliderFloat("Height scale", &heightScale, 0.01f, 10.f);

                ImGui::NewLine();
                if (ImGui::Button("Generate"))
                {
                    // TODO smth
                }
                else if (autoUpdate)
                {

                }
                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
            }
            ImGui::Separator();
            if (ImGui::TreeNodeEx("Noise Map Generation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static bool autoUpdate = false;
                int32_t seed = 0;
                float scale = 0.1f;

                if (ImGui::DragInt("Seed", &seed))
                    seed = 1;
                if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.f, 100.f))
                    scale = 0.1f;

                // Select noise function
                ImGui::NewLine();
                const char* noiseTypes[] = { "Random", "Perlin", "Accumulated Perlin"};
                static int noiseType = 0;
                ImGui::Combo("Noise function", &noiseType, noiseTypes, 
                             IM_ARRAYSIZE(noiseTypes));

                if (noiseType == 2)
                {
                    int32_t octaves = 1;
                    float persistence = 0.1f;
                    float lacunarity = 1.f;
                    // TODO (?)
                    if (ImGui::SliderInt("Octaves", &octaves, 1, 32))
                        octaves = 1;
                    if (ImGui::DragFloat("Persistence", &persistence, 0.01f, 0.f, 1.f))
                        persistence = 1.f;
                    if (ImGui::DragFloat("Lacunarity", &lacunarity, 0.01f, 1.f, 100.f))
                        lacunarity = 2.f;
                }

                // TODO (?)
                if (ImGui::Button("  Apply  "))
                    seed++;

                ImGui::SameLine();
                ImGui::Checkbox("Auto", &autoUpdate);
                ImGui::TreePop();
            }
            ImGui::Separator();
            if (ImGui::TreeNode("Shading"))
            {
                static bool usingShading = false;
                if (ImGui::Checkbox(" Shading ", &usingShading))
                    usingShading = false;

                ImGui::Text("Directional Light");
                ImGui::Separator();
                static glm::vec3 lightDir      = glm::vec3(1.0f);
                static glm::vec3 lightAmbient  = glm::vec3(1.0f);
                static glm::vec3 lightDiffuse  = glm::vec3(1.0f);
                static glm::vec3 lightSpecular = glm::vec3(1.0f);

                ImGui::DragFloat3("Direction", glm::value_ptr(lightDir), 0.1f);
                ImGui::ColorEdit3("Ambient",  glm::value_ptr(lightAmbient), 
                                              ImGuiColorEditFlags_Float);
                ImGui::ColorEdit3("Diffuse",  glm::value_ptr(lightDiffuse), 
                                              ImGuiColorEditFlags_Float);
                ImGui::ColorEdit3("Specular", glm::value_ptr(lightSpecular), 
                                              ImGuiColorEditFlags_Float);

                ImGui::Text("Material");
                ImGui::Separator();

                ImGui::TreePop();
            }
        }
        /*
        if (ImGui::CollapsingHeader("Sky effects"))
        {
            static glm::vec3 light_col(1.0f);
            static glm::vec3 fog_col(1.0f);
            static float fog_falloff = 1.0f;

            ImGui::ColorEdit3("Light color", glm::value_ptr(light_col));
            ImGui::ColorEdit3("Fog color", glm::value_ptr(fog_col));
            ImGui::SliderFloat("Fog falloff dist", &fog_falloff, 1.f, 1000.f);
        }
        */
        ImGui::End();
    }

    status_window();
}

void Application::status_window()
{
    // Overlay when flying with camera
    if (m_state == STATE_FREEFLY)
        ImGui::SetNextWindowBgAlpha(0.35f);

    // Collapsed or Clipped
    if (!ImGui::Begin("Application Metrics", NULL))
    {
        ImGui::End();
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    // frametime and FPS
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / io.Framerate, io.Framerate);
    ImGui::Text("? vertices, ? indices (? triangles)");
    //ImGui::Text("%d vertices, %d indices (%d triangles)", 
    //            m_terrain->totalVertices(), m_terrain->totalIndices(),
    //            m_terrain->totalTriangles());

    ImGui::End();
}

void Application::set_state_modify()
{
    set_state(STATE_MODIFY);

    // Show the cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::set_state_freefly()
{
    set_state(STATE_FREEFLY);

    // Hide the cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Application::set_vsync(bool enabled)
{
    if (enabled)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);
}

void Application::cameraSetPresetTop()
{
    m_camera->set_position(glm::vec3(5,20,0));
    m_camera->set_pitch(-89);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(45);
}

void Application::cameraSetPresetFront()
{
    m_camera->set_position(glm::vec3(0,5,13));
    m_camera->set_pitch(-22);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(45);
}

void Application::cameraSetPresetSideWays()
{
    m_camera->set_position(glm::vec3(7,11,12));
    m_camera->set_pitch(-40);
    m_camera->set_yaw(245);
    m_camera->set_field_of_view(45);
}

