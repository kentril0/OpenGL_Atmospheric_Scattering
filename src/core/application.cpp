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

    m_drawMeshProgram = std::make_unique<Shader>(
                                         "shaders/draw_mesh.vs", 
                                         "shaders/draw_mesh.fs");
    // Load meshes
    meshes = Mesh::from_file("objects/sphere.obj");

    m_atmosphereProgram = std::make_unique<Shader>(
                                         "shaders/draw_atmosphere.vs", 
                                         "shaders/draw_atmosphere.fs");

    // Atmospheric scattering coefficients
    viewSamples  = 16;
    lightSamples = 8;
    
    sunPos = glm::vec3(0, 0, -1);
    m_sunAngle = 0; //M_PI * 0.5f;
    I_sun  = 22.0;

    R_e    = 760.0;
    R_a    = 1100.0;
    beta_R = glm::vec3(5.5e-3f, 15.0e-3f, 22.4e-3f);
    beta_M = 21e-3f;
    H_R    = 100.0;
    H_M    = 20.0;
    g      = 0.888;

    viewSamples = defViewSamples;
    lightSamples = defLightSamples;
    sunPos = sunDir;
    I_sun = e_I_sun;
    R_e = e_R_e;
    R_a = e_R_a;
    beta_R = e_beta_R;
    beta_M = e_beta_M;
    H_R = e_H_R;
    H_M = e_H_M;
    g = e_g;

    m_camera = std::make_unique<Camera>(float(m_width) / float(m_height), 
                                        glm::vec3(0, R_e-1, 30));

    // --------------------------------------------------------------------------
    // Register callbacks
    // --------------------------------------------------------------------------
    m_callbackMap = {
        // Key,             Action,     State           TODO lambda functions
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
        { {KEY_CAM_LEFT,    GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_left} },
        { {KEY_CAM_SPEEDUP, GLFW_PRESS, STATE_FREEFLY}, {&Application::camera_speedUp} },
        { {KEY_CAM_SPEEDUP, GLFW_RELEASE, STATE_FREEFLY}, {&Application::camera_speedUp} }
    };

    // --------------------------------------------------------------------------
    // Setup OpenGL states
    // --------------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    //
    //glEnable(GL_CULL_FACE);
    	//	glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
    //glm::mat4 view(1.0);
    //glm::mat4 proj = glm::perspective((float)(3.14159 / 4.), 
    //                                  (float)((float)m_width/ (float)m_height), 
    //                                  0.1f, 1000.0f);
    //m_projView = proj * view;

    glm::mat4 m = glm::scale(glm::mat4(1.0f), 
                                 glm::vec3(R_e, 
                                           R_e, 
                                           R_e));
    m_drawMeshProgram->use();
    m_drawMeshProgram->set_mat4("MVP",  proj * view * m);
    meshes[0]->draw();

    glm::mat4 model = glm::scale(glm::mat4(1.0f), 
                                 glm::vec3(R_a, 
                                           R_a, 
                                           R_a));

    m_atmosphereProgram->use();
    m_atmosphereProgram->set_mat4("M", model);
    // TODO MVP fix
    m_atmosphereProgram->set_mat4("MVP", proj * view * model);
    
    m_atmosphereProgram->set_vec3("viewPos", m_camera->position());

    if (m_animateSun)
    {
        m_sunAngle = glm::mod(m_sunAngle - 0.5 * m_deltaTime, M_PI + 0.1);
        sunPos.y = glm::sin(m_sunAngle);
        sunPos.z = glm::cos(m_sunAngle);
    }

    m_atmosphereProgram->set_vec3("sunPos", sunPos);

    m_atmosphereProgram->set_int("viewSamples", viewSamples);
    m_atmosphereProgram->set_int("lightSamples", lightSamples);
    m_atmosphereProgram->set_float("I_sun", I_sun);
    m_atmosphereProgram->set_float("R_e", R_e);
    m_atmosphereProgram->set_float("R_a", R_a);

    // Rayleight scattering coefficient
    m_atmosphereProgram->set_vec3("beta_R", beta_R);
    m_atmosphereProgram->set_float("beta_M", beta_M);
    m_atmosphereProgram->set_float("H_R", H_R);
    m_atmosphereProgram->set_float("H_M", H_M);
    m_atmosphereProgram->set_float("g", g);

    m_atmosphereProgram->set_float("toneMappingFactor", m_toneMapping * 1.0);


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
    // TODO
    //if (m_animateCamera)
    //    m_camera->updateAnim(m_deltaTime, R_e + 1);
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
            static float fov = m_camera->field_of_view();
            static float nearC = m_camera->near_plane_dist();
            static float farC = m_camera->far_plane_dist();

            glm::vec3 pos = m_camera->position();
            float pitch = m_camera->pitch();
            float yaw = m_camera->yaw();

            if (ImGui::DragFloat3("Position", glm::value_ptr(pos)))
                m_camera->set_position(pos);
            if (ImGui::SliderFloat("Pitch angle", &pitch, -89.f, 89.f, "%.0f deg"))
                m_camera->set_pitch(pitch);
            if (ImGui::SliderFloat("Yaw angle", &yaw, 0.f, 360.f, "%.0f deg"))
                m_camera->set_yaw(yaw);
            if (ImGui::SliderAngle("Field of view", &fov, 0.f, 180.f))
                m_camera->set_field_of_view(fov);
            if (ImGui::SliderFloat("Near plane", &nearC, 0.f, 10.f))
                m_camera->set_near_plane_dist(nearC);
            if (ImGui::SliderFloat("Far plane", &farC, 100.f, 2000.f))
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
        if (ImGui::CollapsingHeader("Atmosphere Controls", 
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            {
              // TODO animate
                // TODO presets or defaults


                ImGui::Text("Accuracy of rays"); // ?? TODO
                ImGui::SliderInt("View Samples", &viewSamples, 1, 64); 
                ImGui::SliderInt("Light Samples", &lightSamples, 1, 64);

                ImGui::Separator();
                ImGui::Text("Sun properties");
                ImGui::DragFloat3("Sun Direction", glm::value_ptr(sunPos), 0.1);
                ImGui::SliderFloat("Sun Intensity", &I_sun, 0.01, 100.);
                ImGui::Checkbox(" Animate ", &m_animateSun);
                ImGui::Checkbox(" Tone mapping ", &m_toneMapping);

                ImGui::Separator();
                ImGui::Text("Earth properties [km]");

                ImGui::SliderFloat("Earth radius", &R_e, 1., 10000);
                ImGui::SliderFloat("Atmosphere radius", &R_a, 1., 10000);

                ImGui::Separator();
                ImGui::Text("Rayleigh Scattering");
                ImGui::DragFloat3("Coefficient", glm::value_ptr(beta_R), 
                                   1e-4f, 0.0, 1.0, "%.4f");
                ImGui::SliderFloat("Scale height", &H_R, 1.0, 1e4f);

                ImGui::Separator();
                ImGui::Text("Mie Scattering");
                ImGui::SliderFloat("Coefficient", &beta_M, 1e-3, 1.);
                ImGui::SliderFloat("Scale height#", &H_M, 1.0, 1e4f);
                ImGui::SliderFloat("Anisotropy (Direction)", &g, 0.1, 1.0);


                if (ImGui::Button("Defaults"))
                {
                    viewSamples = defViewSamples;
                    lightSamples = defLightSamples;
                    sunPos = sunDir;
                    I_sun = e_I_sun;
                    R_e = e_R_e;
                    R_a = e_R_a;
                    beta_R = e_beta_R;
                    beta_M = e_beta_M;
                    H_R = e_H_R;
                    H_M = e_H_M;
                    g = e_g;
                }

            }
            ImGui::Separator();
        }

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

