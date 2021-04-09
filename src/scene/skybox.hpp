/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file skybox.hpp
 * @brief Skybox abstraction
 *********************************************************/

#pragma once

#include "opengl/shader.hpp"
#include "opengl/vertex_array.hpp"


class Skybox
{
public:
    /**
     * @brief Loads and sets up skybox faces and VAO for rendering.
     * @param sh Shader program used to render the skybox.
     * @param faces A vector (or initializer list) of 6 texture paths for each face
     *              of the cubemap.
     * @param alpha Whether the textures of the faces have alpha channel.
     */
    Skybox(const std::shared_ptr<Shader>& sh, 
           const std::vector<const char*> faces, bool alpha = false);

    ~Skybox();

    /**
     * @brief Renders the cubemap using its shader and VAO.
     * @param view Camera view matrix.
     * @param proj Camera projection matrix.
     */
    void render(const glm::mat4& view, const glm::mat4& proj) const;

private:    

    void setup_cubemap(const std::vector<const char*>& faces, bool alpha);

    void setup_vao();

private:    
    uint32_t m_id;

    std::shared_ptr<Shader> m_shader;
    VertexArray m_vao;
};
