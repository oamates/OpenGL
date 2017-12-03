#ifndef _octree_included_723895613498561847561302856213085612983551293845129387
#define _octree_included_723895613498561847561302856213085612983551293845129387

#include <string>
#include <vector>
#include "primitive.hpp"
#include "scene.hpp"

// Octree structure

struct node_t 
{
    int child[8];                   // children
    float coords[6];                // xmin, ymin, zmin, xmax, ymax, zmax
    std::vector<int> objects_id;    // object ids to display
};

// Octree structure

struct octree_t
{
    octree_t(Scene* scene);
    ~octree_t(); 
    void build(int node_id, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
    bool isTriangleInNode(Triangle triangle, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax);
    std::vector<node_t> m_nodes;
    int m_nb_prim_max; // max primitive number for a node (dynamic value to be sent to the shader)

    node_t* m_root;
    float m_xmin;
    float m_xmax;
    float m_ymin;
    float m_ymax;
    float m_zmin;
    float m_zmax;
    float m_sizeX;
    float m_sizeY;
    float m_sizeZ;
    
    int m_objects_number;
    int m_objects_max;
    int m_level;
    int m_max_level;

    int m_cur_node_id;
    Scene* m_scene;
};

#endif // _octree_included_723895613498561847561302856213085612983551293845129387
