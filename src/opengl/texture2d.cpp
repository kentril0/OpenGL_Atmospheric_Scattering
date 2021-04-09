/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file texture2d.cpp
 * @brief OpenGL 2D texture abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "texture2d.hpp"


Texture2D::Texture2D(bool mps)
    : m_width(0), 
      m_height(0), 
      m_internal_format(GL_RGBA8),
      m_image_format(GL_RGBA),
      m_mipmaps(mps),
      m_filterMin(GL_LINEAR_MIPMAP_LINEAR),
      m_filterMag(GL_LINEAR)
{
    DERR("Texture def CONSTR");

    init_texture();
}

Texture2D::Texture2D(const std::string& filename, bool alpha, bool mps)
    : m_width(0), 
      m_height(0), 
      m_internal_format(GL_RGBA8),
      m_image_format(alpha ? GL_RGBA : GL_RGB),
      m_mipmaps(mps),
      m_filterMin(GL_LINEAR_MIPMAP_LINEAR),
      m_filterMag(GL_LINEAR)
{
    DERR("Texture def CONSTR");

    init_texture();
    load(filename, alpha);
}

Texture2D::Texture2D(const uint8_t* data, uint32_t w, uint32_t h, bool alpha, bool mps)
    : m_width(w), 
      m_height(h), 
      m_internal_format(alpha ? GL_RGBA8 : GL_RGB8),
      m_image_format(alpha ? GL_RGBA : GL_RGB),
      m_mipmaps(mps),
      m_filterMin(GL_LINEAR_MIPMAP_LINEAR),
      m_filterMag(GL_LINEAR)
{
    DERR("Texture def CONSTR");

    init_texture();
    set_data_immutable(data);

    // Generate Mipmaps
    gen_mipmap();
}

Texture2D::~Texture2D()
{
    DERR("Texture def DESTR");

    glDeleteTextures(1, &m_id);
}

void Texture2D::load(const std::string& filename, bool alpha)
{
    int w, h, channels;
    unsigned char *data = load_image(filename.c_str(), alpha, &w, &h, &channels);

    // Save image dimensions
    m_width = w;
    m_height = h;

    set_data_immutable(data);

    free_image_data(data);

    // Generate Mipmaps
    gen_mipmap();
}

void Texture2D::upload(const uint8_t* data, int w, int h)
{
    // Save the dimensions
    m_width = w;
    m_height = h;

    bind();

    // Specify storage for all levels of a 2D array texture
    glTexImage2D(GL_TEXTURE_2D,         // texture type
  	             0,                     // level
  	             m_internal_format,     // sized internal format
  	             m_width, m_height,     // dimensions
                 0,                     // border
                 m_image_format,        // image format
                 GL_UNSIGNED_BYTE,      // image datatype
                 data);                 // pointer to the image data

    // Generate Mipmaps
    gen_mipmap();

    //unbind();
}

void Texture2D::upload(const float* data, int w, int h)
{
    // Save the dimensions
    m_width = w;
    m_height = h;

    bind();

    // Specify storage for all levels of a 2D array texture
    glTexImage2D(GL_TEXTURE_2D,         // texture type
  	             0,                     // level
  	             m_internal_format,     // sized internal format
  	             m_width, m_height,     // dimensions
                 0,                     // border
                 m_image_format,        // image format
                 GL_FLOAT,              // image datatype
                 data);                 // pointer to the image data

    // Generate Mipmaps
    gen_mipmap();

    //unbind();
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::bind_unit(uint32_t unit) const
{
    glBindTextureUnit(unit, m_id);
}

void Texture2D::set_repeat()
{
    set_custom_wrap(GL_REPEAT);
}

void Texture2D::set_mirrored_repeat()
{
    set_custom_wrap(GL_MIRRORED_REPEAT);
}

void Texture2D::set_clamp_to_edge()
{
	set_custom_wrap(GL_CLAMP_TO_EDGE);
}

void Texture2D::set_clamp_to_border(const glm::vec4& border_color)
{
	set_custom_wrap(GL_CLAMP_TO_BORDER);

#if OPENGL_VERSION >= 45
	glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
#else
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
#endif
}

void Texture2D::set_custom_wrap(uint32_t wrap)
{
#if OPENGL_VERSION >= 45
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap);	
#else
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);	
#endif
}

void Texture2D::set_custom_wrap(uint32_t wrap_s, uint32_t wrap_t)
{
#if OPENGL_VERSION >= 45
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap_t);	
#else
    bind();
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, wrap_s);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, wrap_t);	
#endif
}

void Texture2D::init_texture()
{
#if OPENGL_VERSION >= 45
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
#else
    glGenTextures(1, &m_id);
    bind();
#endif
}

void Texture2D::set_data_immutable(const uint8_t* data)
{
    // Specify IMMUTABLE storage for all levels of a 2D array texture
    //  Create texture with log2 of width levels
#if OPENGL_VERSION >= 45
    glTextureStorage2D(m_id,
                       (m_mipmaps ? std::log2(m_width) : 1),    // number of texture levels
                       m_internal_format,           // sized format of stored data *RGBA8
                       m_width, m_height);          // in texels

    // Specify a 2D texture subimage
    glTextureSubImage2D(m_id,               // texture id
                        0,                  // level
                        0, 0,               // xoffset, yoffset in the texture array
                        m_width, m_height,  // width, height of the subimage
                        m_image_format,     // format of the pixel data *RED, RGB, RGBA
                        GL_UNSIGNED_BYTE,   // data type of the pixel data, *BYTE, FLOAT, INT
                        data);              // A pointer to the image data in memory
#else
    bind();

    glTexStorage2D(GL_TEXTURE_2D,          // Target
                    (m_mipmaps ? std::log2(m_width) : 1),
                    m_internal_format,
                    m_width, m_height);

    glTexSubImage2D(GL_TEXTURE_2D,          // Target
                        0,
                        0, 0,
                        m_width, m_height,
                        m_image_format,
                        GL_UNSIGNED_BYTE,
                        data);
#endif
}

void Texture2D::set_filtering()
{
#if OPENGL_VERSION >= 45
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, m_filterMin);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, m_filterMag);
#else
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filterMag);
#endif
}

void Texture2D::gen_mipmap()
{
    if (!m_mipmaps)
        return; 

#if OPENGL_VERSION >= 45
	glGenerateTextureMipmap(m_id);
#else
    glGenerateMipmap(GL_TEXTURE_2D);
#endif

    set_filtering();
}

void Texture2D::set_filtering(uint32_t min_f, uint32_t mag_f)
{
    m_filterMin = min_f;
    m_filterMag = mag_f;

    set_filtering();
}

void Texture2D::set_linear_filtering()
{
    // Returns the weighted average of the four texture elements that are closest 
    //  to the specified texture coordinate
    m_filterMin = m_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    m_filterMag = GL_LINEAR;

    set_filtering();
}

void Texture2D::activate(uint32_t unit) const
{
    if (unit > 80)
        LOG_WARN("Going over of the ActiveTexture maximum units supported");

    glActiveTexture(GL_TEXTURE0 + unit);
}

