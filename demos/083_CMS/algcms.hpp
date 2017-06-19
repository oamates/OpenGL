#ifndef _cms_alg_included_78946375463727856102735651025612510235605612852735682
#define _cms_alg_included_78946375463727856102735651025612510235605612852735682

#include <vector>
#include <list>

#include <glm/glm.hpp>

#include "isosurface.hpp"
#include "edge.hpp"
#include "range.hpp"
#include "cell.hpp"
#include "array3d.hpp"
#include "octree.hpp"
#include "tables.hpp"

namespace cms
{

struct point_t
{
    glm::vec3 position;
    float value;

    point_t(const glm::vec3& position, float value) 
        : position(position), value(value)
    {}

    ~point_t() {};

};

struct mesh_t
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;  
    mesh_t() {};
};

struct vertex_t
{
    glm::vec3 position;
    glm::vec3 normal;

    vertex_t() {}

    vertex_t(const glm::vec3& position, const glm::vec3& normal)
        : position(position), normal(normal)
    {}
};

//=======================================================================================================================================================================================================================
// Cubical Marching Squares isosurface extraction algorithm
//=======================================================================================================================================================================================================================

struct AlgCMS : public Isosurface
{
    //===================================================================================================================================================================================================================
    // algorithm data
    //===================================================================================================================================================================================================================
    Array3D<float> m_sampleData;                                                // The sampling 1D array masked in an Array3D wrapper class
    Array3D<edge_block_t> m_edgeData;                                           // The edgeblock 1D array masked in an Array3D wrapper class EdgeBlock has 3 edges
    std::vector<vertex_t> m_vertices;                                           // The vertex array, storing all the vertices in the mesh-to-be
    octree_t* octree;                                                           // the octree of the current function

    mesh_t mesh;                                                                // a Mesh object which will get populated, once the algorithm is done, and store the verts, inds, normals
    Isosurface* m_fn;                                                           // A ptr to the specified Isosurface
    glm::ivec3 m_samples;                                                       // the samples in xyz
    Range m_container[3];                                                       // the bbox of the function

    float m_xMax, m_xMin,
          m_yMax, m_yMin,
          m_zMax, m_zMin;
    
    glm::vec3 m_offsets;                                                        // the dimensions of the bbox as a vec3
    cell_t* octree_root;                                                        // a pointer to the root of the octree

    unsigned int m_octMinLvl;                                                   // The level to which the base grid is to be subdivided (foundation of the octree)
    unsigned int m_octMaxLvl;                                                   // The maximum level (depth) of the octree
    
    float complex_surface_threshold;                                            // The user defined threshold of what should be regarded as a complex surface within a cell
    unsigned int zero_search_iterations;                                        // the linear interpolation quality, e.g. the maximum number of recursions

    //===================================================================================================================================================================================================================
    // constructor and destructor
    //===================================================================================================================================================================================================================
    AlgCMS(Isosurface* i_fn, const Range i_container[], unsigned int min_level, unsigned int max_level);
                        
    ~AlgCMS()                                                                   // destructor, destorying the octree instance
        { if (octree) delete octree; }

    //===================================================================================================================================================================================================================
    // interface Functions
    //===================================================================================================================================================================================================================
    float operator() (float x, float y, float z) const                          // the overloaded function call operator from Isosurface
        { return (*m_fn)(x, y, z); }

    //===================================================================================================================================================================================================================
    // the main function which extracts a surface out of sampling data, saving it into a given mesh object
    //===================================================================================================================================================================================================================
    void extractSurface(mesh_t& mesh)
    {
        octree->buildOctree();                                                  // call the function that would recursively generate the octree
        octree_root = octree->root;                                             // get the octree root cell
        cubicalMarchingSquaresAlg();                                            // traverse the octree and create components from each leaf cell
        tessellationTraversal(octree_root, mesh);                               // traverse the tree again and meshing all components

        for(unsigned i = 0; i < m_vertices.size(); ++i)                         // load the vertices onto the mesh
            mesh.vertices.push_back(m_vertices[i].position);
    }

    glm::vec3 find_zero(unsigned int quality, const point_t& p0, const point_t& p1)
    {
        float alpha = p0.value / (p0.value - p1.value);
        glm::vec3 p = p0.position + alpha * (p1.position - p0.position);        // interpolate

        float value = (*m_fn)(p.x, p.y, p.z);                                   // resample
        point_t pt(p, value);                                                   // save the point

        const float EPSILON = 0.00001f;
        if((glm::abs(value) < EPSILON) || (quality == 0)) return p;             // return if good enough

        if(value < 0.0f)
        {
            p = (p0.value > 0.0f) ? find_zero(quality - 1, pt, p0):
                                    find_zero(quality - 1, pt, p1);
        }
        else if(value > 0.0f)
        {
            p = (p0.value < 0.0f) ? find_zero(quality - 1, p0, pt):
                                    find_zero(quality - 1, p1, pt);
        }

        return p;
    }
  
    void makeFaceSegments(const glm::ivec3 inds[], face_t* i_face);

    void tessellateComponent(mesh_t& mesh, std::vector<unsigned int>& component)
    {
        int numOfInds = component.size();
        assert(numOfInds >= 3);
    
        if(numOfInds == 3)                                                      // three vertices - just output a plain triangle
        {
            for(int i = 0; i < 3; ++i)
                mesh.indices.push_back(component[i]);
            return;
        }                                                          

        vertex_t median;                                                        // more than three vertices - find the median and make a fan
        median.position = glm::vec3(0.0f);

        for(unsigned int i = 0; i < component.size(); ++i)
            median.position += m_vertices[component[i]].position;

        median.position /= float(numOfInds);
        
        float value = (*m_fn)(median.position.x, median.position.y, median.position.z);
        median.normal = glm::normalize(gradient(0.5f * m_offsets, median.position, value));
        median.position -= value * median.normal;

        m_vertices.push_back(median);
        component.push_back(m_vertices.size() - 1);

        for(unsigned int i = 0; i < component.size() - 1; ++i)                  // -2 because median index is at (size-1) and we stich end to begin later
        {
            mesh.indices.push_back(component[component.size() - 1]);
            mesh.indices.push_back(component[i]);
            mesh.indices.push_back(component[i + 1]);
        }
  
        mesh.indices.push_back(component[component.size() - 1]);                // Connecting the last and the first
        mesh.indices.push_back(component[component.size() - 2]);
        mesh.indices.push_back(component[0]);
    }

    //===================================================================================================================================================================================================================
    // gradient of the field function
    //===================================================================================================================================================================================================================
    glm::vec3 gradient(const glm::vec3& dimensions, const glm::vec3& position)
    {
        float value = (*m_fn)(position.x, position.y, position.z);
        float dx = (*m_fn)(position.x + dimensions.x, position.y,                position.z);
        float dy = (*m_fn)(position.x,                position.y + dimensions.y, position.z);
        float dz = (*m_fn)(position.x,                position.y,                position.z + dimensions.z);
        return glm::vec3(dx - value, dy - value, dz - value);
    }

    glm::vec3 gradient(const glm::vec3& dimensions, const glm::vec3& position, const float& value)
    {
        float dx = (*m_fn)(position.x + dimensions.x, position.y,                position.z);
        float dy = (*m_fn)(position.x,                position.y + dimensions.y, position.z);
        float dz = (*m_fn)(position.x,                position.y,                position.z + dimensions.z);
        return glm::vec3(dx - value, dy - value, dz - value);
    }
    
    //===================================================================================================================================================================================================================
    // traverses the octree and tessellates all the components on each LEAF cell
    //===================================================================================================================================================================================================================
    void tessellationTraversal(cell_t* cell, mesh_t& mesh)
    {
        if(!cell) return;                                                           // skip empty nodes
  
        if(cell->state == BRANCH)                                                   // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                tessellationTraversal(cell->children[i], mesh);
            return;
        }
        
        for(unsigned int i = 0; i < cell->components.size(); ++i)                   // the node is a leaf, tessellate segment
            tessellateComponent(mesh, cell->components[i]);
    }

    // Finds all the edges that are located inbetween the two given points.
    // it stores the array3d indices of those edges in the provided vector
    // and returns a int value of the direction in which they advance 0-right(x), 1-up(y), 2-front(z)
    int getEdgesBetwixt(Range& o_range, const glm::ivec3& pt0, const glm::ivec3& pt1) const;

    // Finds the exact place of the isovalue crossing point by tracking
    // down the sign change, returning the index of the first point (smaller)
    int exactSignChangeIndex(const Range& range, int& dir, glm::ivec3& ind0, glm::ivec3& ind1) const;

    //===================================================================================================================================================================================================================
    // creates a new vertex on the edge of the exact crossing point it calls the findCrossingPoint
    //===================================================================================================================================================================================================================
    void makeVertex(strip_t& strip, const int& dir, const glm::ivec3& crossingIndex0, const glm::ivec3& crossingIndex1, int index)
    {
        glm::vec3 pos0 = m_sampleData.getPositionAt(crossingIndex0);                // recover two points and find the surface zero between them
        float val0 = m_sampleData.getValueAt(crossingIndex0);
        point_t pt0(pos0, val0);
    
        glm::vec3 pos1 = m_sampleData.getPositionAt(crossingIndex1);
        float val1 = m_sampleData.getValueAt(crossingIndex1);
        point_t pt1(pos1, val1);
    
        glm::vec3 zero_point = find_zero(zero_search_iterations, pt0, pt1);         // find the exact position and normal at the crossing point
        glm::vec3 normal = glm::normalize(gradient(m_offsets, zero_point));
    
        vertex_t vert(zero_point, normal);                                          // create a vertex
        m_vertices.push_back(vert);
    
        strip.data[index] = m_vertices.size() - 1;                                  // place the data onto the currect strip
        strip.block[index] = crossingIndex0;
        strip.dir[index] = dir;
    
        edge_block_t edge_block = m_edgeData.getValueAt(crossingIndex0);            // put the data onto the global 3D array of edges
        if(edge_block.empty)
            edge_block.empty = false;
        assert(edge_block.edge_indices[dir] == -1);
        edge_block.edge_indices[dir] = m_vertices.size() - 1;
        m_edgeData.setValueAt(crossingIndex0, edge_block);
    }

    //===================================================================================================================================================================================================================
    // creates a strip and populates both of it's sides by calling populate strip
    //===================================================================================================================================================================================================================
    void makeStrip(int edge0, int edge1, const glm::ivec3 inds[], face_t* face, int strip_index)
    {
        assert((edge0 != -1) && (edge1 != -1));
        strip_t strip(false, edge0, edge1);
        populate_strip(strip, inds, 0);                                              // 1st edge of First Strip - e0a
        populate_strip(strip, inds, 1);                                              // 2nd edge of First Strip - e0b

        // todo :: Check for face sharp features here

        face->strips[strip_index] = strip;                                          // populate current face with the created strip
        face->skip = false;
    }

    //===================================================================================================================================================================================================================
    // populates a given strip by adding a new vertex on the exact isosurface crossing point or copying an existing one
    //===================================================================================================================================================================================================================
    void populate_strip(strip_t& strip, const glm::ivec3 inds[], int index)
    {
        const int8_t faceEdge = strip.edge[index];                                  // get the edge on the currently examined face
        glm::ivec3 ind_0 = inds[VERTEX_MAP[faceEdge][0]];
        glm::ivec3 ind_1 = inds[VERTEX_MAP[faceEdge][1]];
    
        Range range;                                                                // get the range and the direction (of an edge block) which the edge represents
        int dir = getEdgesBetwixt(range, ind_0, ind_1);
        assert(abs(ind_0[dir] - ind_1[dir]) > 0);
        assert((ind_0[dir] == range.m_lower) || (ind_0[dir] == range.m_upper));
        assert((ind_1[dir] == range.m_lower) || (ind_1[dir] == range.m_upper));
    
        int signChange = exactSignChangeIndex(range, dir, ind_0, ind_1);            // find the exact sign change on that bigger edge range, getting the index of the sample, just before the sign change = edge of change
        assert((signChange >= range.m_lower) && (signChange < range.m_upper));
    
        glm::ivec3 crossingIndex_0 = ind_0;                                         // set the exact two point indices between the zero crossing
        glm::ivec3 crossingIndex_1 = ind_0;
        crossingIndex_0[dir] = signChange;
        crossingIndex_1[dir] = signChange + 1;
        assert(m_sampleData.getValueAt(crossingIndex_0) * m_sampleData.getValueAt(crossingIndex_1) <= 0.0f);
    
        bool duplicate = false;                                                     // check for duplicate vertices on the same edge
        if(m_edgeData.getValueAt(crossingIndex_0).empty == false)                   // check global datastructor edgeblock
        {
            if(m_edgeData.getValueAt(crossingIndex_0).edge_indices[dir] != -1)      // check exact global edge
            {
                strip.data[index] = m_edgeData.getValueAt(crossingIndex_0).edge_indices[dir];
                strip.block[index] = crossingIndex_0;
                strip.dir[index] = dir;
                duplicate = true;
            }
        }
    
        if(!duplicate)                                                              // if there is no previous vertex registered to that edge, proceed to find it.
            makeVertex(strip, dir, crossingIndex_0, crossingIndex_1, index);
    }

    //===================================================================================================================================================================================================================
    // performs the main stages of the algorithm
    //===================================================================================================================================================================================================================
    void cubicalMarchingSquaresAlg()
    {
        generateSegments(octree->root);                                             // traverse through the octree and generate segments for all LEAF cells
        editTransitionalFaces();                                                    // resolve transitional faces
        traceComponent();                                                           // trace the strips into segments and components
    }

    //===================================================================================================================================================================================================================
    // starts from a given node, traverses through the octree and generates segments for all LEAF cells
    //===================================================================================================================================================================================================================
    void generateSegments(cell_t* cell)
    {
        if(!cell) return;                                                           // skip empty nodes
  
        if(cell->state == BRANCH)                                                   // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                generateSegments(cell->children[i]);
            return;
        }

        glm::ivec3 indices[4];                                                      // the node is a leaf, generate segment   
        for(int f = 0; f < 6; ++f)                                                  // for all the faces in this LEAF cell
        {
            for(int v = 0; v < 4; ++v)                                              // convert face vertex index to cell vertex index
            {
                const uint8_t vindex = FACE_VERTEX[f][v];
                indices[v] = cell->point_indices[vindex];
            }
            cell->faces[f]->strips.resize(2);
            makeFaceSegments(indices, cell->faces[f]);
        }
    }

    //===================================================================================================================================================================================================================
    // loops through all transitional faces and calls the resolve function on them
    //===================================================================================================================================================================================================================
    void editTransitionalFaces()
    {
        std::vector<cell_t*> cells = octree->cells;
    
        for(unsigned int i = 0; i < cells.size(); ++i)                              // loop through all cells and all faces and find every transitional face
            for(int j = 0; j < 6; ++j)
                if(cells[i]->faces[j]->state == TRANSIT_FACE)
                    resolveTransitionalFace(cells[i]->faces[j]);                    // pass it for getting the data from it's twin
    }

    // For a given transitional face, collect all
    void segmentFromTwin(face_t* face, std::vector<unsigned int>& o_comp, int lastData, int& currentEdge);

    // Loading all the vertices onto the mesh
    // params :: A regference to the mesh that has to be populated with verts
    void createMesh(mesh_t& o_mesh);

    // collect all the strips of a cell into a single array
    // also populate another array with transitional segments in the case
    // that the cell had transitional faces
    void collectStrips(cell_t* c, std::vector<strip_t>& o_cellStrips, std::vector<std::vector<unsigned int>>& o_transitSegs);

    // Taking all the strips of a given cell and linking them together to form components
    void linkStrips(std::vector<unsigned int>& o_comp, std::vector<strip_t>& strips, std::vector<std::vector<unsigned int>>& transitSegs);

    //===================================================================================================================================================================================================================
    // tests whether a given strip and an existing segment match by checking the first and last value of the segment against the strip data
    //===================================================================================================================================================================================================================
    bool compareStripToSeg(strip_t& strip, std::vector<unsigned int>& segment)
    {
        unsigned int s0 = strip.data[0];
        unsigned int s1 = strip.data[1];
        return (((segment.front() == s0) && (segment.back() == s1)) || ((segment.front() == s1) && (segment.back() == s0)));
    }

    //===================================================================================================================================================================================================================
    // inserts data from twin of a transitional face using provided segments
    //===================================================================================================================================================================================================================
    void insertDataFromTwin(std::vector<unsigned int>& component, std::vector<std::vector<unsigned int>>& segments, strip_t& strip, bool& transit, int& addedInIter, const bool& backwards)
    {
        for(unsigned int i = 0; i < segments.size(); ++i)                               // Loop through all the transitional strips and find the one corresponding to this strip
        {
            if(compareStripToSeg(strip, segments[i]))                                   // check if the strip's data matches the segment
            {
                if(backwards)
                {
                    for(int j = segments[i].size() - 1; j > 0; --j)
                        component.push_back(segments[i][j - 1]);
                }
                else
                {
                    for(unsigned int j = 1; j < segments[i].size(); ++j)
                        component.push_back(segments[i][j]);
                }

                segments.erase(segments.begin() + i);
                ++addedInIter;
                transit = true;
                break;
            }
        }
    }

    // Takes a transitional face and collects all the strrips from it's twin
    // linking them together, those that can get linked.
    // param :: a transitional face
    void resolveTransitionalFace(face_t* face);

    //===================================================================================================================================================================================================================
    // loops through all levels of the octree from the leafs up to the root and looping through all cells linking their strips into components
    //===================================================================================================================================================================================================================
    void traceComponent()
    {
        std::vector<cell_t*> cells = octree->cells;
    
        for(unsigned int i = 0; i < m_octMaxLvl; ++i)                               // trace the strips into segments and components
            for(unsigned int j = 0; j < cells.size(); ++j)                          // loop through all cells and link components for all LEAF cells
                if(cells[j]->level == m_octMaxLvl - i)
                {
                    if(cells[j]->state == LEAF)                                     // trace the segments to form components
                    {
                        std::vector<strip_t> cellStrips;
                        std::vector<std::vector<unsigned int>> transitSegs;
                        std::vector<unsigned int> component;
                    
                        collectStrips(cells[j], cellStrips, transitSegs);           // collect all the strips from that cell
                    
                        while(cellStrips.size() > 0)                                // link the strips into components
                        {
                            linkStrips(component, cellStrips, transitSegs);
                            cells[j]->components.push_back(component);
                            component.clear();
                        }
                    }
                }
    }

    void traverseFace(face_t* face, std::vector<strip_t>& transitStrips)
    {
        if(!face) return;
        assert(face->state != TRANSIT_FACE);
    
        if(face->state == BRANCH_FACE)                                              // if it is a BRANCH face, traverse through all it's children
        {
            for(int i= 0; i < 4; ++i)
                traverseFace(face->children[i], transitStrips);
            return;
        }

        for(unsigned int i = 0; i < face->strips.size(); ++i)                       // it is a LEAF face, check all its strips and collect all valid
        {
            if(face->strips[i].skip == false)
                transitStrips.push_back(face->strips[i]);
            else
                assert(face->strips[i].data[0] == -1);
        }
    }

};

} // namespace cms

#endif // _cms_alg_included_78946375463727856102735651025612510235605612852735682
