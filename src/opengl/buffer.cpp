/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file buffer.cpp
 * @brief OpenGL Vertex Buffer Object and Index Buffer
 *        Object abstractions
 *********************************************************/

#include "core/pch.hpp"
#include "buffer.hpp"


VertexBuffer::VertexBuffer()
{
    init_buffer();
}

VertexBuffer::VertexBuffer(uint32_t size, bool immutable)
{
    init_buffer();

    // Dynamic - for data to be uploaded later
#if OPENGL_VERSION >= 45
    if (immutable)
        glNamedBufferStorage(m_id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    else
        glNamedBufferData(m_id, size, nullptr, GL_DYNAMIC_DRAW);
#elif OPENGL_VERSION >= 44
    bind();

    if (immutable)
        glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    else
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#else
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#endif
}

VertexBuffer::VertexBuffer(uint32_t size, const void* data, bool immutable)
{
    init_buffer();

#if OPENGL_VERSION >= 45
    if (immutable)
    {
        // Only dynamic for storage
        glNamedBufferStorage(m_id, size, data, GL_DYNAMIC_STORAGE_BIT);
    }
    else
        glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
#elif OPENGL_VERSION >= 44
    bind();

    if (immutable)
        glBufferStorage(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_STORAGE_BIT);
    else
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
#else
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
#endif
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::set_data(uint32_t size, const void* data, int32_t offset) const
{
#if OPENGL_VERSION >= 45
    glNamedBufferSubData(m_id, offset, size, data);
#else
    bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
#endif
}

void VertexBuffer::reallocate(uint32_t size, const void* data, bool updates) const
{
#if OPENGL_VERSION >= 45
    if (updates)
        glNamedBufferData(m_id, size, data, GL_DYNAMIC_DRAW);
    else
        glNamedBufferData(m_id, size, data, GL_STATIC_DRAW);
#else
    bind();
    if (updates)
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
#endif
}

// ----------------------------------------------------------------------------
// Index Buffer
// ----------------------------------------------------------------------------

IndexBuffer::IndexBuffer(uint32_t count, const uint32_t* indices)
  : m_count(count)
{
#if OPENGL_VERSION >= 45
    glCreateBuffers(1, &m_id);
#else
    glGenBuffers(1, &m_id);
    bind();
#endif

    // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
#if OPENGL_VERSION >= 45
    glNamedBufferStorage(m_id, count * sizeof(uint32_t), indices, GL_DYNAMIC_STORAGE_BIT);
#elif OPENGL_VERSION >= 44
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, 
                    GL_DYNAMIC_STORAGE_BIT);
#else
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
#endif
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void IndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// ----------------------------------------------------------------------------
// Util
// ----------------------------------------------------------------------------
void VertexBuffer::init_buffer()
{
#if OPENGL_VERSION >= 45
    glCreateBuffers(1, &m_id);
#else
    glGenBuffers(1, &m_id);
#endif
}

