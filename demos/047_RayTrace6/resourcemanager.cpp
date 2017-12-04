// Application resources manager implementation
#include <fstream>
#include <sstream>
 
#include "log.hpp"
#include "resourcemanager.hpp"
#include "defines.hpp"
#include "helper.hpp"
#include "renderer.hpp"

ResourceManager::ResourceManager()
    : FTexIndex(0)
{ }

ResourceManager::~ResourceManager()
{
    foreach(tex, FTextures)
    {
        free(tex->second->content);
        delete tex->second;
    }
}

std::string concatenate(const std::string& parBase, int parIndex)
{
    std::string result = parBase;
    result+="[";
    result+=convertToString(parIndex);
    result+="]";
    return result;
}

// Loads texture from BMP file
const Texture* ResourceManager::LoadTexture(const std::string& parFileName)
{
    debug_color_msg(DEBUG_GREEN_COLOR, "Loading texture: %s", parFileName.c_str());
    FILE* f = fopen(parFileName.c_str(), "rb");

    if(!f)
    {
        debug_color_msg(DEBUG_RED_COLOR, "Error opening file: %s", parFileName.c_str());    
        return 0;
    }

    Texture* newTex = new Texture();
    debug_color_msg(DEBUG_GREEN_COLOR, "File %s successfully opened.", parFileName.c_str()); 

    unsigned char info[54];                         // 54 byte BMP header
    fread(info, sizeof(unsigned char), 54, f); 
    newTex->w = *(int*) &info[18];                  // Image dimensions
    newTex->l = *(int*) &info[22];
    
    
    int size = 3 * newTex->w  * newTex->l;          // Allocate space for rgb image 3 * dimX * dimY bytes
    unsigned char* data = new unsigned char[size];
    fread(data, sizeof(unsigned char), size, f);
    fclose(f);

    for(int i = 0; i < size; i += 3)                // BGR <--> RGB
    {
        unsigned char tmp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = tmp;
    }
    
    newTex->content = data;                         // Copie dans la structure textures

    glGenTextures(1, &newTex->id);
    glBindTexture(GL_TEXTURE_2D, newTex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTex->w, newTex->l, 0, GL_RGB, GL_UNSIGNED_BYTE, newTex->content);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    FTexIndex++;
    return newTex;
}

// Chargement d'un fichier .obj avec 3 textures associées, on ignore le fichier .mtl
// Inspiré de http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Load_OBJ
ObjFile* ResourceManager::LoadModel(const std::string& parFileName, const std::string& parAlbTexFileName, const std::string& parRugTexFileName, const std::string& parSpecTexFileName, const std::string& parNormalTexFileName )
{
    std::vector<glm::dvec3> vertices;           // Liste des points    
    std::vector<glm::dvec3> normales;           // Liste des normales par triangle
    // Gestion des triangles
    std::vector<indexList> indexes;
    std::vector<indexList> uvList;
    
    // Liste des coordonnées de mappage
    std::vector<vec2> mapping;
    
    // Onverture du fichier
    std::fstream in;
    in.open(parFileName.c_str(), std::fstream::in);
    if (!in) 
    { 
        debug_color_msg(DEBUG_RED_COLOR, "Cannot find model obj: %s", parFileName.c_str()); 
        return 0;
    }
    debug_color_msg(DEBUG_ORANGE_COLOR, "Parsing model obj: %s ...", parFileName.c_str()); 

    ObjFile* newModel = new ObjFile();
    std::string line;
    while (getline(in, line)) 
    {
        if (line.substr(0, 2) == "v ") 
        {
            std::stringstream s(line.substr(2));
            glm::dvec3 v; 
            s >> v.x; 
            s >> v.y; 
            s >> v.z; 
            vertices.push_back(v);
        }
        else if (line.substr(0, 2) == "f ") 
        {
            std::stringstream s(line.substr(2));
            std::string a,b,c;
            std::vector<std::string> lineSplitted = split(line,' ');
            a = lineSplitted[1];
            b = lineSplitted[2];
            c = lineSplitted[3];
            std::vector<std::string> index1 = split(a, '/');
            std::vector<std::string> index2 = split(b, '/');
            std::vector<std::string> index3 = split(c, '/');
            indexList triangleIndex;
            triangleIndex.p0 = convertToInt(index1[0]);
            triangleIndex.p1 = convertToInt(index2[0]);
            triangleIndex.p2 = convertToInt(index3[0]);
            triangleIndex.p0--;
            triangleIndex.p1--;
            triangleIndex.p2--;
            indexes.push_back(triangleIndex);
            indexList uvindex;
            uvindex.p0 = convertToInt(index1[1]);
            uvindex.p1 = convertToInt(index2[1]);
            uvindex.p2 = convertToInt(index3[1]);
            uvindex.p0--;
            uvindex.p1--;
            uvindex.p2--;
            uvList.push_back(uvindex);
        }
        else if(line[0] == 'v' && line[1] == 't') 
        {   
            std::istringstream s(line.substr(2));
            float u,v;
            s >> u;
            s >> v;
            vec2 map;
            map.u = u;
            map.v = v;
            mapping.push_back(map);
        }
        else if(line[0] == 'v' && line[1] == 'n') 
        { 
            std::istringstream s(line.substr(2));
            glm::dvec3 normal;
            s >> normal.x;
            s >> normal.y;
            s >> normal.z;
            normales.push_back(normal);
        }
    }           
    
    for (int i = 0; i < indexes.size(); i+=1) 
    {
        Triangle newTriangle;
        newTriangle.p0 = vertices[indexes[i].p0];
        newTriangle.p1 = vertices[indexes[i].p1];
        newTriangle.p2 = vertices[indexes[i].p2];
        newTriangle.uv0 = mapping[uvList[i].p0];
        newTriangle.uv1 = mapping[uvList[i].p1];
        newTriangle.uv2 = mapping[uvList[i].p2];

        newTriangle.normale = glm::cross(newTriangle.p1 - newTriangle.p0, newTriangle.p2 - newTriangle.p0);
        newTriangle.normale = glm::normalize(newTriangle.normale);
        newModel->listTriangle.push_back(newTriangle);
    }

    // Creation du fichier de texture en dur pour l'obj
    foreach(triangle,newModel->listTriangle)
    {
        newModel->material.color = glm::dvec4(0.2, 0.3, 0.8, 1.0);
        newModel->material.indiceRefraction = 1.44;
        newModel->material.texAlbedo = 0;
        newModel->material.texRough = 1;
        newModel->material.texSpec = 2;
    }

    // On charge les textures
    newModel->albTex = LoadTexture(parAlbTexFileName);
    newModel->rugTex = LoadTexture(parRugTexFileName);
    newModel->specTex = LoadTexture(parSpecTexFileName);
    newModel->normalTex = LoadTexture(parNormalTexFileName);
    
    return newModel;
}
