/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file camera.cpp
 * @brief FPS Camera abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "core/application.hpp"


// Indicies to array of active move states
#define KEY_FORWARD     GLFW_KEY_W
#define KEY_BACKWARD    GLFW_KEY_S
#define KEY_RIGHT       GLFW_KEY_D
#define KEY_LEFT        GLFW_KEY_A


Camera::Camera(float aspect_ratio, const glm::vec3& pos, const glm::vec3& up,
               const glm::vec3& front,
               float yaw, float pitch)
  : m_position(pos), 
    m_front(front),
    m_up(up), 
    m_aspectRatio(aspect_ratio),
    m_fov(DEFAULT_FOV_DEG),
    m_nearPlane(DEFAULT_NEAR_PLANE), m_farPlane(DEFAULT_FAR_PLANE),
    m_yaw(yaw), m_pitch(pitch),
    m_lastX(0), m_lastY(0),
    m_firstCursor(true),
    m_isForward(false),
    m_isBackward(false),
    m_isRight(false),
    m_isLeft(false)
{

}

void Camera::on_mouse_move(double x, double y)
{
    // First time the cursor entered the screen, prevents jumps
    if (m_firstCursor)
    {
        m_lastX = static_cast<int>(x);
        m_lastY = static_cast<int>(y);
        m_firstCursor = false;
    }

    // Calculate the offset movement between the last and current frame
    float dx = float(x - m_lastX) * MOUSE_SENSITIVITY;
    float dy = float(m_lastY - y) * MOUSE_SENSITIVITY;

    m_lastX = static_cast<int>(x);
    m_lastY = static_cast<int>(y);

    m_pitch += dy;
    m_pitch = glm::clamp(m_pitch, MIN_PITCH_DEG, MAX_PITCH_DEG);
    
    m_yaw = glm::mod(m_yaw + dx, MAX_YAW_DEG);

    update();
}

void Camera::update()
{
    // calculate the new front vector
    m_front.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
    m_front.y = sinf(glm::radians(m_pitch));
    m_front.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));

    m_front = glm::normalize(m_front);

    // re-calculate the right and up vector
    // normalize because their length gets closer to 0 the more you look up or down
    //  -> results in slower movement
    m_right = glm::normalize(glm::cross(m_front, WORLD_UP));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::update(float dt)
{
    float velocity = MOVE_SPEED * dt;
    m_position += m_isForward  * m_front * velocity;
    m_position -= m_isBackward * m_front * velocity;
    m_position += m_isRight    * m_right * velocity;
    m_position -= m_isLeft     * m_right * velocity;
}

void Camera::on_mouse_button(int button, int action, int mods)
{

}

void Camera::on_key_pressed(int key, int action)
{
    if (action == GLFW_PRESS)
    {           
        if (key == KEY_CAM_FORWARD)
            m_isForward = true;
        else if (key == KEY_CAM_BACKWARD)
            m_isBackward = true;
        else if (key == KEY_CAM_RIGHT)
            m_isRight = true;
        else if (key == KEY_CAM_LEFT)
            m_isLeft = true;
        else if (key == KEY_CAM_RCURSOR)
        {
            m_isForward = m_isBackward = m_isRight = m_isLeft = false;
            m_firstCursor = true;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (key == KEY_CAM_FORWARD)
            m_isForward = false;
        else if (key == KEY_CAM_BACKWARD)
            m_isBackward = false;
        else if (key == KEY_CAM_RIGHT)
            m_isRight = false;
        else if (key == KEY_CAM_LEFT)
            m_isLeft = false;        
    }
}

