#ifndef _material_included_3146587093257805356845724634260498163745132451346135
#define _material_included_3146587093257805356845724634260498163745132451346135

#include <string> 
#include <glm/glm.hpp>                                                      
#include <glm/gtx/string_cast.hpp>

#include "log.hpp"

//=======================================================================================================================================================================================================================
// surface material structure
//=======================================================================================================================================================================================================================

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

    unsigned int flags;                                                                     // logical OR of texture flags present in the material (for the shader to know which textures are actually bound)

    /* Proper usage of the following five parameters that can be encountered in an mtl-file requires writing a ray-trace engine. */
    /* This will definitely be done at some level of maturity of the ATAS project, but for now let us just ignore them. */

    // int illum;                                                                           // enum indicating illumination model, as WaveFront understands it
    // glm::vec3 Kt;                                                                        // "Kt", transmittance of the material (rgb-spectral factor for light passing through the material)
    // glm::vec3 Ke;                                                                        // "Ke", emissive coeficient, for radio-active surfaces emitting light
    // float Ni;                                                                            // "Ni", optical density of the surface or its index of refraction (determines how light is bent as it passes through the surface)
    // std::string disp;                                                                    // "disp" : heightmap scalar texture used to deform the surface of an object, requires tesselation or complicated geometry shader
};

//=======================================================================================================================================================================================================================
// default material
//=======================================================================================================================================================================================================================

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
    material.flags = 0;
}

static void print_material(const material_t& material)
{
    debug_msg("material.name = %s.", material.name.c_str());
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
    debug_msg("\tmaterial.texture_flags = %x", material.flags);
}

#endif  // _material_included_3146587093257805356845724634260498163745132451346135    