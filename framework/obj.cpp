#include <cstdio>
#include <vector>
#include <map>
#include <istream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "obj.hpp"
#include "log.hpp"
#include "vertex.hpp"
#include "image.hpp"

//===================================================================================================================================================================================================================
// Auxiliary structure, needed at the time of obj loading only
//===================================================================================================================================================================================================================

struct pnt_index
{
    int p, n, t;

    pnt_index() {};
    pnt_index(unsigned int p, unsigned int n, unsigned int t) : p(p), n(n), t(t) {};

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
    int p, n;

    pn_index(unsigned int p, unsigned int n) : p(p), n(n) {};

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


model::model(const std::string& filename, const std::string& dir) : textured(false)
{
    std::ifstream input_stream(filename);                                  // ok, let us begin by opening the input file
    if (!input_stream)
        exit_msg("Cannot open object file %s.", filename.c_str());

    std::map<std::string, int> material_map;

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

        if (token == "mtllib")                                              // directive to load material file received
        {
            std::string mtl_relative_path;
            line >> mtl_relative_path;
            std::string mtl_file_name = dir + mtl_relative_path;
            if (!load_mtl(material_map, mtl_file_name))
                exit_msg("Error loading material file %s.", mtl_file_name.c_str());
            continue;
        }

        if (token == "usemtl")
        {
            std::string material_name;
            line >> material_name;
            std::map<std::string, int>::iterator it = material_map.find(material_name);
            if (it == material_map.end())
                exit_msg("Error : Referenced material (%s) not found.", material_name.c_str());

            if (triangles.size() > last_material_index)
            {
                last_material_index = triangles.size();
                material_index.push_back(std::pair<int, GLuint>(material_id, last_material_index));
            }
            material_id = it->second;
        }
    }

    if (triangles.size() > last_material_index)
    {
        last_material_index = triangles.size();
        material_index.push_back(std::pair<int, GLuint>(material_id, last_material_index));
    }

    debug_msg("File parsed. #positions : %d. #normals : %d. #uvs : %d. #indices : %d.", (int) positions.size(), (int) normals.size(), (int) uvs.size(), (int) triangles.size());
                                                                            // calculate normals if they are missing


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

    std::vector<GLuint> indices(triangles.size());                          // index buffer that will be used by OpenGL


    if (!uvs.empty())                                                       // flatten vertices and indices
    {
        textured = true;
        std::map<pnt_index, unsigned int> vcache;                           // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pnt2_t> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)
        {
            pnt_index& v = triangles[i];

            std::map<pnt_index, unsigned int>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n, t) triple already exists in the map
                indices[i] = it->second;                                    // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if ((v.n == -1) || (v.t == -1))
                    exit_msg("Error :: textured model :: invalid index triple (%d, %d, %d)", v.p, v.n, v.t);

                vertices.push_back(vertex_pnt2_t(positions[v.p], normals[v.n], uvs[v.t]));
                vcache[v] = vindex;
                indices[i] = vindex;
                vindex++;
            }
        }

        vertex_array.init<vertex_pnt2_t, GLuint>(GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());

        std::string vao_filename = filename + ".vao";
        debug_msg("Saving vao of PNT2 type :: %s", vao_filename.c_str());
        vao_t::store<vertex_pnt2_t, GLuint>(vao_filename.c_str(), GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());
    }
    else
    {
        std::map<pn_index, unsigned int> vcache;                            // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pn_t> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)
        {
            pn_index v = pn_index(triangles[i].p, triangles[i].n);

            std::map<pn_index, unsigned int>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n) triple already exists in the map
                indices[i] = it->second;                                    // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if (v.n == -1)
                    exit_msg("Error :: non-textured model :: invalid index pair (%d, %d)", v.p, v.n);

                vertices.push_back(vertex_pn_t(positions[v.p], normals[v.n]));
                vcache[v] = vindex;
                indices[i] = vindex;
                vindex++;
            }
        }
        vertex_array.init<vertex_pn_t, GLuint>(GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());

        std::string vao_filename = filename + ".vao";
        debug_msg("Saving vao of PN type :: %s", vao_filename.c_str());
        vao_t::store<vertex_pn_t, GLuint>(vao_filename.c_str(), GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());
    }

    load_textures(dir);
};

bool model::load_mtl(std::map<std::string, int>& material_map, const std::string& mtl_file_name)
{
    std::ifstream input_stream(mtl_file_name);
    if (!input_stream)
        exit_msg("Error : referenced material file %s not found.", mtl_file_name.c_str());

    material_t material;
    default_material(material);

    int l = 0;

    for (std::string buffer; getline(input_stream, buffer); ++l)
    {
        if (buffer.empty()) continue;
        std::stringstream line(buffer);
        std::string token;
        line >> token;

        if (token.empty()) continue;
        if (token == "newmtl")
        {
            if (!material.name.empty())                                     // discard nameless materials
            {
                material_map.insert(std::pair<std::string, int>(material.name, static_cast<int>(materials.size())));
                materials.push_back(material);
            }
            default_material(material);                                     // initial temporary material
            line >> material.name;
            continue;
        }

      #define CHECK_TOKEN(name, dest) if (token == name) { if (line >> dest) continue; else debug_msg("Error parsing material file at line %d. token = %s", l, token.c_str()); }

        CHECK_TOKEN("Ka", material.Ka);
        CHECK_TOKEN("Kd", material.Kd);
        CHECK_TOKEN("Ks", material.Ks);
        CHECK_TOKEN("Ns", material.Ns);
        CHECK_TOKEN("d",  material.d);

        CHECK_TOKEN("map_Ka", material.map_Ka);
        CHECK_TOKEN("map_Kd", material.map_Kd);
        CHECK_TOKEN("map_Ks", material.map_Ks);
        CHECK_TOKEN("map_Ns", material.map_Ns);
        CHECK_TOKEN("map_d",  material.map_d);

      #undef CHECK_TOKEN

        if ((token == "map_bump") || (token == "bump"))
        {
            if (line >> material.map_bump)
            {
                line >> token;
                if (token == "-bm")
                {
                    if (line >> material.bm)
                        continue;
                    else
                        debug_msg("Error parsing material file at line %d.", l);
                }
                continue;
            }
            else
                debug_msg("Error parsing material file at line %d.", l);
        }
    }

    material_map.insert(std::pair<std::string, int>(material.name, static_cast<int>(materials.size())));
    materials.push_back(material);                                          // flush last material
    return true;
}

void model::load_textures(const std::string& dir)
{
    for (size_t i = 0; i < materials.size(); ++i)
    {
        material_t& material = materials[i];

        if (!material.map_Ka.empty())                                       // ambient texture load
        {
            if (!textures.count(material.map_Ka))
            {
                std::string path = dir + material.map_Ka;
                textures[material.map_Ka] = image::png::texture2d(path.c_str());
            }
            material.flags |= AMBIENT_TEXTURE_FLAG;
        }

        if (!material.map_Kd.empty())                                       // diffuse texture load
        {
            if (!textures.count(material.map_Kd))
            {
                std::string path = dir + material.map_Kd;
                textures[material.map_Kd] = image::png::texture2d(path.c_str());
            }
            material.flags |= DIFFUSE_TEXTURE_FLAG;
        }

        if (!material.map_Ks.empty())                                       // specular texture load
        {
            if (!textures.count(material.map_Ks))
            {
                std::string path = dir + material.map_Ks;
                textures[material.map_Ks] = image::png::texture2d(path.c_str());
            }
            material.flags |= SPECULAR_TEXTURE_FLAG;
        }

        if (!material.map_Ns.empty())                                       // shininess texture load
        {
            if (!textures.count(material.map_Ns))
            {
                std::string path = dir + material.map_Ns;
                textures[material.map_Ns] = image::png::texture2d(path.c_str());
            }
            material.flags |= SHININESS_TEXTURE_FLAG;
        }

        if (!material.map_bump.empty())                                     // bump texture load
        {
            int channels = 1;
            if (!textures.count(material.map_bump))
            {
                std::string path = dir + material.map_bump;
                textures[material.map_bump] = image::png::texture2d(path.c_str(), &channels);
            }
            material.flags |= ((channels == 1) ? HEIGHTMAP_TEXTURE_FLAG : NORMALMAP_TEXTURE_FLAG);
        }

        if (!material.map_d.empty())                                        // mask texture load
        {
            if (!textures.count(material.map_d))
            {
                std::string path = dir + material.map_d;
                textures[material.map_d] = image::png::texture2d(path.c_str());
            }
            material.flags |= MASK_TEXTURE_FLAG;
        }
    }
};
