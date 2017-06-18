#ifndef _cms_mesh_included_1872356784325143058634561237512030785203751238546234
#define _cms_mesh_included_1872356784325143058634561237512030785203751238546234

#include <vector>
#include <string>
#include <fstream>
#include <limits>

#include "log.hpp"

namespace cms
{


struct Mesh
{
    std::vector<float> m_vertices;
    std::vector<float> m_normals;
    std::vector<unsigned int> m_indices;

  
    Mesh() {};
    Mesh(std::vector<float> i_vertices, std::vector<unsigned int> i_indices) 
        : m_vertices(i_vertices), m_indices(i_indices)
    {}

    Mesh(std::vector<float> i_vertices, std::vector<float> i_normals, std::vector<unsigned int> i_indices)
        : m_vertices(i_vertices), m_normals(i_normals), m_indices(i_indices)
    {}
  
    inline void setVertices(std::vector<float> i_vertices)                                  // Accessors and Mutators
        { m_vertices = i_vertices; }
    inline std::vector<float> getVertices() const
        { return m_vertices; }

    inline void setIndices(std::vector<unsigned int> i_indices)
        { m_indices = i_indices; }
    inline std::vector<unsigned int> getIndices() const
        { return m_indices; }

    inline void setNormals(std::vector<float> i_normals)
        { m_normals = i_normals; }
    inline std::vector<float> getNormals() const
        { return m_normals; }

    
    unsigned int vertexCount() const                                                        // Returns the number of vertices in the mesh
        { return m_vertices.size() / 3; }
  
    unsigned int indexCount() const                                                         // Returns the number of indices in the mesh
        { return m_indices.size(); }
  
    unsigned int faceCount() const                                                          // Returns the number of faces in the mesh /triangles/
        { return m_indices.size() / 3; }

    
    void pushVertex(float i_x, float i_y, float i_z)                                        // push vertex on the end of the mesh vertex vector
    {
        m_vertices.push_back(i_x);
        m_vertices.push_back(i_y);
        m_vertices.push_back(i_z);
    }

  
    void pushNormal(float i_x, float i_y, float i_z)                                        // push normal on the end of the mesh normal vector
    {
        m_normals.push_back(i_x);
        m_normals.push_back(i_y);
        m_normals.push_back(i_z);
    }

    
    void pushIndex(int i_ind)                                                               // push index on the end of the mesh index vector
        { m_indices.push_back(i_ind); }

    // Populates a float array with the extreme coordinates of the mesh
    // in the following format: {+x +y +z -x -y -z}, takes in a pointer to an array of 6 floats
    void getBoundingBox(float* i_bbox) const
    {
        // Assigning the bbox to some improbable limits
        i_bbox[0] = i_bbox[1] = i_bbox[2] =  std::numeric_limits<float>::max();
        i_bbox[3] = i_bbox[4] = i_bbox[5] = -std::numeric_limits<float>::max();

        
        for(unsigned int i = 0; i < m_vertices.size(); i += 3)                              // X limits
        {
            i_bbox[0] = std::min(i_bbox[0], m_vertices[i]);
            i_bbox[3] = std::max(i_bbox[3], m_vertices[i]);
        }
        
        for(unsigned int i = 1; i < m_vertices.size(); i += 3)                              // Y limits
        {
            i_bbox[1] = std::min(i_bbox[1], m_vertices[i]);
            i_bbox[4] = std::max(i_bbox[4], m_vertices[i]);
        }

        for(unsigned int i = 2; i < m_vertices.size(); i += 3)                              // Z limits
        {
            i_bbox[2] = std::min(i_bbox[2], m_vertices[i]);
            i_bbox[5] = std::max(i_bbox[5], m_vertices[i]);
        }
    }

    // Exports the mesh as a Wavefront OBJ file
    void exportOBJ(const std::string &i_fName) const
    {
        std::string fullPath = i_fName;

        std::fstream fileOut;
        fileOut.open(fullPath.c_str(), std::ios::out);

        fileOut << "# CMS Isosurface extraction." << std::endl;

        for(unsigned int i = 0; i < m_vertices.size(); i += 3)
        {
            fileOut << "v " << m_vertices[i] << " " << m_vertices[i + 1] << " " << m_vertices[i + 2] << std::endl;
        }

        // Write the face info
        for(unsigned f = 0; f < m_indices.size(); f += 3)
        {
            fileOut << "f " << m_indices[f] + 1 << " " << m_indices[f + 1] + 1 << " " << m_indices[f + 2] + 1 << std::endl;
        }

        debug_msg("Exported mesh path: %s", fullPath.c_str());
    }

};


} // namespace cms

#endif // _cms_mesh_included_1872356784325143058634561237512030785203751238546234