/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file shader.cpp
 * @brief OpenGL Shader program abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "shader.hpp"


// Types of errors to check 
#define COMPILE_ERRORS 1
#define LINK_ERRORS    2


Shader::Shader(const char *vert_src, const char *frag_src, const char *geom_src)
{
    compile(vert_src, frag_src, geom_src);
}

void Shader::use()
{
    glUseProgram(m_id);
    //return *this;
}

uint32_t Shader::create_shader(const char *source, uint32_t type)
{
    std::string text = load_file(source);
    const char* text_c = text.data();

    fix_version(text);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &text_c, nullptr);
    glCompileShader(shader);

    // Check any compilation errors
    check_errors(shader, COMPILE_ERRORS);

    return shader;
}

void Shader::compile(const char *vert_src, const char *frag_src, const char *geom_src)
{
    LOG_INFO("Compiling sources: " << vert_src << ' ' << frag_src);
    GLuint sh_vert, sh_frag, sh_geom;

    // Vertex shader
    sh_vert = create_shader(vert_src, GL_VERTEX_SHADER);
    // Fragment shader
    sh_frag = create_shader(frag_src, GL_FRAGMENT_SHADER);
    // Geometry shader
    if (geom_src != nullptr)
        sh_geom = create_shader(geom_src, GL_GEOMETRY_SHADER);
    
    // Create program
    m_id = glCreateProgram();

    // Attach shaders
    glAttachShader(m_id, sh_vert);
    glAttachShader(m_id, sh_frag);
    if (geom_src != nullptr)
        glAttachShader(m_id, sh_geom);

    // Link the program and check for any link errors
    glLinkProgram(m_id);
    check_errors(m_id, LINK_ERRORS);

    // Clean up after linked, no longer needed
    glDeleteShader(sh_vert);
    glDeleteShader(sh_frag);
    if (geom_src != nullptr)
        glDeleteShader(sh_geom);

    glDetachShader(m_id, sh_vert);
    glDetachShader(m_id, sh_frag);
    if (geom_src != nullptr)
        glDetachShader(m_id, sh_geom);

    LOG_INFO("OK");
}

void Shader::set_float(const char *name, float value)
{
    glUniform1f(glGetUniformLocation(m_id, name), value);
}

void Shader::set_int(const char *name, int value)
{
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void Shader::set_vec2(const char *name, float v0, float v1)
{
    glUniform2f(glGetUniformLocation(m_id, name), v0, v1);
}

void Shader::set_vec2(const char *name, const glm::vec2 &value)
{
    glUniform2f(glGetUniformLocation(m_id, name), value.x, value.y);
}

void Shader::set_vec3(const char *name, float v0, float v1, float v2)
{
    glUniform3f(glGetUniformLocation(m_id, name), v0, v1, v2);
}

void Shader::set_vec3(const char *name, const glm::vec3 &value)
{
    glUniform3f(glGetUniformLocation(m_id, name), value.x, value.y, value.z);
}

void Shader::set_vec4(const char *name, float v0, float v1, float v2, float v3)
{
    glUniform4f(glGetUniformLocation(m_id, name), v0, v1, v2, v3);
}

void Shader::set_vec4(const char *name, const glm::vec4 &value)
{
    glUniform4f(glGetUniformLocation(m_id, name), 
                value.x, value.y, value.z, value.w);
}

void Shader::set_mat3(const char *name, const glm::mat3 &matrix)
{
    glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, false, 
                       glm::value_ptr(matrix));
}

void Shader::set_mat4(const char *name, const glm::mat4 &matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, false, 
                       glm::value_ptr(matrix));
}

void Shader::check_errors(uint32_t object, int type)
{
    int success;
    const unsigned int log_size = 1024;
    char log[log_size];

    if (type == COMPILE_ERRORS)
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, log_size, NULL, log);
            LOG_ERR("| Error::Shader: Compile-time error: " 
                << "\n" << log 
                << "\n ------------------------------------------------------ "
            );
        }
    }
    else if (type == LINK_ERRORS)
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, log_size, NULL, log);
            LOG_ERR("| Error::Shader: Link-time error: " 
                << "\n" << log 
                << "\n ------------------------------------------------------ "
            );
        }
    }
    else
    {
        massert(false, "Incorrect Shader error type");
    }
}

void Shader::fix_version(std::string& code)
{
    std::string ver_str = GLSL_VERSION_STR;
    ver_str += GLSL_PROFILE;

    // Version string is present -> need to set to current version
    if (code[0] == '#')
    {
        // Replace until end of the line
        size_t endl = code.find('\n');
        if (endl != std::string::npos)
            code.replace(0, endl, ver_str);
        else
            code.replace(0, ver_str.length(), ver_str);
    }
    else
    {
        // Prepend the version string
        ver_str += '\n';
        code.replace(0, 0, ver_str);
    }
}

