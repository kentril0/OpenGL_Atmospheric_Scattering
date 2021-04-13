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
// Static members
// --------------------------------------------------------------------------
static void HelpMarker(const char* desc);


// --------------------------------------------------------------------------
Application::Application(GLFWwindow* w, size_t initial_width, size_t initial_height) 
  : m_window(w),
    m_width(initial_width), 
    m_height(initial_height),
    m_state(STATE_MODIFY),
    m_totalVertices(0), m_totalIndices(0)
{
    LOG_INFO("Screen Dimensions: " << m_width << " x " << m_height);

    // "Show" the cursor
    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // --------------------------------------------------------------------------
    // TODO initialize TODO
    // --------------------------------------------------------------------------
    
    m_drawMeshProgram = std::make_shared<Shader>(
                                         "shaders/draw_mesh.vert", 
                                         "shaders/draw_mesh.frag");
    // Load meshes
    m_meshes = Mesh::from_file("objects/sphere.obj");

    for (auto& mesh : m_meshes) {
        m_totalVertices += mesh->vertices();
        m_totalIndices += mesh->indices();
    }

    m_atmosphere = std::make_unique<Atmosphere>(m_drawMeshProgram, 
                                                m_meshes[0].get());

    m_camera = std::make_unique<Camera>(float(m_width) / float(m_height), 
                                        glm::vec3(0, 
                                                  m_atmosphere->get_earthRadius() -1,
                                                  30));

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
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
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
    // TODO only in resize?
    glViewport(0, 0, m_width, m_height);

    // --------------------------------------------------------------------------
    // Sart the Dear ImGUI frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // --------------------------------------------------------------------------
    // Draw the scene
    // --------------------------------------------------------------------------
    m_atmosphere->draw(m_deltaTime);

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

    // Get projection and view matrices defined by the camera
    //m_projView = m_camera->proj_matrix() * m_camera->view_matrix();

    m_atmosphere->set_projView(m_camera->proj_matrix(), m_camera->view_matrix());
    m_atmosphere->set_viewPos(m_camera->position());

    // TODO
    //if (m_animateCamera)
    //    m_camera->updateAnim(m_deltaTime, R_e + 1);
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
            if (ImGui::SliderFloat("Far plane", &farC, 100.f, 3000.f))
                m_camera->set_far_plane_dist(farC);

            // TODO camera preset positions relative to the ground
            ImGui::Text("Position Presets");
            ImGui::Separator();
            if (ImGui::Button("On Ground")) { cameraSetPresetOnGround(); }
            ImGui::SameLine();
            if (ImGui::Button("Above Atmosphere")) { cameraSetPresetAboveAtmosphere(); }

            ImGui::NewLine();
        }
        if (ImGui::CollapsingHeader("Atmosphere Controls", 
                                    ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 sunDir = m_atmosphere->get_sunDir();
            static float R_e = m_atmosphere->get_earthRadius();
            static float R_a = m_atmosphere->get_atmosRadius();

            if (ImGui::TreeNodeEx("Optical coefficients", 
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                static float I_sun = m_atmosphere->get_sunIntensity();
                bool animateSun = m_atmosphere->is_animateSun();
                static glm::vec3 beta_R = m_atmosphere->get_rayleighScattering();
                static float H_R = m_atmosphere->get_rayleighScaleHeight();
                static float beta_M = m_atmosphere->get_mieScattering();
                static float H_M = m_atmosphere->get_mieScaleHeight();
                static float g = m_atmosphere->get_mieScatteringDir();
                static float sunAngle = m_atmosphere->get_sunAngle();

                ImGui::Separator();
                ImGui::Text("Sun properties");
                ImGui::SameLine();
                if (ImGui::Button("Defaults##sun")) {
                    m_atmosphere->set_sunDefaults();
                    sunDir = m_atmosphere->get_sunDir();
                    I_sun = m_atmosphere->get_sunIntensity();
                    sunAngle = m_atmosphere->get_sunAngle();
                    animateSun = m_atmosphere->is_animateSun();
                }
                if (ImGui::SliderFloat("Sun Intensity", &I_sun, 0.01, 100.)) {
                    m_atmosphere->set_sunIntensity(I_sun);
                }
                if (ImGui::SliderAngle("Sun Angle", &sunAngle, -10.f, 190.f)) {
                    m_atmosphere->set_animateSun(false);
                    m_atmosphere->set_sunAngle(sunAngle);
                }
                if (ImGui::Checkbox(" Animate ", &animateSun)) {
                    m_atmosphere->set_animateSun(animateSun);
                }

                ImGui::Separator();
                ImGui::Text("Rayleigh Scattering");
                HelpMarker("Simulates scattering on small particles of air.");
                ImGui::SameLine();
                if (ImGui::Button("Defaults##ray")) {
                    m_atmosphere->set_rayleighDefaults();
                    beta_R = m_atmosphere->get_rayleighScattering();
                    H_R = m_atmosphere->get_rayleighScaleHeight();
                }
                if (ImGui::DragFloat3("Coefficient", glm::value_ptr(beta_R), 
                                   1e-4f, 0.0, 1.0, "%.4f")) {
                    m_atmosphere->set_rayleighScattering(beta_R);
                }
                HelpMarker("Scattering coefficient for wavelengths of red, green,\n"
                           "and blue light.\n"
                           "The less for a certain wavelength, the more prominent\n"
                           "(less out-scattered) its color, in [m^-1]");
                if (ImGui::SliderFloat("Scale height", &H_R, 1.0, R_a - R_e)) {
                    m_atmosphere->set_rayleighScaleHeight(H_R);
                }
                HelpMarker("Altitude by which the density of the atmosphere\n"
                           "decreases by a factor of e, in [km]");
                ImGui::Separator();

                ImGui::Text("Mie Scattering");
                HelpMarker("Simulates scattering on aerosols, i.e. larger particles\n"
                           "of air.");
                ImGui::SameLine();
                if (ImGui::Button("Defaults##mie")) {
                    m_atmosphere->set_mieDefaults();
                    beta_M = m_atmosphere->get_mieScattering();
                    H_M = m_atmosphere->get_mieScaleHeight();
                    g = m_atmosphere->get_mieScatteringDir();
                }
                if (ImGui::SliderFloat("Coefficient", &beta_M, 1e-3f, 1.f)) {
                    m_atmosphere->set_mieScattering(beta_M);
                }
                HelpMarker("Scattering coefficient for wavelength of visible light,\n"
                           "the higher the value, the foggier it gets, in [m^-1]");
                if (ImGui::SliderFloat("Scale height##mie", &H_M, 1.0f, R_a - R_e)) {
                    m_atmosphere->set_mieScaleHeight(H_M);
                }
                HelpMarker("Altitude by which the density of the atmosphere\n"
                           "decreases by a factor of e, in [km]");
                if (ImGui::SliderFloat("Anisotropy", &g, 0.01f, 1.0f)) {
                    m_atmosphere->set_mieScatteringDir(g);
                }
                HelpMarker("Directivity of the light in the medium, higher\n"
                           "values result in stronger forward directivity");

                if (ImGui::Button("All to Defaults")) {
                    m_atmosphere->set_defaults();
                    sunDir = m_atmosphere->get_sunDir();
                    R_e = m_atmosphere->get_earthRadius();
                    R_a = m_atmosphere->get_atmosRadius();
                    I_sun = m_atmosphere->get_sunIntensity();
                    animateSun = m_atmosphere->is_animateSun();
                    beta_R = m_atmosphere->get_rayleighScattering();
                    H_R = m_atmosphere->get_rayleighScaleHeight();
                    beta_M = m_atmosphere->get_mieScattering();
                    H_M = m_atmosphere->get_mieScaleHeight();
                    g = m_atmosphere->get_mieScatteringDir();
                    sunAngle = m_atmosphere->get_sunAngle();
                }
                ImGui::TreePop();
            }

            ImGui::Separator();
            if (ImGui::TreeNode("Render options (Dangerous)"))
            {
                static int viewSamples = m_atmosphere->get_viewSamples();
                static int lightSamples = m_atmosphere->get_lightSamples();
                static bool toneMapping = m_atmosphere->is_toneMapping();
                static bool renderEarth = m_atmosphere->is_renderEarth();

                ImGui::Text("Quality options");
                if (ImGui::DragFloat3("Sun Direction", glm::value_ptr(sunDir), 0.1)) {
                    m_atmosphere->set_animateSun(false);
                    m_atmosphere->set_sunDir(sunDir);
                }
                if (ImGui::SliderInt("View Samples", &viewSamples, 1, 64)) {
                    m_atmosphere->set_viewSamples(viewSamples);
                }
                if (ImGui::SliderInt("Light Samples", &lightSamples, 1, 64)) {
                    m_atmosphere->set_lightSamples(lightSamples);
                }
                if (ImGui::Checkbox(" Tone mapping ", &toneMapping)) {
                    m_atmosphere->set_toneMapping(toneMapping);
                }
                ImGui::Separator();

                ImGui::Separator();
                ImGui::Text("Planet properties [km]");
                ImGui::SameLine();
                if (ImGui::Button("Defaults##pl")) {
                    m_atmosphere->set_sizeDefaults();
                    R_e = m_atmosphere->get_earthRadius();
                    R_a = m_atmosphere->get_atmosRadius();
                    renderEarth = m_atmosphere->is_renderEarth();
                }
                if (ImGui::SliderFloat("Earth radius", &R_e, 1., 10000)) {
                    m_atmosphere->set_earthRadius(R_e);
                    if (R_e > R_a)
                        R_a = R_e; 
                }
                if (ImGui::SliderFloat("Atmosphere radius", &R_a, R_e, 10000)) {
                    m_atmosphere->set_atmosRadius(R_a);
                }
                if (ImGui::Checkbox(" Render Ground ", &renderEarth)) {
                    m_atmosphere->set_renderEarth(renderEarth);
                }

                ImGui::TreePop();
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
    ImGui::Text("%u vertices, %u indices (%u triangles)", 
                m_totalVertices, m_totalIndices, 
                (uint32_t)(m_totalIndices / 3));

    ImGui::End();
}

void Application::on_resize(GLFWwindow *window, int width, int height)
{
    m_width = width;
    m_height = height;
    // TODO
    // glViewport?? glViewport(0, 0, width, height);
}

void Application::on_mouse_move(GLFWwindow *window, double x, double y) 
{ 
    if (m_state == STATE_FREEFLY)
        m_camera->on_mouse_move(x, y);
}

void Application::on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        set_state_freefly();
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        set_state_modify();
    }
}

void Application::on_key_pressed(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    m_key = key;
    m_keyAction = action;

    call_registered(key, action);
}

void Application::set_state_modify()
{
    set_state(STATE_MODIFY);
    m_camera->key_reset(0);

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

void Application::cameraSetPresetOnGround()
{
    m_camera->set_position(glm::vec3(0, 
                           m_atmosphere->get_earthRadius() -1,
                           30));
    m_camera->set_pitch(20);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(glm::radians(60.0));
}

void Application::cameraSetPresetAboveAtmosphere()
{
    m_camera->set_position(glm::vec3(0, 
                           m_atmosphere->get_atmosRadius(),
                           30));
    m_camera->set_pitch(-10);
    m_camera->set_yaw(270);
    m_camera->set_field_of_view(glm::radians(60.0));
}

