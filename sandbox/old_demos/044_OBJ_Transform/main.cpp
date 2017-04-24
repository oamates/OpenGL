//========================================================================================================================================================================================================================
// DEMO 041: Object loader
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "constants.hpp"
#include "glfw_window.hpp"
#include "log.hpp"
#include "camera3d.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vao.hpp"
#include "vertex_types.hpp"
#include "momenta.hpp"

#include <cstdio>
#include <vector>
#include <map>
#include <istream>
#include <sstream>
#include <fstream>
#include <iomanip>


//========================================================================================================================================================================================================================
// 3d moving camera : standard initial orientation in space
//========================================================================================================================================================================================================================
const double linear_velocity = 0.66f;
const double angular_rate = 0.0001f;
static camera3d camera;

//========================================================================================================================================================================================================================
// keyboard and mouse handlers
//========================================================================================================================================================================================================================

void keyboard_handler(int key, int scancode, int action, int mods)
{
    if      ((key == GLFW_KEY_UP)    || (key == GLFW_KEY_W)) camera.move_forward(linear_velocity);  
    else if ((key == GLFW_KEY_DOWN)  || (key == GLFW_KEY_S)) camera.move_backward(linear_velocity); 
    else if ((key == GLFW_KEY_RIGHT) || (key == GLFW_KEY_D)) camera.straight_right(linear_velocity);
    else if ((key == GLFW_KEY_LEFT)  || (key == GLFW_KEY_A)) camera.straight_left(linear_velocity);
};

void mouse_handler(double dx, double dy, double duration)
{
    duration = glm::max(duration, 0.01);    
    double norm = sqrt(dx * dx + dy * dy);
    if (norm > 0.01f)
    {
        dx /= norm; dy /= norm;
        double angle = angular_rate * sqrt(norm) / (duration + 0.01);
        camera.rotateXY(dx, dy, angle);
    };
};

//========================================================================================================================================================================================================================
// object loader functions
//========================================================================================================================================================================================================================

struct material_t
{
    std::string name;

    glm::vec3 Ka;                                                                           // "Ka", (rgb-spectral) ambient reflectivity factor
    glm::vec3 Kd;                                                                           // "Kd", diffuse reflectivity factor
    glm::vec3 Ks;                                                                           // "Ks", specular reflectivity factor
    float Ns;                                                                               // "Ns", shininess (exponent) in Phong formula for specular component
    float d;                                                                                // "d", dissolve factor : 0 - fully transparent, 1 - fully opaque
    float bm;                                                                               // "-bm" parameter in "bump" or "map_bump" statement

    std::string map_Ka;                                                                     // "map_Ka" : ambient reflectivity texture data file
    std::string map_Kd;                                                                     // "map_Kd" : diffuse reflectivity texture data file
    std::string map_Ks;                                                                     // "map_Ks" : specular reflectivity texture data file
    std::string map_Ns;                                                                     // "map_Ns" : single channel (!) texture file that contains shininess exponent
    std::string map_d;                                                                      // "map_d"  : dissolve/mask/alpha texture
    std::string map_bump;                                                                   // "map_bump" or "bump" : single channel (!) texture that contains heightmap data 

    unsigned int texture_flags;                                                             // logical OR of texture flags present in the material (for the shader to know which textures are actually bound)

    /* Proper usage of the following five parameters that can be encountered in an mtl-file requires writing a ray-trace engine. */
    /* This will definitely be done at some level of maturity of the ATAS project, but for now let us just ignore them. */

    // int illum;                                                                           // enum indicating illumination model, as WaveFront understands it
    // glm::vec3 Kt;                                                                        // "Kt", transmittance of the material (rgb-spectral factor for light passing through the material)
    // glm::vec3 Ke;                                                                        // "Ke", emissive coeficient, for radio-active surfaces emitting light
    // float Ni;                                                                            // "Ni", optical density of the surface or its index of refraction (determines how light is bent as it passes through the surface)
    // std::string disp;                                                                    // "disp" : heightmap scalar texture used to deform the surface of an object, requires tesselation or complicated geometry shader
};

//===================================================================================================================================
// Auxiliary structure, needed at the time of obj loading only
//===================================================================================================================================

struct pnt_index_t
{
    int p, n, t;

    pnt_index_t() {};
    pnt_index_t(unsigned int p, unsigned int n, unsigned int t) : p(p), n(n), t(t) {};

    friend bool operator < (const pnt_index_t& lhs, const pnt_index_t& rhs)                     // for std::map construction to work
    {
        if (lhs.p < rhs.p) return true;
        if (lhs.p > rhs.p) return false;
        if (lhs.n < rhs.n) return true;
        if (lhs.n > rhs.n) return false;
        return lhs.t < rhs.t;
    }

    static bool read(std::stringstream& is, pnt_index_t& index, const pnt_index_t& pnt_max);
};

struct pn_index_t
{
    int p, n;

    pn_index_t(unsigned int p, unsigned int n) : p(p), n(n) {};

    friend bool operator < (const pn_index_t& lhs, const pn_index_t& rhs)                       // for std::map construction to work
    {
        if (lhs.p < rhs.p) return true;
        if (lhs.p > rhs.p) return false;
        return lhs.n < rhs.n;
    }
};


//===================================================================================================================================================================================================================
// The function tries to read index triple from a given input stream, converts relative (= negative) indices to absolute,
// and checks that the triple does not break pnt_max boundary. 
// Note: only position attribute index is absolutely required, texture coordinate and normal index can be omitted. Value -1 indicates this case.
//===================================================================================================================================================================================================================

bool pnt_index_t::read(std::stringstream& is, pnt_index_t& index, const pnt_index_t& pnt_max)                
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



//===================================================================================================================================
// Static methods
//===================================================================================================================================

static void default_material(material_t& material)
{                
    material.name = "";

    material.Ka = glm::vec3(0.17f);
    material.Kd = glm::vec3(0.50f);
    material.Ks = glm::vec3(0.33f);
    material.Ns = 20.0f;
    material.d = 1.0f;
    material.bm = 0.1f;

    material.map_Ka = "";  
    material.map_Kd = "";  
    material.map_Ks = "";  
    material.map_Ns = "";  
    material.map_d = "";   
    material.map_bump = "";

    material.texture_flags = 0;
}

template<typename real_t>
std::istream& operator >> (std::istream& is, glm::tvec3<real_t>& v)
{
    is >> v.x >> v.y >> v.z;
    return is;
};

template<typename real_t>
std::istream& operator >> (std::istream& is, glm::tvec2<real_t>& v)
{
    is >> v.x >> v.y;
    return is;
};

struct model_t
{
    bool textured;
    vao vertex_array;
    GLuint vao_id, vbo_id, ibo_id;

    std::vector<std::pair<int, GLuint>> material_index;                     // vector of <material_id, index at which material with this id is used>
    std::vector<material_t> materials;                                      // array of materials used to render the model
    std::map<std::string, GLuint> textures;    

    model_t() {};
    model_t(const std::string& file_name, const std::string& mtl_base_path);    
    bool load_mtl(std::map<std::string, int>& material_map, const std::string& mtl_file_name);
    void load_textures(const std::string& mtl_base_path);

    template<typename real_t> 
    struct data_t
    {
        std::vector<glm::tvec3<real_t>> positions;
        std::vector<glm::tvec3<real_t>> normals;
        std::vector<glm::tvec2<real_t>> uvs;
        std::vector<pnt_index_t> triangles;
    };

    template<typename real_t> 
    static data_t<real_t> load_geometry(const std::string file_name)
    {
        std::ifstream input_stream(file_name);                                  // ok, let us begin by opening the input file
        if (!input_stream)
            exit_msg("Cannot open object file %s.", file_name.c_str());

        data_t<real_t> data;
    
        int l = 0;                                                              // line number, for debug messages
    
        for (std::string buffer; getline(input_stream, buffer); ++l)
        {        
            if (buffer.empty()) continue;
            std::stringstream line(buffer);
            std::string token;
            line >> token;
                                                                                
            if (token == "v")                                                   // got new vertex position
            {
                glm::tvec3<real_t> position;
                if (line >> position)
                {
                    data.positions.push_back(position);                         // vec3 read successfully, add the position
                    continue;
                }
                else
                    exit_msg("Error parsing vertex position at line %d.", l);
            }
            if (token == "vn")                                                  // got new vertex normal
            {
                glm::tvec3<real_t> normal;
                if (line >> normal)
                {
                    data.normals.push_back(normal);                             // vec3 read successfully, add the normal
                    continue;
                }
                else
                    exit_msg("Error parsing normal at line %d.", l);
            }
            if (token == "vt")                                                  // got new texture coordinate
            {
                glm::tvec2<real_t> uv;
                if (line >> uv)
                {
                    data.uvs.push_back(uv);                                     // vec2 read successfully, add the texture coordinate
                    continue;
                }
                else
                    exit_msg("Error parsing texture coordinate at line %d.", l);        
            }
            
            if (token == "f")                                                   // new polygonal face begin
            {
                pnt_index_t pnt_max = pnt_index(data.positions.size(), data.normals.size(), data.uvs.size());
                pnt_index_t A, B;
    
                if (!pnt_index_t::read(line, A, pnt_max))
                    exit_msg("Error parsing index triple at line %d : ", l);
    
                if (!pnt_index_t::read(line, B, pnt_max))
                    exit_msg("Error parsing index triple at line %d.", l);
    
                do
                {
                    pnt_index_t C;
                    if (!pnt_index_t::read(line, C, pnt_max))
                        exit_msg("Error parsing index triple at line %d.", l);
    
                    data.triangles.push_back(A);
                    data.triangles.push_back(B);
                    data.triangles.push_back(C);
                    B = C;
                    line >> std::ws;
                }
                while(!line.eof());        
    
                continue;
            }
    
        }

        debug_msg("File parsed. #positions : %d. #normals : %d. #uvs : %d. #indices : %d.", 
                    (int) data.positions.size(), (int) data.normals.size(), (int) data.uvs.size(), (int) data.triangles.size());
    
        if (data.normals.empty())
        {
            data.normals.resize(data.positions.size(), glm::vec3(0.0f));
            for (size_t t = 0; t < data.triangles.size(); t += 3)
            {
                unsigned int pA = data.triangles[t + 0].p;
                unsigned int pB = data.triangles[t + 1].p;
                unsigned int pC = data.triangles[t + 2].p;
                data.triangles[t + 0].n = pA;
                data.triangles[t + 1].n = pB;
                data.triangles[t + 2].n = pC;
    
                glm::tvec3<real_t> vA = data.positions[pA], vB = data.positions[pB], vC = data.positions[pC];
                glm::tvec3<real_t> normal = glm::normalize(glm::cross(vB - vA, vC - vA));
    
                data.normals[pA] += normal;
                data.normals[pB] += normal;
                data.normals[pC] += normal;
            }
            for (size_t n = 0; n < data.normals.size(); ++n)
                data.normals[n] = glm::normalize(data.normals[n]);
        }
        return data;
    };

    static const unsigned int AMBIENT_TEXTURE_PRESENT   = 0x01;
    static const unsigned int DIFFUSE_TEXTURE_PRESENT   = 0x02;
    static const unsigned int SPECULAR_TEXTURE_PRESENT  = 0x04;
    static const unsigned int SHININESS_TEXTURE_PRESENT = 0x08;
    static const unsigned int HEIGHTMAP_TEXTURE_PRESENT = 0x10;
    static const unsigned int NORMALMAP_TEXTURE_PRESENT = 0x20;
    static const unsigned int MASK_TEXTURE_PRESENT      = 0x40;
};



model_t::model_t(const std::string& file_name, const std::string& mtl_base_path)
    : textured(false),
      vao_id(0), vbo_id(0), ibo_id(0)
{
    std::ifstream input_stream(file_name);                                  // ok, let us begin by opening the input file
    if (!input_stream)
        exit_msg("Cannot open object file %s.", file_name.c_str());

    std::map<std::string, int> material_map;
    
    std::vector<glm::vec3> positions;                                       
    std::vector<glm::vec3> normals;                                         
    std::vector<glm::vec2> uvs;                                             

    std::vector<pnt_index_t> triangles;                                     // triangles, polygonal faces are triangulated on the fly as triangle fans

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
            pnt_index_t pnt_max = pnt_index_t(positions.size(), normals.size(), uvs.size());
            pnt_index_t A, B;

            if (!pnt_index_t::read(line, A, pnt_max))
                exit_msg("Error parsing index triple at line %d : ", l);

            if (!pnt_index_t::read(line, B, pnt_max))
                exit_msg("Error parsing index triple at line %d.", l);

            do
            {
                pnt_index_t C;
                if (!pnt_index_t::read(line, C, pnt_max))
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
            std::string mtl_file_name = mtl_base_path + mtl_relative_path;
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
        std::map<pnt_index_t, unsigned int> vcache;                         // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pnt2> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)                       
        {
            pnt_index_t& v = triangles[i];

            std::map<pnt_index_t, unsigned int>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n, t) triple already exists in the map
                indices[i] = it->second;                                    // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if ((v.n == -1) || (v.t == -1))
                    exit_msg("Error :: textured model :: invalid index triple (%d, %d, %d)", v.p, v.n, v.t);

                vertices.push_back(vertex_pnt2(positions[v.p], normals[v.n], uvs[v.t]));
                vcache[v] = vindex;
                indices[i] = vindex;
                vindex++;
            }
        }

        vertex_array.init<vertex_pnt2, GLuint>(GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());

        std::string vao_file_name = file_name + ".vao";
        vao::store<vertex_pnt2, GLuint>(vao_file_name.c_str(), GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());
    }
    else
    {
        std::map<pn_index_t, unsigned int> vcache;                          // vertex cache to reuse vertices that use the same attribute index triple
        std::vector<vertex_pn> vertices;

        unsigned int vindex = 0;
        for (size_t i = 0; i < triangles.size(); ++i)                       
        {
            pn_index_t v = pn_index_t(triangles[i].p, triangles[i].n);

            std::map<pn_index_t, unsigned int>::iterator it = vcache.find(v);
            if (it != vcache.end())                                         // if given (p, n) triple already exists in the map
                indices[i] = it->second;                                    // just add the corresponding index
            else                                                            // otherwise, create a new entry in the map and in the vertex buffer
            {
                if (v.n == -1)
                    exit_msg("Error :: non-textured model :: invalid index pair (%d, %d)", v.p, v.n);

                vertices.push_back(vertex_pn(positions[v.p], normals[v.n]));
                vcache[v] = vindex;
                indices[i] = vindex;
                vindex++;
            }
        }
        vertex_array.init<vertex_pn, GLuint>(GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());

        std::string vao_file_name = file_name + ".vao";
        vao::store<vertex_pn, GLuint>(vao_file_name.c_str(), GL_TRIANGLES, vertices.data(), vertices.size(), indices.data(), indices.size());
    }

    load_textures(mtl_base_path);
}; 

bool model_t::load_mtl(std::map<std::string, int>& material_map, const std::string& mtl_file_name)
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

void model_t::load_textures(const std::string& mtl_base_path)
{
    for (size_t i = 0; i < materials.size(); ++i)
    {
        material_t& material = materials[i];
        
        if (!material.map_Ka.empty())                                       // ambient texture load
        {
            if (!textures.count(material.map_Ka))
            {
                std::string path = mtl_base_path + material.map_Ka;
                textures[material.map_Ka] = texture::texture2d_png(path.c_str());
            }
            material.texture_flags |= AMBIENT_TEXTURE_PRESENT;
        }

        if (!material.map_Kd.empty())                                       // diffuse texture load
        {
            if (!textures.count(material.map_Kd))
            {
                std::string path = mtl_base_path + material.map_Kd;
                textures[material.map_Kd] = texture::texture2d_png(path.c_str());
            }
            material.texture_flags |= DIFFUSE_TEXTURE_PRESENT;
        }

        if (!material.map_Ks.empty())                                       // specular texture load
        {
            if (!textures.count(material.map_Ks)) 
            {
                std::string path = mtl_base_path + material.map_Ks;
                textures[material.map_Ks] = texture::texture2d_png(path.c_str());
            }
            material.texture_flags |= SPECULAR_TEXTURE_PRESENT;
        }
        
        if (!material.map_Ns.empty())                                       // shininess texture load
        {
            if (!textures.count(material.map_Ns))
            {
                std::string path = mtl_base_path + material.map_Ns;
                textures[material.map_Ns] = texture::texture2d_png(path.c_str());
            }
            material.texture_flags |= SHININESS_TEXTURE_PRESENT;
        }

        if (!material.map_bump.empty())                                     // bump texture load
        {
            int channels = 1;
            if (!textures.count(material.map_bump))
            {
                std::string path = mtl_base_path + material.map_bump;
                textures[material.map_bump] = texture::texture2d_png(path.c_str(), &channels);
            }
            material.texture_flags |= ((channels == 1) ? HEIGHTMAP_TEXTURE_PRESENT : NORMALMAP_TEXTURE_PRESENT);
        }

        if (!material.map_d.empty())                                        // mask texture load
        {
            if (!textures.count(material.map_d))
            {
                std::string path = mtl_base_path + material.map_d;
                textures[material.map_d] = texture::texture2d_png(path.c_str());
            }
            material.texture_flags |= MASK_TEXTURE_PRESENT;
        }
    }
};

struct lattice_t
{
    GLuint vao_id, vbo_id;
    GLuint V;

    lattice_t(int N)
    {
        V = 6 * (N + N + 1) * (N + N + 1);
        std::unique_ptr<glm::vec3[]> vertices(new glm::vec3[V]);

        int index = 0;
        for (int x = -N; x <= N; ++x)
            for (int y = -N; y <= N; ++y)
            {
                vertices[index++] = glm::vec3( x,  y, -N);
                vertices[index++] = glm::vec3( x,  y,  N);
                vertices[index++] = glm::vec3(-N,  x,  y);
                vertices[index++] = glm::vec3( N,  x,  y);
                vertices[index++] = glm::vec3( y, -N,  x);
                vertices[index++] = glm::vec3( y,  N,  x);
            }        

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, V * sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        debug_msg("V = %d", V);
    }

    ~lattice_t()
    {
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    }
    
    void render()
    {   
        glBindVertexArray(vao_id);
        glDrawArrays(GL_LINES, 0, V);        
    }
};

struct axes_t
{
    GLuint vao_id, vbo_id;

    axes_t(float l)
    {
        vertex_pc vertices[6];

        vertices[0] = vertex_pc(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        vertices[1] = vertex_pc(glm::vec3(   l, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        vertices[2] = vertex_pc(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        vertices[3] = vertex_pc(glm::vec3(0.0f,    l, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        vertices[4] = vertex_pc(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vertices[5] = vertex_pc(glm::vec3(0.0f, 0.0f,    l), glm::vec3(0.0f, 0.0f, 1.0f));

        glGenVertexArrays(1, &vao_id);
        glBindVertexArray(vao_id);
        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(vertex_pc), glm::value_ptr(vertices[0].position), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pc), (const GLvoid *) offsetof(vertex_pc, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_pc), (const GLvoid *) offsetof(vertex_pc, color));
    }

    ~axes_t()
    {
        glDeleteBuffers(1, &vbo_id);
        glDeleteVertexArrays(1, &vao_id);
    }
    
    void render()
    {   
        glBindVertexArray(vao_id);
        glDrawArrays(GL_LINES, 0, 6);        
    }
};

int main()
{
    //===================================================================================================================================================================================================================
    // GLFW window creation + GLEW library initialization
    // 8AA samples, OpenGL 4.3 context, screen resolution : 1920 x 1080
    //===================================================================================================================================================================================================================
    glfw_window window("OBJ Loader", 8, 3, 3, 1920, 1080);
    window.log_info();
    window.mouse_handler = mouse_handler;
    window.keyboard_handler = keyboard_handler;
    camera.infinite_perspective(constants::two_pi / 6.0f, window.aspect_ratio(), 0.1f);

    int LATTICE_SIZE = 5;
    lattice_t coordinate_lattice(LATTICE_SIZE);
    axes_t axes(2.0f * LATTICE_SIZE);


    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program lattice_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/lattice.vs"),
                                glsl_shader(GL_FRAGMENT_SHADER, "glsl/lattice.fs"));
    lattice_shader.enable();
    GLuint uniform_pv_matrix0 = lattice_shader.uniform_id("projection_view_matrix");
    glUniform1f(lattice_shader.uniform_id("scale"), 1.0f);
    glUniform1f(lattice_shader.uniform_id("inv_bound"), 1.0f / LATTICE_SIZE);


    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program axes_shader(glsl_shader(GL_VERTEX_SHADER,   "glsl/axes.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/axes.fs"));
    axes_shader.enable();
    GLuint uniform_pv_matrix1 = axes_shader.uniform_id("projection_view_matrix");
    glUniform1f(axes_shader.uniform_id("scale"), 1.0f);

    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : no texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program blinn_phong_nouv(glsl_shader(GL_VERTEX_SHADER,   "glsl/blinn-phong-nouv.vs"),
                                  glsl_shader(GL_FRAGMENT_SHADER, "glsl/blinn-phong-nouv.fs"));

    blinn_phong_nouv.enable();

    GLint uniform_camera_ws0 = blinn_phong_nouv.uniform_id("camera_ws");
    GLint uniform_light_ws0  = blinn_phong_nouv.uniform_id("light_ws");
    GLint uniform_Ka0        = blinn_phong_nouv.uniform_id("Ka");
    GLint uniform_Kd0        = blinn_phong_nouv.uniform_id("Kd");
    GLint uniform_Ks0        = blinn_phong_nouv.uniform_id("Ks");
    GLint uniform_Ns0        = blinn_phong_nouv.uniform_id("Ns");
    GLint uniform_d0         = blinn_phong_nouv.uniform_id("d");


    //===================================================================================================================================================================================================================
    // Load standard Blinn-Phong shader : model includes texture coordinates
    //===================================================================================================================================================================================================================
    glsl_program blinn_phong(glsl_shader(GL_VERTEX_SHADER,   "glsl/blinn-phong.vs"),
                             glsl_shader(GL_FRAGMENT_SHADER, "glsl/blinn-phong.fs"));

    blinn_phong.enable();
    glUniform1i(blinn_phong.uniform_id("map_Ka"), 0);               // ambient texture                       
    glUniform1i(blinn_phong.uniform_id("map_Kd"), 1);               // diffuse texture                       
    glUniform1i(blinn_phong.uniform_id("map_Ks"), 2);               // specular texture                      
    glUniform1i(blinn_phong.uniform_id("map_Ns"), 3);               // specular shininess texture            
    glUniform1i(blinn_phong.uniform_id("map_bump"), 4);             // bump texture                          
    glUniform1i(blinn_phong.uniform_id("map_d"), 5);                // mask texture                          

    GLint uniform_camera_ws = blinn_phong.uniform_id("camera_ws");
    GLint uniform_light_ws  = blinn_phong.uniform_id("light_ws");
    GLint uniform_Ka        = blinn_phong.uniform_id("Ka");
    GLint uniform_Kd        = blinn_phong.uniform_id("Kd");
    GLint uniform_Ks        = blinn_phong.uniform_id("Ks");
    GLint uniform_Ns        = blinn_phong.uniform_id("Ns");
    GLint uniform_d         = blinn_phong.uniform_id("d");
    GLint uniform_bm        = blinn_phong.uniform_id("bm");

    GLint uniform_texture_flags = blinn_phong.uniform_id("texture_flags");


    //===================================================================================================================================================================================================================
    // Global OpenGL state : since there are no depth writes, depth buffer needs not be cleared
    //===================================================================================================================================================================================================================
    glClearColor(0.015f, 0.005f, 0.045f, 1.0f);
    glEnable(GL_DEPTH_TEST);   
    glEnable(GL_CULL_FACE);

    //===============================================================================================================================
    // create UBO for common uniforms, bind it to UBO target 0 and connect with shader uniform blocks
    //===============================================================================================================================
    struct
    {
        glm::mat4 projection_view_matrix;
        glm::mat4 projection_matrix;
        glm::mat4 view_matrix;
        glm::mat4 camera_matrix;
    } matrices;

    GLuint ubo_id;
    glGenBuffers(1, &ubo_id);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(matrices), 0, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_id);

    blinn_phong_nouv.bind_uniform_buffer("matrices", 0);
    blinn_phong.bind_uniform_buffer("matrices", 0);

    //===============================================================================================================================
    // create VBOs from a set of vectors
    //===============================================================================================================================

    std::string objPath = "models\\demon.obj";
    std::string mtlPath = "models\\";

    model_t model(objPath, mtlPath);

    //===================================================================================================================================================================================================================
    // Log the materials information
    //===================================================================================================================================================================================================================
    for (size_t i = 0; i < model.materials.size(); i++)
    {
        material_t& material = model.materials[i];
        debug_msg("material[%ld].name = %s.", i, material.name.c_str());
        debug_msg("\tmaterial.Ka = %s.", glm::to_string(material.Ka).c_str());
        debug_msg("\tmaterial.Kd = %s.", glm::to_string(material.Kd).c_str());
        debug_msg("\tmaterial.Ks = %s.", glm::to_string(material.Ks).c_str());
        debug_msg("\tmaterial.Ns = %f.", material.Ns);
        debug_msg("\tmaterial.d  = %f.", material.d);
        debug_msg("\tmaterial.bm = %f.", material.bm);

        debug_msg("\tmaterial.map_Ka = %s", material.map_Ka.c_str());
        debug_msg("\tmaterial.map_Kd = %s", material.map_Kd.c_str());
        debug_msg("\tmaterial.map_Ks = %s", material.map_Ks.c_str());
        debug_msg("\tmaterial.map_Ns = %s", material.map_Ns.c_str());
        debug_msg("\tmaterial.map_d = %s", material.map_d.c_str());
        debug_msg("\tmaterial.map_bump = %s", material.map_bump.c_str());

        debug_msg("\tmaterial.texture_flags = %x", material.texture_flags);
    }

    const float light_radius = 7500.0f;


    

    //===================================================================================================================================================================================================================
    // The main loop
    //===================================================================================================================================================================================================================

    while(!window.should_close())
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        matrices.projection_matrix = camera.projection_matrix;
        matrices.view_matrix = camera.view_matrix;
        matrices.projection_view_matrix = camera.projection_matrix * camera.view_matrix;
        matrices.camera_matrix = glm::inverse(camera.view_matrix);

        //===============================================================================================================================================================================================================
        // Render model
        //===============================================================================================================================================================================================================

        if (model.textured)
            blinn_phong.enable();
        else
            blinn_phong_nouv.enable();


        float time = window.time();
        glm::vec3 light_ws = glm::vec3(light_radius * cos(time), 3500.0f * sin(0.577 * time), light_radius * sin(time));
        glm::vec3 camera_ws = camera.position();
        if (model.textured)
        {
            glUniform3fv(uniform_light_ws, 1, glm::value_ptr(light_ws));
            glUniform3fv(uniform_camera_ws, 1, glm::value_ptr(camera_ws));
        }
        else
        {
            glUniform3fv(uniform_light_ws0, 1, glm::value_ptr(light_ws));
            glUniform3fv(uniform_camera_ws0, 1, glm::value_ptr(camera_ws));
        }

        //===============================================================================================================================================================================================================
        // Write common shader data to shared uniform buffer
        //===============================================================================================================================================================================================================
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
        GLvoid* buf_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
        memcpy(buf_ptr, &matrices, sizeof(matrices));
        glUnmapBuffer(GL_UNIFORM_BUFFER);

//        glBindVertexArray(mdl.vao_id);

        unsigned int index = 0;
        for (size_t i = 0; i < model.material_index.size(); ++i)
        {
            int material_id = model.material_index[i].first; 

            if (model.textured)
            {
                if (material_id == -1)
                {
                    glm::vec3 Ka = glm::vec3(0.17f);
                    glm::vec3 Kd = glm::vec3(0.50f);
                    glm::vec3 Ks = glm::vec3(0.33f);
                    float Ns = 20.0f;
                    float d = 1.0f;
                
                    glUniform3fv(uniform_Ka, 1, glm::value_ptr(Ka));
                    glUniform3fv(uniform_Kd, 1, glm::value_ptr(Kd));
                    glUniform3fv(uniform_Ks, 1, glm::value_ptr(Ks));
                    glUniform1f (uniform_Ns, Ns);
                    glUniform1f (uniform_d, d);
                    glUniform1i (uniform_texture_flags, 0);
                }
                else
                {
                    material_t& material = model.materials[material_id];

                    if (!material.map_Ka.empty())
                    {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_Ka]);
                    }
                    
                    if (!material.map_Kd.empty())
                    {
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_Kd]);
                    }
                    
                    if (!material.map_Ks.empty())
                    {
                        glActiveTexture(GL_TEXTURE2);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_Ks]);
                    }
                    
                    if (!material.map_Ns.empty())
                    {
                        glActiveTexture(GL_TEXTURE3);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_Ns]);
                    }
                    
                    if (!material.map_bump.empty())
                    {
                        glActiveTexture(GL_TEXTURE4);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_bump]);
                    }
                    
                    if (!material.map_d.empty())
                    {
                        glActiveTexture(GL_TEXTURE5);
                        glBindTexture(GL_TEXTURE_2D, model.textures[material.map_d]);
                    }
                    
                    glUniform3fv(uniform_Ka, 1, glm::value_ptr(material.Ka));
                    glUniform3fv(uniform_Kd, 1, glm::value_ptr(material.Kd));
                    glUniform3fv(uniform_Ks, 1, glm::value_ptr(material.Ks));
                    glUniform1f (uniform_Ns, material.Ns);
                    glUniform1f (uniform_d,  material.d);
                    glUniform1f (uniform_bm, material.bm);

                    glUniform1i (uniform_texture_flags, material.texture_flags);

                }
            }
            else
            {
                if (material_id == -1)
                {
                    glm::vec3 Ka = glm::vec3(0.17f);
                    glm::vec3 Kd = glm::vec3(0.50f);
                    glm::vec3 Ks = glm::vec3(0.33f);
                    float Ns = 20.0f;
                    float d = 1.0f;
                
                    glUniform3fv(uniform_Ka0, 1, glm::value_ptr(Ka));
                    glUniform3fv(uniform_Kd0, 1, glm::value_ptr(Kd));
                    glUniform3fv(uniform_Ks0, 1, glm::value_ptr(Ks));
                    glUniform1f (uniform_Ns0, Ns);
                    glUniform1f (uniform_d0, d);
                }
                else
                {
                    material_t& material = model.materials[material_id];
                    glUniform3fv(uniform_Ka0, 1, glm::value_ptr(material.Ka));
                    glUniform3fv(uniform_Kd0, 1, glm::value_ptr(material.Kd));
                    glUniform3fv(uniform_Ks0, 1, glm::value_ptr(material.Ks));
                    glUniform1f (uniform_Ns0, material.Ns);
                    glUniform1f (uniform_d0, material.d);
                }
            }

            GLuint last_index = model.material_index[i].second; 
            
            
            model.vertex_array.render(last_index - index, (const GLvoid *) (sizeof(GLuint) * index));

            index = last_index;                                


        }
        
        //===============================================================================================================================================================================================================
        // Render coordinate system
        //===============================================================================================================================================================================================================
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        lattice_shader.enable();
        glUniformMatrix4fv(uniform_pv_matrix0, 1, GL_FALSE, glm::value_ptr(matrices.projection_view_matrix));
        coordinate_lattice.render();

        axes_shader.enable();
        glUniformMatrix4fv(uniform_pv_matrix1, 1, GL_FALSE, glm::value_ptr(matrices.projection_view_matrix));
        axes.render();

        glDisable(GL_BLEND);

        window.swap_buffers();
        window.poll_events();
    };

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================    return 0;
    return 0;
}                               