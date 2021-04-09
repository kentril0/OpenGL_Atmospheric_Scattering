/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file mesh.hpp
 * @brief Model representation in a scene
 *********************************************************/

#pragma once

#include "opengl/vertex_array.hpp"


class Mesh
{
public:
    Mesh(int32_t positionLoc = 0,
         int32_t normalLoc   = 1,
         int32_t texCoordLoc = 2);

    Mesh(const std::vector<float>& vertices, 
         const std::vector<float>& normals,
         const std::vector<float>& texCoords,
         const std::vector<uint32_t>& indices,
         int32_t positionLoc = 0,
         int32_t normalLoc   = 1,
         int32_t texCoordLoc = 2);

    /**
     * @brief Reads Meshes from a .OBJ file
     * @param filename Name of the file.obj
     * @param positionLoc Binding point of position attribute in shader
     * @param normalLoc Binding point of normal attribute in shader
     * @param texCoordLoc Binding point of texture coordinate attribute in shader
     * @return Vector of unique ptrs to loaded Meshes (to not copy constr)
     */
    static std::vector<std::unique_ptr<Mesh>> from_file(
        const std::string& filename,
        int32_t positionLoc = 0,
        int32_t normalLoc   = 1,
        int32_t texCoordLoc = 2);


    /** @brief Binds VAO and issues a draw call */
    void draw() const;

    void reinit_vao();

    void resize();

    uint32_t vertices() const { return m_vertices; }
    uint32_t indices() const { return m_indices; }

private:
    uint32_t m_vertices;
    uint32_t m_indices;

    int32_t m_positionLoc = -1;
    int32_t m_normalLoc   = -1;
    int32_t m_texCoordLoc = -1;

    VertexArray m_vao;
    // TODO needed??
    std::vector<float> m_verticesData;
    std::vector<float> m_normals;
    std::vector<float> m_texCoords;
    std::vector<uint32_t> m_indicesData;

    uint32_t m_drawMode;
};

