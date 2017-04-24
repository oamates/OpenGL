#ifndef _obj_included_034614309675063451670816708134605741360856431875637418564
#define _obj_included_034614309675063451670816708134605741360856431875637418564

#include <vector>
#include <map>
#include "vao.hpp"
#include "material.hpp"

struct model
{
    bool textured;
    vao_t vertex_array;
    GLuint vao_id, vbo_id, ibo_id;

    std::vector<std::pair<int, GLuint>> material_index;                     // vector of <material_id, index at which material with this id is used>
    std::vector<material_t> materials;                                      // array of materials used to render the model
    std::map<std::string, GLuint> textures;    

    model(const std::string& filename, const std::string& dir);    
    bool load_mtl(std::map<std::string, int>& material_map, const std::string& mtl_file_name);
    void load_textures(const std::string& mtl_base_path);

    static const unsigned int AMBIENT_TEXTURE_FLAG   = 0x01;
    static const unsigned int DIFFUSE_TEXTURE_FLAG   = 0x02;
    static const unsigned int SPECULAR_TEXTURE_FLAG  = 0x04;
    static const unsigned int SHININESS_TEXTURE_FLAG = 0x08;
    static const unsigned int HEIGHTMAP_TEXTURE_FLAG = 0x10;
    static const unsigned int NORMALMAP_TEXTURE_FLAG = 0x20;
    static const unsigned int MASK_TEXTURE_FLAG      = 0x40;
};

#endif  // _obj_included_034614309675063451670816708134605741360856431875637418564