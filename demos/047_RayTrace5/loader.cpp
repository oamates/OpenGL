#include <cstdio>
#include <vector>
#include <map>
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>
#include <iomanip>
    
#include "loader.hpp"
#include "log.hpp"
#include "vertex.hpp"
#include "image.hpp"

//===================================================================================================================================================================================================================
// Auxiliary structure, needed at the time of obj loading only
//===================================================================================================================================================================================================================
struct pnt_index
{
    GLushort p, n, t;

    pnt_index() {};
    pnt_index(GLushort p, GLushort n, GLushort t) : p(p), n(n), t(t) {};

    friend bool operator < (const pnt_index& lhs, const pnt_index& rhs)                     // for std::map construction to work
    {
        if (lhs.p < rhs.p) return true;
        if (lhs.p > rhs.p) return false;
        if (lhs.n < rhs.n) return true;
        if (lhs.n > rhs.n) return false;
        if (lhs.t < rhs.t) return true;
        return false;
    }
};

struct pn_index
{
    GLushort p, n;

    pn_index(GLushort p, unsigned int n) : p(p), n(n) {};

    friend bool operator < (const pn_index& lhs, const pn_index& rhs)                       // for std::map construction to work
    {
        if (lhs.p < rhs.p) return true;
        if (lhs.p > rhs.p) return false;
        if (lhs.n < rhs.n) return true;
        return false;
    }
};


std::istream& operator >> (std::istream& is, glm::vec3& v)
{
    is >> v.x >> v.y >> v.z;
    return is;
};

std::istream& operator >> (std::istream& is, glm::vec2& v)
{
    is >> v.x >> v.y;
    return is;
};


//===================================================================================================================================================================================================================
// The function tries to read index triple from a given input stream, converts relative (= negative) indices to absolute,
// and checks that the triple does not break pnt_max boundary. 
// Note: only position attribute index is absolutely required, texture coordinate and normal index can be omitted. Value -1 indicates this case.
//===================================================================================================================================================================================================================

bool read_pnt_index(std::stringstream& is, pnt_index& index, const pnt_index& pnt_max)                
{           
    index.n = -1;                                                           // Valid index triples are (no spaces or tabs is allowed)  
    index.t = -1;                                                           // 1. p     --- just a position                            
                                                                            // 2. p//n  --- position + normal index                    
                                                                            // 3. p/t   --- position + texture coordinate              
                                                                            // 4. p/t/n --- complete index triple                      
    if (!(is >> index.p) || (index.p == 0)) return false;                   // indices in obj-file begin from 1, 0 is invalid value
    if (index.p > 0)
        index.p--;
    else
        index.p = pnt_max.p + index.p;                                      // negative indices are relative to the current array length

    if ((index.p < 0) || (index.p >= pnt_max.p)) return false;                                         
                                                                            
    int slash = is.get();

    if (slash != '/') return true;                                          // we got only position index : p, triple successfully read (case 1)
                                                                            // .. so we just got slash after position index, check the next character
    if (is.peek() != '/')                                                   // ok, this next character is not a slash separator, so it must be texture coordinate index
    {
        if (!(is >> index.t) || (index.t == 0)) return false;               // something went wrong or invalid value 0 was read ... report failure

        if (index.t > 0)                                                    // convert texture coordinate index
            index.t--;
        else
            index.t = pnt_max.t + index.t;                                  

        if ((index.t < 0) || (index.t >= pnt_max.t)) return false;          // and check that it lies withing the boundaries

        slash = is.get(); 
        if (slash != '/') return true;                                      // triple successfully read (case 3)
    }
    else                                                                    // two slashes in a row, so the input must be of the form p//n
        is.get();                                                           // read the slash from stream
                                                                            // now read the normal index
    if (!(is >> index.n) || (index.n == 0)) return false;                   // something went wrong or invalid value 0 was read ... report failure

    if (index.n > 0)                                                        // convert normal index
        index.n--;
    else
        index.n = pnt_max.n + index.n;                                  

    if ((index.n < 0) || (index.n >= pnt_max.n)) return false;              // and check that it lies withing the boundaries
    return true;                                                            // triple successfully read (case 2 or case 4)
                                                                            // hooray, it is done 
}


bool loadMesh_assimp(
	const char* filename,
	std::vector<GLushort>& indices_out,
	std::vector<glm::vec3> & positions_out, 
	std::vector<glm::vec2> & uvs_out, 
	std::vector<glm::vec3> & normals_out)
{
    std::ifstream input_stream(filename);                                  // ok, let us begin by opening the input file
    if (!input_stream)
        exit_msg("Cannot open object file %s.", filename);

    std::vector<glm::vec3> positions;                                       
    std::vector<glm::vec3> normals;                                         
    std::vector<glm::vec2> uvs;                                             

    std::vector<pnt_index> triangles;                                       // triangles, polygonal faces are triangulated on the fly as triangle fans

    int l = 0;                                                              // line number, for debug messages

    int material_id = -1;                                                   // -1 is the index of the default material
    GLuint last_material_index = 0;

    for (std::string buffer; getline(input_stream, buffer); ++l)
    {        
        if (buffer.empty()) continue;
        std::stringstream line(buffer);
        std::string token;
        line >> token;
                                                                            
        if (token == "v")                                                   // got new vertex position
        {
            glm::vec3 position;
            if (line >> position)
            {
                positions.push_back(position);                              // vec3 read successfully, add the position
                continue;
            }
            else
                exit_msg("Error parsing vertex position at line %d.", l);
        }
        if (token == "vn")                                                  // got new vertex normal
        {
            glm::vec3 normal;
            if (line >> normal)
            {
                normals.push_back(normal);                                  // vec3 read successfully, add the normal
                continue;
            }
            else
                exit_msg("Error parsing normal at line %d.", l);
        }
        if (token == "vt")                                                  // got new texture coordinate
        {
            glm::vec2 uv;
            if (line >> uv)
            {
                uvs.push_back(uv);                                          // vec2 read successfully, add the texture coordinate
                continue;
            }
            else
                exit_msg("Error parsing texture coordinate at line %d.", l);        
        }
        
        if (token == "f")                                                   // new polygonal face begin
        {
            pnt_index pnt_max = pnt_index(positions.size(), normals.size(), uvs.size());
            pnt_index A, B;

            if (!read_pnt_index(line, A, pnt_max))
                exit_msg("Error parsing index triple at line %d : ", l);

            if (!read_pnt_index(line, B, pnt_max))
                exit_msg("Error parsing index triple at line %d.", l);

            do
            {
                pnt_index C;
                if (!read_pnt_index(line, C, pnt_max))
                    exit_msg("Error parsing index triple at line %d.", l);

                triangles.push_back(A);
                triangles.push_back(B);
                triangles.push_back(C);
                B = C;
                line >> std::ws;
            }
            while(!line.eof());        

            continue;
        }

    }
    debug_msg("File %s parsed. #positions : %d. #normals : %d. #uvs : %d. #indices : %d.", filename, (int) positions.size(), (int) normals.size(), (int) uvs.size(), (int) triangles.size());

    if (normals.empty())
    {
        normals.resize(positions.size(), glm::vec3(0.0f));
        for (size_t t = 0; t < triangles.size(); t += 3)
        {
            unsigned int pA = triangles[t + 0].p;
            unsigned int pB = triangles[t + 1].p;
            unsigned int pC = triangles[t + 2].p;
            triangles[t + 0].n = pA;
            triangles[t + 1].n = pB;
            triangles[t + 2].n = pC;

            glm::vec3 vA = positions[pA], vB = positions[pB], vC = positions[pC];
            glm::vec3 normal = glm::normalize(glm::cross(vB - vA, vC - vA));

            normals[pA] += normal;
            normals[pB] += normal;
            normals[pC] += normal;
        }
        for (size_t n = 0; n < normals.size(); ++n)
            normals[n] = glm::normalize(normals[n]);
    }
                                                                            // Note : while it is possible to generate normals from position and index data
                                                                            // there is no any canonical way to generate texture coordinates
    indices_out.resize(triangles.size());                                   // index buffer that will be used by OpenGL

    if (!uvs.empty())                                                       // flatten vertices and indices
    {
        std::map<pnt_index, GLushort> vcache;                               // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pnt2_t> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)                       
        {
            pnt_index& v = triangles[i];

            std::map<pnt_index, GLushort>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n, t) triple already exists in the map
                indices_out[i] = it->second;                                // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if ((v.n == -1) || (v.t == -1))
                    exit_msg("Error :: textured model :: invalid index triple (%d, %d, %d)", v.p, v.n, v.t);

                positions_out.push_back(positions[v.p]);
                normals_out.push_back(normals[v.n]);
                uvs_out.push_back(uvs[v.t]);
                vcache[v] = vindex;
                indices_out[i] = vindex;
                vindex++;
            }
        }
    }
    else
    {
        std::map<pn_index, GLushort> vcache;                                // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pn_t> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)                       
        {
            pn_index v = pn_index(triangles[i].p, triangles[i].n);

            std::map<pn_index, GLushort>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n) triple already exists in the map
                indices_out[i] = it->second;                                // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if (v.n == -1)
                    exit_msg("Error :: non-textured model :: invalid index pair (%d, %d)", v.p, v.n);

                positions_out.push_back(positions[v.p]);
                normals_out.push_back(normals[v.n]);
                vcache[v] = vindex;
                indices_out[i] = vindex;
                vindex++;
            }
        }
    }
    return true;
}