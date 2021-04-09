/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file camera.hpp
 * @brief FPS Camera abstraction
 *********************************************************/

#pragma once


class Camera
{
public:
    /**
     * TODO fix defaults
     * @brief Create camera
     * @param aspect_ratio 
     * @param pos Position of the camera in the scene
     * @param up Direction up of the camera
     * @param front Direction where the camera looks
     * @param yaw Yaw of the camera in degrees
     * @param pitch Pitch of the camera in degrees
     */
    Camera(float aspect_ratio,
           const glm::vec3& pos = glm::vec3(0.0f, 5.0f, 0.0f), 
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           const glm::vec3& front = glm::vec3(0.0f, 0.0f, -1.0f),
           float yaw = DEFAULT_YAW_DEG, float pitch = DEFAULT_PITCH_DEG);

    void update(float deltaTime);

    // ------------------------------------------------------------------------
    // Getters
    // ------------------------------------------------------------------------
    
    const glm::vec3& position() const { return m_position; }

    const glm::vec3& direction() const { return m_front; }

    /** @return pitch in degrees */
    float pitch() const { return m_pitch; }

    /** @return yaw in degrees */
    float yaw() const { return m_yaw; }

    // TODO make it proj view mat!
    // TODO make class variable
    glm::mat4 view_matrix() const 
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 proj_matrix() const
    {
        return glm::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
    }

    /** @return Field of view in degrees */
    float field_of_view() const { return m_fov; }

    float near_plane_dist() const { return m_nearPlane; }

    float far_plane_dist() const { return m_farPlane; }

    // ------------------------------------------------------------------------
    // Setters
    // ------------------------------------------------------------------------
   
    void set_position(const glm::vec3& p) { m_position = p; }

    void set_pitch(float v) { 
        m_pitch = glm::clamp(v, MIN_PITCH_DEG, MAX_PITCH_DEG); 
        update();
    }

    void set_yaw(float v) {
        m_yaw = glm::mod(v, MAX_YAW_DEG);
        update();
    }

    /**
     * @brief Sets the properties of projection matrix.
     * @param aspect_ratio Screen aspect ratio.
     * @param fov Field of view, in degrees.
     * @param near_plane Distance to near_plane plane.
     * @param far_plane Distance to far_plane plane.
     */
    void set_proj_mat_props(float aspect_ratio, float fov, float near_plane, 
                            float far_plane)
    {
        m_aspectRatio = aspect_ratio;
        m_fov         = glm::radians(fov);
        m_nearPlane   = near_plane;
        m_farPlane    = far_plane;
    }

    void set_aspect_ratio(float as) { m_aspectRatio = as; }

    /**
     * @param fov Field of view in degrees */
    void set_field_of_view(float fov) { m_fov = fov; }

    void set_near_plane_dist(float dist) { m_nearPlane = dist; }

    void set_far_plane_dist(float dist) { m_farPlane = dist; }

    // ------------------------------------------------------------------------
    // Input handlers - control the camera
    // ------------------------------------------------------------------------
    
    void on_mouse_move(double x, double y);

    void on_mouse_button(int button, int action, int mods);

    void on_key_pressed(int key, int action);

    // TODO assumes GLFW_PRESS = 1, GLFW_RELEASE = 0
    void key_forward(int action)  { m_isForward = action; }
    void key_backward(int action) { m_isBackward = action; }
    void key_right(int action)    { m_isRight = action; }
    void key_left(int action)     { m_isLeft = action; }

    void key_reset(int action)
    { 
        m_firstCursor = true; 
        m_isForward = m_isBackward = m_isRight = m_isLeft = false;
    }

private:
    void update();

private:
    glm::vec3 m_position;         ///< Position of the camera
    glm::vec3 m_front;            ///< WHere the camera is looking at
    glm::vec3 m_right;            ///< Right vector of the camera
    glm::vec3 m_up;               ///< Up vector of the camera
    
    // Properties of projection matrix
    float m_aspectRatio, m_fov, m_nearPlane, m_farPlane;     

    // Looking in which direction in xz plane 
    //  0 degrees - looking in   -z direction
    //  90 degrees - looking in  -x direction
    //  180 degrees - looking in +z direction
    //  270 degrees - looking in +x direction
    float m_yaw;

    // positive ... looking from above the xz plane
    // negative ... looking from below the xz plane
    float m_pitch;
    
    int m_lastX, m_lastY;     ///< Last position of cursor on the screen
    bool m_firstCursor;      ///< First time the cursor is registered

    // Active movement states of the camera 
    bool m_isForward, m_isBackward, m_isRight, m_isLeft;

    // ------------------------------------------------------------------------
    // Constants - Defaults, maximums, etc.
    // ------------------------------------------------------------------------
    inline static const float DEFAULT_YAW_DEG       = 270.0f;
    inline static const float DEFAULT_PITCH_DEG     = 0.0f;
    inline static const float DEFAULT_SPEED         = 2.5f;

    inline static const float DEFAULT_FOV_DEG       = 45.0f;
    inline static const float DEFAULT_NEAR_PLANE    = 0.01f;
    inline static const float DEFAULT_FAR_PLANE     = 1000.0f;

    inline static const float MOUSE_SENSITIVITY     = 0.1f;
    inline static const float MOVE_SPEED            = 10.0f;

    inline static const float MAX_PITCH_DEG         = 89.0f;
    inline static const float MIN_PITCH_DEG         = -89.0f;
    inline static const float MAX_YAW_DEG           = 360.0f;

    inline static const glm::vec3 WORLD_UP          = glm::vec3(0.0f, 1.0f, 0.0f);
};

