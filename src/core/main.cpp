/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file main.cpp
 * @brief Main file for init. of GLFW, OpenGL context, 
 *        ImGui and Application
 *********************************************************/

#include "pch.hpp"
#include "application.hpp"

#include <GLFW/glfw3.h>


// GLFW handler functions
void on_resize(GLFWwindow *window, int width, int height);

void on_mouse_move(GLFWwindow *window, double x, double y);

void on_mouse_pressed(GLFWwindow *window, int button, int action, int mods);

void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                    int mods);

#if OPENGL_VERSION >= 43
// Callback function to be called when OpenGL emits an error
void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, 
                                    GLsizei length, const char *message,
                                    const void *user_parameter);
#endif

int main(void)
{
    const size_t initial_width = SCREEN_INIT_WIDTH;
    const size_t initial_height = SCREEN_INIT_HEIGHT;

    // TODO logfile for errors

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return -1;
    }

    // Request OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    // Create window
    GLFWwindow *window = glfwCreateWindow(initial_width, initial_height, 
                                          PROJECT_NAME, NULL, NULL);
    if (!window)
    {
        std::cerr << "Could not create a window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make window's context current
    glfwMakeContextCurrent(window);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Could not initialize OpenGL context!" << std::endl;
        return -1;
    }

    LOG_INFO("Vendor: " << glGetString(GL_VENDOR));
    LOG_INFO("Renderer: " << glGetString(GL_RENDERER));
    LOG_INFO("OpenGL version: " << glGetString(GL_VERSION));
	LOG_INFO("GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#if OPENGL_VERSION >= 43
    glDebugMessageCallback(opengl_debug_callback, nullptr);
#endif

    //-------------------------------------------------------------------------
    // Setup Dear ImGUI context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGUI style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION_STR);
    //-------------------------------------------------------------------------

    {
        // Initialize the Application
        Application application(window, initial_width, initial_height);

        // Set window callbacks
        glfwSetWindowUserPointer(window, &application);
        glfwSetWindowSizeCallback(window, on_resize);
        glfwSetCursorPosCallback(window, on_mouse_move);
        glfwSetMouseButtonCallback(window, on_mouse_pressed);
        glfwSetKeyCallback(window, on_key_pressed);

        // Wait for the user to close the window
        while (!glfwWindowShouldClose(window))
        {
            // Application main loop
            application.loop();

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }   // Application is freed here, before GLFW

    //-------------------------------------------------------------------------
    // ImGUI cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();    
    //-------------------------------------------------------------------------
    
    // Free every allocated resource associated with GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void on_resize(GLFWwindow *window, int width, int height)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_resize(window, width, height);
}

void on_mouse_move(GLFWwindow *window, double x, double y)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_mouse_move(window, x, y);
}

void on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_mouse_pressed(window, button, action, mods);
}

void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                    int mods)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_key_pressed(window, key, scancode, action, mods);
}

#if OPENGL_VERSION >= 43
void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, 
                                    GLenum severity, GLsizei length, 
                                    const char *message,
                                    const void *user_parameter)
{
  switch (type)
  {
      case GL_DEBUG_TYPE_ERROR:
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
          std::cerr << message << std::endl;
          return;
      default:
          return;
  }
}
#endif
