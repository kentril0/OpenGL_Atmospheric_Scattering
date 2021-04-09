/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file skybox.cpp
 * @brief Skybox abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "skybox.hpp"

#define CUBE_FACES 6


Skybox::Skybox(const std::shared_ptr<Shader>& sh, 
               const std::vector<const char*> faces, bool alpha)
  : m_shader(sh)
{
    // Loads the textures and sets them up as cubemap faces
    setup_cubemap(faces, alpha);

    // Sets up a VAO with unit cube vertices
    setup_vao();

    // Sets the samplerCube
    //shader.use();
    //shader.set_int("skybox", 0);
}

Skybox::~Skybox()
{
    glDeleteTextures(1, &m_id);
}


void Skybox::setup_cubemap(const std::vector<const char*>& faces, bool alpha)
{
    massert(faces.size() == CUBE_FACES, "Incorrect number of skybox faces");

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    // Load each face and setup its texture paramters
    int width, height, channels;
    for (uint8_t i = 0; i < faces.size(); ++i)
    {
        unsigned char *data = load_image(faces[i], alpha, &width, &height, &channels);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, 
                         (alpha == true ? GL_RGBA : GL_RGB), 
                         width, height, 
                         0, (alpha == true ? GL_RGBA : GL_RGB), 
                         GL_UNSIGNED_BYTE, data);
             
            free_image_data(data);
            continue;
        }

        LOG_ERR("Failed to load skybox texture: \"" << faces[i] << '"');
        free_image_data(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // TODO mipmaps

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    LOG_OK("Skybox was successfully loaded!");
}

void Skybox::setup_vao()
{
    const float vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    std::shared_ptr<VertexBuffer> vbo = 
        std::make_shared<VertexBuffer>(sizeof(vertices), vertices);

    vbo->set_layout(BufferLayout({
        {ElementType::Float3, "Position"}})
    );

    m_vao.add_vertex_buffer(vbo);
}

void Skybox::render(const glm::mat4& view, const glm::mat4& proj) const
{
    // Expects depth test to be run BEFORE

    // Changes depth function to render the skybox as the furthest object
    glDepthFunc(GL_LEQUAL);
    
    m_shader->use();

    // Remove the translation part from the view matrix
    glm::mat4 ts_view = glm::mat4(glm::mat3(view));

    m_shader->set_mat4("projview",  proj * ts_view);
    
    m_vao.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

