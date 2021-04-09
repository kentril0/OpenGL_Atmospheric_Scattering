/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file shader.hpp
 * @brief OpenGL Shader program abstraction
 *********************************************************/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class Shader
{
public:
    /** @brief Default constructor, use function "compile" to supply 
     *         shader sources.
     */
    Shader() : m_id(0) {}

    /**
     * @brief Creates shaders and compiles a shader program
     * @param vert_src Contents of vertex shader source file
     * @param frag_src Contents of fragment shader source file
     * @param geom_src Contents of geometry shader source file
     */
    Shader(const char* vert_src, 
           const char* frag_src, 
           const char* geom_src = nullptr);

    /** @brief Creates shaders and compiles a shader program 
     *  @param See Shader constructor */
    void compile(const char* vert_src, 
                 const char* frag_src, 
                 const char* geom_src = nullptr);

    /** @brief Activate shader program */
    void use();

    /**
     * @brief Set float value of a uniform variable of an ACTIVE program
     * @param name Name of the uniform variable
     * @param value Value to be set
     */
    void set_float(const char* name,
                   float value);

    /**
     * @brief Set integer value of a uniform variable of an ACTIVE program
     * @param name Name of the uniform variable
     * @param value Value to be set
     */
    void set_int(const char* name,
                 int value);

    /**
     * @brief Set float value of a vec2 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param v0 Value to be set
     * @param v1 Value to be set
     */
    void set_vec2(const char* name,
                  float v0, float v1);

    /**
     * @brief Set float value of a vec2 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param value GLM vec2 value
     */
    void set_vec2(const char* name,
                  const glm::vec2& value);

    /**
     * @brief Set float value of a vec3 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param v0 Value to be set
     * @param v1 Value to be set
     * @param v2 Value to be set
     */
    void set_vec3(const char* name,
                  float v0, float v1, float v2);

    /**
     * @brief Set float value of a vec3 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param value GLM vec3 value
     */
    void set_vec3(const char* name,
                  const glm::vec3& value);

    /**
     * @brief Set float value of a vec4 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param v0 Value to be set
     * @param v1 Value to be set
     * @param v2 Value to be set
     * @param v3 Value to be set
     */
    void set_vec4(const char* name,
                  float v0, float v1, float v2, float v3);

    /**
     * @brief Set float value of a vec4 Uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param value GLM vec4 value
     */
    void set_vec4(const char* name,
                  const glm::vec4& value);
    /**
     * @brief Set float value of a matrix3 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param value GLM mat3 value 
     */
    void set_mat3(const char* name,
                  const glm::mat3& matrix);

    /**
     * @brief Set float value of a matrix4 uniform variable of an ACTIVE program
     * @param name Name of the variable
     * @param value GLM mat4 value 
     */
    void set_mat4(const char* name,
                  const glm::mat4& matrix);
 
private:
 
    /**
     * @brief Creates and compiles a shader
     * @param source Contents of the shader source file
     * @param type Type of the shader to be created
     * @return ID of the compiled shader object
     */
    uint32_t create_shader(const char* source, uint32_t type);

    /**
     * @brief Checks any compilation or link errors
     * @param object Either a shader or a shader program object
     * @param type Type of error to check for
     */
    void check_errors(uint32_t object, int type);

    /**
     * @brief Forces shader code to CURRENT OpenGL version according to 
     *  GLSL_VERSION_STR and GLSL_PROFILE defined in pch.hpp
     * @param code Loaded GLSL shader code
     * @return GLSL code with current OpenGL version
     */
    void fix_version(std::string& code);

private:

    uint32_t m_id;    ///< Program ID reference
};

