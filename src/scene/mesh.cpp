/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file mesh.hpp
 * @brief Model representation in a scene
 *********************************************************/

#include "core/pch.hpp"
#include "mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


Mesh::Mesh(int32_t positionLoc,
           int32_t normalLoc  ,
           int32_t texCoordLoc)
  : m_vertices(0),
    m_indices(0),
    m_positionLoc(positionLoc),
    m_normalLoc(normalLoc),
    m_texCoordLoc(texCoordLoc),
    m_drawMode(GL_TRIANGLES)
{
}

Mesh::Mesh(const std::vector<float>& vertices,
           const std::vector<float>& normals,
           const std::vector<float>& texCoords,
           const std::vector<uint32_t>& indices,
           int32_t positionLoc,
           int32_t normalLoc  ,
           int32_t texCoordLoc)
  : m_vertices(0),
    m_indices(0),
    m_positionLoc(positionLoc),
    m_normalLoc(normalLoc),
    m_texCoordLoc(texCoordLoc),
    m_drawMode(GL_TRIANGLES)
{
    // Assumes triangles as draw primitive
    m_vertices = vertices.size() / 3;

    // buffer for vertices
    // buffer for normals
    // buffer for texcoords

    auto vboVertices = std::make_shared<VertexBuffer>(
         vertices.size() * sizeof(float), vertices.data());
    vboVertices->set_layout(BufferLayout({{ElementType::Float3, "position"}}));

    auto vboNormals = std::make_shared<VertexBuffer>(
         normals.size() * sizeof(float), normals.data());
    vboNormals->set_layout(BufferLayout({{ElementType::Float3, "normal"}}));

    auto vboTexels = std::make_shared<VertexBuffer>(
        texCoords.size() * sizeof(float), texCoords.data());
    vboTexels->set_layout(BufferLayout({{ElementType::Float2, "texCoord"}}));
 
    m_vao.add_vertex_buffer(vboVertices);
    m_vao.add_vertex_buffer(vboNormals);
    m_vao.add_vertex_buffer(vboTexels);

    if (!indices.empty())
    {
        m_indices = indices.size();
        auto ibo = std::make_shared<IndexBuffer>(m_indices, indices.data());
        m_vao.set_index_buffer(ibo);
    }
}

void Mesh::reinit_vao()
{
    // Assumes triangles as draw primitive
    m_vertices = m_verticesData.size() / 3;

    // buffer for vertices
    // buffer for normals
    // buffer for texcoords

    LOG_INFO("m_vert: " << m_verticesData.size());
    auto vboVertices = std::make_shared<VertexBuffer>(
         m_verticesData.size() * sizeof(float), m_verticesData.data());
    vboVertices->set_layout(BufferLayout({{ElementType::Float3, "position"}}));

    auto vboNormals = std::make_shared<VertexBuffer>(
         m_normals.size() * sizeof(float), m_normals.data());
    vboNormals->set_layout(BufferLayout({{ElementType::Float3, "normal"}}));

    auto vboTexels = std::make_shared<VertexBuffer>(
        m_texCoords.size() * sizeof(float), m_texCoords.data());
    vboTexels->set_layout(BufferLayout({{ElementType::Float2, "texCoord"}}));
 
    m_vao.clear();
    m_vao.add_vertex_buffer(vboVertices);
    m_vao.add_vertex_buffer(vboNormals);
    m_vao.add_vertex_buffer(vboTexels);

    if (!m_indicesData.empty())
    {
        m_indices = m_indicesData.size();
        LOG_INFO("indices: " << m_indices);
        auto ibo = std::make_shared<IndexBuffer>(m_indices, m_indicesData.data());
        m_vao.set_index_buffer(ibo);
    }
}

std::vector<std::unique_ptr<Mesh>> Mesh::from_file(const std::string& filename,
                                                   int32_t positionLoc,
                                                   int32_t normalLoc,
                                                   int32_t texCoordLoc)
{
    std::vector<tinyobj::shape_t> shapes;
    // TODO unused
    std::vector<tinyobj::material_t> materials;

    LOG_INFO("Loading object: " << filename);
    std::string err;
    bool succ = tinyobj::LoadObj(shapes, materials, err, filename.c_str());
    if (!succ)
    {
        LOG_ERR("Mesh: Could not load an object file using tinyobj.");
        return {};
    }

    std::vector<std::unique_ptr<Mesh>> meshes;

    // Over each shape that may be in the file
    //for (uint32_t s = 0; s < shapes)
    for (auto& s : shapes)
    {
        std::vector<float> vertices   = s.mesh.positions;
        std::vector<float> normals    = s.mesh.normals;
        std::vector<float> texCoords  = s.mesh.texcoords;
        std::vector<uint32_t> indices = s.mesh.indices;

        //meshes.push_back(Mesh(vertices, normals, texCoords, indices)); 
        meshes.push_back(std::make_unique<Mesh>()); 
        // TODO
        meshes.back()->m_verticesData = std::move(vertices);
        meshes.back()->m_normals = std::move(normals);
        meshes.back()->m_texCoords = std::move(texCoords);
        meshes.back()->m_indicesData = std::move(indices);
        meshes.back()->reinit_vao();
    }

    return meshes;
}

void Mesh::draw() const
{
    m_vao.bind();
    auto indexBuffer = m_vao.index_buffer();

    if (indexBuffer == nullptr)
        glDrawArrays(m_drawMode, 0, m_vertices);
    else
        glDrawElements(m_drawMode, m_indices, GL_UNSIGNED_INT, nullptr);
}
