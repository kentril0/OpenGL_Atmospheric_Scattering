/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file buffer.hpp
 * @brief OpenGL Vertex Buffer Object and Index Buffer
 *        Object abstractions
 *********************************************************/

#pragma once

#include <cstdint>
#include <vector>


#define ALIGNED_4 4

/**
 * @brief Provides an interface for working with OpenGL vertex buffer objects 
 *  (VBOs) (and index buffer). Each VBO consists of a buffer layout 
 *  (class BufferLayout), that is the description how the data is organized 
 *  into buffer elements (BufferElement) (e.g., position, color, ...). 
 *  Each buffer element is of a data type, called ElementType (e.g. position: 
 *  Float3 (which is vec3), color: Float4 (vec4), ...).
 *
 *  Such buffers can be later seamlessly passed to VertexArray interface to
 *  define Vertex Array Objects.
 *
 * Usage examples:
 *  1) Buffer of vertices:
 *
 *      VertexBuffer vbo(sizeof(vertices), vertices);
 *      {
 *          BufferLayout layout = {
 *              { ElementType::Float3, "Position"}
 *          };
 *          vbo.set_layout(layout);
 *      }
 *
 *  2) Interleaved buffer of vertices and colors:
 *
 *      VertexBuffer vbo(sizeof(data), data);
 *      vbo.set_layout(BufferLayout({
 *          { ElementType::Float3, "Position"},
 *          { ElementType::Float4, "Color"}})
 *      );
 *
 *  3) Interleaved buffer of struct { vec3 Vertex; uint8_t3 color; }:
 *
 *      VertexBuffer vbo(sizeof(data), data);
 *      vbo.set_layout(BufferLayout({
 *          { ElementType::Float3, "Position"},
 *          { ElementType::UInt8_3, "Color", true, offsetof(Vertex, color)}
 *          }, true)    // true for unpacked data
 *      );
 *
 *  4) Index buffer:
 *      
 *      IndexBuffer ibo(sizeof(indices) / sizeof(uint32_t), indices);
 *
 *  Note: deinterleaved variant is not supported? TODO 
 *        just break it into more VBOs
 */


// ----------------------------------------------------------------------------
// Buffer Layouts for VAO
// ----------------------------------------------------------------------------
/**
 * @brief Data type of elements in buffers
 */
enum class ElementType 
{
    Bool = 0,
    Float, Float2, Float3, Float4,
    Mat3, Mat4,
    Int, Int2, Int3, Int4, 
    UInt8, UInt8_2, UInt8_3, 
    UInt, UInt2, UInt3
};

/**
 * @brief Each buffer consists of elements, e.g. Position, Color, ...
 */
struct BufferElement
{
    ElementType type;
    uint32_t offset;
    bool normalized;

    BufferElement() = default;

    /**
     * @brief Initializes the buffer element.
     * @param type Type of the element (e.g., vertex: ElementType::Float3).
     * @param desc Description of the element (e.g. "Position"), only as a note.
     * @param normalized Whether the components of the element should be normalized.
     * @param offset Leave at 0 or supply custom element offset.
     */
    BufferElement(ElementType type, const char* desc, bool normalized = false, 
                  uint32_t offset = 0)
      : type(type), offset(offset), normalized(normalized)
    { 
        (void)desc;
    }

    /**
     * @brief Size of element data type in bytes
     */
    inline uint32_t element_type_size() const
    {
        switch(type)
        {
            case ElementType::Float:     return 4;
            case ElementType::Float2:    return 4 * 2;
            case ElementType::Float3:    return 4 * 3;
            case ElementType::Float4:    return 4 * 4;
            case ElementType::Mat3:      return 4 * 3 * 3;
            case ElementType::Mat4:      return 4 * 4 * 4;
            case ElementType::Int:       return 4;
            case ElementType::Int2:      return 4 * 2;
            case ElementType::Int3:      return 4 * 3;
            case ElementType::Int4:      return 4 * 4;
            case ElementType::UInt8:     return 1;
            case ElementType::UInt8_2:   return 1 * 2;
            case ElementType::UInt8_3:   return 1 * 3;
            case ElementType::UInt:      return 4 * 2;
            case ElementType::UInt2:     return 4 * 2;
            case ElementType::UInt3:     return 4 * 3;
            case ElementType::Bool:      return 1;
        }
        massert(false, "Unknown buffer element data type!");
        return 0;
    }

    /**
     * @return Number of components in an element
     */
    inline uint32_t components_count() const
    {
        switch(type)
        {
            case ElementType::Float:     return 1;
            case ElementType::Float2:    return 2;
            case ElementType::Float3:    return 3;
            case ElementType::Float4:    return 4;
            case ElementType::Mat3:      return 3;  // 3 * Float3
            case ElementType::Mat4:      return 4;  // 4 * Float4
            case ElementType::Int:       return 1;
            case ElementType::Int2:      return 2;
            case ElementType::Int3:      return 3;
            case ElementType::Int4:      return 4;
            case ElementType::UInt8:     return 1;
            case ElementType::UInt8_2:   return 2;
            case ElementType::UInt8_3:   return 3;
            case ElementType::UInt:      return 1;
            case ElementType::UInt2:     return 2;
            case ElementType::UInt3:     return 3;
            case ElementType::Bool:      return 1;
        }
        massert(false, "Unknown buffer element data type!");
        return 0;
    }
};

/**
 * @brief Each buffer consists of a layout, which is defined by
 *        buffer elements.
 */
class BufferLayout
{
public:
    BufferLayout() { }

    /**
     * @brief TODO
     * @param elements
     * @param unpacked Whether the elements are unpacked
     */
    BufferLayout(const std::initializer_list<BufferElement>& elements, bool unpacked = false) 
        : elements(elements), unpacked(unpacked)
    {
        calc_offset_and_stride();
    }

    const std::vector<BufferElement>& get_elements() const { return elements; }

    uint32_t get_stride() const { return stride; }

    std::vector<BufferElement>::iterator begin() { return elements.begin(); }
    std::vector<BufferElement>::iterator end() { return elements.end(); }
    std::vector<BufferElement>::const_iterator begin() const { return elements.begin(); }
    std::vector<BufferElement>::const_iterator end() const { return elements.end(); }
private:
    /** @brief (Re)calculates offset of elements and stride */
    void calc_offset_and_stride()
    {
        uint32_t offset = 0;
        stride = 0;
        for (auto& e : elements)
        {
            bool custom_offset = e.offset != 0;
            uint32_t e_size = e.element_type_size();

            if (!custom_offset)
            {
                e.offset = offset;
                offset += e_size;
                stride += e_size;
            }
            else
            {
                offset = e.offset + e_size;
                stride = e.offset + e_size;
            }

            if (unpacked)
            {
                uint32_t rm = offset % ALIGNED_4;
                offset += rm != 0 ? ALIGNED_4 - rm : 0;

                rm = stride % ALIGNED_4;
                stride += rm != 0 ? ALIGNED_4 - rm : 0;
            }
        }
    }

    std::vector<BufferElement> elements;
    bool unpacked;
    uint32_t stride = 0;
};


// ----------------------------------------------------------------------------
// Buffers Interface
// ----------------------------------------------------------------------------
class VertexBuffer
{
public:
    /**
     * @brief Creates the buffer object.
     */
    VertexBuffer();

    /**
     * @brief Creates the buffer object and initializes with empty data.
     *        Expects data to be uploaded later on.
     *        By default IMMUTABLE! (i.e. does not move in memory)
     * @param size Size of the data to be reserved.
     * @param immutable Whether the buffer is considered immutable 
     *        (i.e. does not move in memory).
     */
    VertexBuffer(uint32_t size, 
                 bool immutable = true);

    /**
     * @brief Creates the buffer object and initializes with data.
     *        Expects data to be uploaded ONLY ONCE (when not immutable)! TODO
     *        By default IMMUTABLE! (i.e. does not move in memory)
     * @param size Size of the data to be reserved.
     * @param data Data to be uploaded.
     * @param immutable Whether the buffer is considered immutable.
     */
    VertexBuffer(uint32_t size, 
                 const void* data,
                 bool immutable = true); 

    ~VertexBuffer();

    void bind() const;
    void unbind() const;

    /**
     * @brief Uploads new data to already initialized buffer of size and at offset.
     * @param size Size of the data to be uploaded in bytes.
     * @param data New data to be uploaded.
     * @param offset Where the replacement will begin in the data store.
     */
    void set_data(uint32_t size,
                  const void* data,
                  int32_t offset = 0) const;

    /**
     * @brief Reallocates the buffer with potentially different size.
     *        Expects no data updates, but possibly another reallocation in the future.
     * @param size Size of the data in bytes.
     * @param data Data to fill in.
     * @param updates True to enable additional updates.
     */
    void reallocate(uint32_t size, const void* data, bool updates = false) const;

    /**
     * @brief Set layout, a description of data that consists of buffer elements.
     * @param layout BufferLayout type
     */
    void set_layout(const BufferLayout& layout) { m_layout = layout; }

    uint32_t            ID() const { return m_id; }
    const BufferLayout& layout() const { return m_layout; }

private:
    void init_buffer();


private:
    uint32_t m_id;
    BufferLayout m_layout;
};


class IndexBuffer
{
public:
    /**
     * @brief Creates and index buffer and supplies the data, expected to be immutable.
     * @param count Number of indices supplied.
     * @param indices Array of indices.
     */
    IndexBuffer(uint32_t count, const uint32_t* indices);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;

    uint32_t ID() const { return m_id; };
    uint32_t count() const { return m_count; };

private:
    uint32_t m_id;
    uint32_t m_count;
};


