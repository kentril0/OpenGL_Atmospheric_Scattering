/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file vertex_array.cpp
 * @brief OpenGL Vertex Array Object abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "vertex_array.hpp"


static GLenum element_to_shader_type(ElementType type)
{
    switch(type)
    {
        case ElementType::Float:     return GL_FLOAT;
        case ElementType::Float2:    return GL_FLOAT;
        case ElementType::Float3:    return GL_FLOAT;
        case ElementType::Float4:    return GL_FLOAT;
        case ElementType::Mat3:      return GL_FLOAT;
        case ElementType::Mat4:      return GL_FLOAT;
        case ElementType::Int:       return GL_INT;
        case ElementType::Int2:      return GL_INT;
        case ElementType::Int3:      return GL_INT;
        case ElementType::Int4:      return GL_INT;
        case ElementType::UInt8:     return GL_UNSIGNED_BYTE;
        case ElementType::UInt8_2:   return GL_UNSIGNED_BYTE;
        case ElementType::UInt8_3:   return GL_UNSIGNED_BYTE;
        case ElementType::UInt:      return GL_UNSIGNED_INT;
        case ElementType::UInt2:     return GL_UNSIGNED_INT;
        case ElementType::UInt3:     return GL_UNSIGNED_INT;
        case ElementType::Bool:      return GL_BYTE;
    }
    massert(false, "Unknown buffer element data type!");
    return 0;
}

VertexArray::VertexArray()
  : m_indexBuffer(nullptr)
{
#if OPENGL_VERSION >= 45
    glCreateVertexArrays(1, &m_id);
#else
    glGenVertexArrays(1, &m_id);
    bind();
#endif

    DERR("VAO default CONSTR: " << m_id);
}

VertexArray::~VertexArray()
{
    DERR("VAO default DESR");
    glDeleteVertexArrays(1, &m_id);

    // TODO Dangerous??
    clear_buffers();
    clear_index();
}

void VertexArray::bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::add_vertex_buffer(const std::shared_ptr<VertexBuffer>& vbo, 
                                    bool instanced)
{
    const auto& layout = vbo->layout();
    massert(layout.get_elements().size(), "Vertex buffer has no buffer elements!");

    bind();

    for (const auto& e : layout)
    {
        switch(e.type)
        {
            case ElementType::Float: 
            case ElementType::Float2:
            case ElementType::Float3:
            case ElementType::Float4:
            case ElementType::Int:   
            case ElementType::Int2:  
            case ElementType::Int3:  
            case ElementType::Int4:  
            case ElementType::UInt8:
            case ElementType::UInt8_2:
            case ElementType::UInt8_3:
            case ElementType::UInt: 
            case ElementType::UInt2: 
            case ElementType::UInt3: 
            case ElementType::Bool:  
            {
#if OPENGL_VERSION >= 45
                // TODO offset x relativeoffset
                glVertexArrayVertexBuffer(m_id, binding_index, vbo->ID(), 
                                          0, layout.get_stride());
                glEnableVertexArrayAttrib(m_id, binding_index);

                glVertexArrayAttribFormat(m_id, binding_index, 
                                          e.components_count(), 
                                          element_to_shader_type(e.type),
                                          e.normalized ? GL_TRUE : GL_FALSE, 
                                          e.offset);
                glVertexArrayAttribBinding(m_id, binding_index, binding_index);
#elif OPENGL_VERSION >= 43
                glBindVertexBuffer(binding_index, vbo->ID(), 
                                   0, layout.get_stride());            
                glEnableVertexAttribArray(binding_index);

                glVertexAttribFormat(binding_index, 
                                     e.components_count(), 
                                     element_to_shader_type(e.type),
                                     e.normalized ? GL_TRUE : GL_FALSE, 
                                     e.offset);
                glVertexAttribBinding(binding_index, binding_index);
#else
                vbo->bind();
                glVertexAttribPointer(binding_index, 
                                      e.components_count(), 
                                      element_to_shader_type(e.type),
                                      e.normalized ? GL_TRUE : GL_FALSE, 
                                      layout.get_stride(),
                                      (void*)e.offset);
                glEnableVertexAttribArray(binding_index);
#endif

                if (instanced)
                    glVertexAttribDivisor(binding_index, 1);

                binding_index++;
                break;
            }
            case ElementType::Mat3:  
            case ElementType::Mat4:  
            {
                uint8_t count = e.components_count();
                for (uint8_t i = 0; i < count; ++i)
                {
#if OPENGL_VERSION >= 45
                    glVertexArrayVertexBuffer(m_id, binding_index, vbo->ID(), 
                                              0, layout.get_stride());
                    glEnableVertexArrayAttrib(m_id, binding_index);

                    glVertexArrayAttribFormat(m_id, binding_index, 
                                              count, 
                                              element_to_shader_type(e.type),
                                              e.normalized ? GL_TRUE : GL_FALSE, 
                                              e.offset + i * count * sizeof(float));
                    glVertexArrayAttribBinding(m_id, binding_index, binding_index);
#elif OPENGL_VERSION >= 43
                    glBindVertexBuffer(binding_index, vbo->ID(), 
                                       0, layout.get_stride());            
                    glEnableVertexAttribArray(binding_index);

                    glVertexAttribFormat(binding_index, 
                                         count, 
                                         element_to_shader_type(e.type),
                                         e.normalized ? GL_TRUE : GL_FALSE, 
                                         e.offset + i * count * sizeof(float));
                    glVertexAttribBinding(binding_index, binding_index);
#else
                    vbo->bind();
                    glVertexAttribPointer(binding_index, 
                                          count, 
                                          element_to_shader_type(e.type),
                                          e.normalized ? GL_TRUE : GL_FALSE, 
                                          layout.get_stride(),
                                          (void*)(e.offset + i * count * sizeof(float)));

                    glEnableVertexAttribArray(binding_index);
#endif
                    // Per instance (GLSL vec4 limitation)
                    glVertexAttribDivisor(binding_index, 1);

                    binding_index++;
                }
                break;
            }
            default:
                massert(false, "Unknown buffer element data type!");
        }
    }

    m_vertexBuffers.push_back(vbo);
}

void VertexArray::set_index_buffer(const std::shared_ptr<IndexBuffer>& ibo)
{
#if OPENGL_VERSION >= 45
    glVertexArrayElementBuffer(m_id, ibo->ID());
#else
    bind();
    // reinit EBO here?
    ibo->bind();

#endif
    m_indexBuffer = ibo;
}

