/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file application.hpp
 * @brief Main application abstraction
 *********************************************************/

#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "scene/skybox.hpp"
#include "scene/camera.hpp"
#include "scene/mesh.hpp"


/**@brief Controls used in application */
enum Controls
{
    KEY_TOGGLE_MENU  = GLFW_KEY_ESCAPE,
    KEY_CAM_FORWARD  = GLFW_KEY_W,
    KEY_CAM_BACKWARD = GLFW_KEY_S,
    KEY_CAM_RIGHT    = GLFW_KEY_D,
    KEY_CAM_LEFT     = GLFW_KEY_A,
    KEY_CAM_RCURSOR  = KEY_TOGGLE_MENU,
    KEY_CAM_SPEEDUP  = GLFW_KEY_LEFT_SHIFT 
};


/**
 * @brief Main application that gets rendered in the setup window
 */
class Application
{
public:
    Application(GLFWwindow* w, size_t initial_width, size_t initial_height);

    ~Application();

    void loop();

    // ----------------------------------------------------------------------------
    // Input events
    // ----------------------------------------------------------------------------
    void on_resize(GLFWwindow *window, int width, int height);
    void on_mouse_move(GLFWwindow *window, double x, double y);
    void on_mouse_pressed(GLFWwindow *window, int button, int action, 
                          int mods);
    void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                        int mods);

private:
    void render();

    void update();

    void set_vsync(bool enabled);

    // ----------------------------------------------------------------------------
    // Key mapping
    // ----------------------------------------------------------------------------
    void call_registered(int key, int action)
    { 
        const auto& it = m_callbackMap.find({key,action,m_state});
        if (it != m_callbackMap.end())
        {
            const auto& vec_callbacks = it->second;
            for (const auto& c : vec_callbacks)
                (this->*c)();
        }
    }
    // ----------------------------------------------------------------------------

    // Functions modifying application state
    void set_state(int s) { DERR("New state: " << s); m_state = s; }

    void set_state_modify();    ///< Shows interface for modifications
    void set_state_freefly();   ///< Hides interface and enables flying w/ camera

    // ImGui functions
    void show_interface();

    void status_window();

    // Camera callbacks
    //void camera_key_pressed() { m_camera->on_key_pressed(m_key, m_keyAction); }
    void camera_forward()     { m_camera->key_forward(m_keyAction); }
    void camera_backward()    { m_camera->key_backward(m_keyAction); }
    void camera_right()       { m_camera->key_right(m_keyAction); }
    void camera_left()        { m_camera->key_left(m_keyAction); }
    void camera_reset()       { m_camera->key_reset(m_keyAction); }
    void camera_speedUp()     { m_camera->key_speedUp(m_keyAction); }

private:
    // ----------------------------------------------------------------------------
    // Typedefs
    // ----------------------------------------------------------------------------
    typedef void (Application::*Callback)(void);
    typedef std::vector<Callback> callbacks;

    /** @brief Used as a key to a map of callbacks, that are called whenever a key
     *         with action and during a state is pressed. */
    struct CallbackKey
    {
        int key;            ///< The GLFW_KEY_* - a.k.a. key on a keyboard
        int action;         ///< Usually a GLFW_PRESS or GLFW_RELEASE
        int state;          ///< The application state

        CallbackKey(int k, int a, int s) : key(k), action(a), state(s) {}
        bool operator==(const CallbackKey &other) const
        {
            return (key == other.key
                    && action == other.action
                    && state == other.state);
        }
    };

    // Make the CallbackKey type hashable 
    struct Callback_hash
    {
        std::size_t operator()(const CallbackKey& k) const
        {
            // Compute individual hash values and combine them 
            //  using XOR and bit shifting:
            return ((std::hash<int>()(k.key) 
                     ^ (std::hash<int>()(k.action) << 1)) >> 1) 
                     ^ (std::hash<int>()(k.state) << 1);
        }
    };

    // ----------------------------------------------------------------------------
    // Data members 
    // ----------------------------------------------------------------------------
    GLFWwindow* m_window;

    size_t m_width;
    size_t m_height;

    // Timestamps
    double m_lastFrame, m_framestamp, m_deltaTime;
    uint32_t m_frames;

    // Maps the key, action and state to callback function
    std::unordered_map<CallbackKey, callbacks, Callback_hash> m_callbackMap;

    int m_key, m_keyAction;        ///< Keyboard key and action

    enum States     ///< Application states, determines 
    {
        STATE_MODIFY,   ///< GUI is shown and allows for modifications
        STATE_FREEFLY   ///< GUI is hidden, camera is moving freely
    };

    int m_state;    ///< Current app state

    // ----------------------------------------------------------------------------
    // Scene
    // ----------------------------------------------------------------------------
    glm::mat4 m_projView;

    std::unique_ptr<Camera> m_camera;

    void cameraSetPresetTop();
    void cameraSetPresetFront();
    void cameraSetPresetSideWays();

    std::unique_ptr<Skybox> m_skybox;

    std::vector<std::unique_ptr<Mesh>> meshes;

    std::unique_ptr<Shader> m_drawMeshProgram; 

    // ----------------------------------------------------------------------------
    // Atmospheric constants
    // ----------------------------------------------------------------------------
    std::unique_ptr<Shader> m_atmosphereProgram; 
    
    bool m_toneMapping = true;  ///< Whether tone mapping function is applied

    int viewSamples;        ///< Number of samples along the view (primary) ray
    int lightSamples;       ///< Number of samples along the light (secondary) ray

    glm::vec3 sunPos;       ///< Position of the sun
    float I_sun;            ///< Intensity of the sun
    float R_e;              ///< Radius of the Earth
    float R_a;              ///< Radius of the atmosphere
    glm::vec3 beta_R;       ///< Rayleigh scattering coefficient
    float beta_M;           ///< Mie scattering coefficient
    float H_R;              ///< Rayleigh scale height
    float H_M;              ///< Mie scale height
    float g;                ///< Mie scattering direction - anisotropy of the medium

    float m_sunAngle;
    bool m_animateSun = true;
    // ----------------------------------------------------------------------------
    // Defaults
    const int defViewSamples = 16;
    const int defLightSamples = 8;

    // Earth presets in [km]
    const glm::vec3 sunDir = glm::vec3(0, 1, 0);
    const float e_I_sun = 20.f;
    const float e_R_e = 6360.;       // 6360e3
    const float e_R_a = 6420.;       // 6420e3;
    const glm::vec3 e_beta_R = glm::vec3(3.8e-3f, 13.5e-3f, 33.1e-3f);
        //(3.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel implementation
        //(5.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel web
        //(5.5e-3f, 15.0e-3f, 22.4e-3f);    // 
    const float e_beta_M = 21e-3f;          // 21e-6f
    const float e_H_R = 7.994;            // 7994, 100
    const float e_H_M = 1.200;            // 1200, 20
    const float e_g = 0.76;             // 0.888

    // Conversions
    const float M_2_KM = 0.001;
    const float KM_2_M = 1000.0;
};

