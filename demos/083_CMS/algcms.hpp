#ifndef _cms_alg_included_78946375463727856102735651025612510235605612852735682
#define _cms_alg_included_78946375463727856102735651025612510235605612852735682

#include <vector>

#include <glm/glm.hpp>

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

struct mesh_t
{
    std::vector<vertex_t> vertices;
    std::vector<unsigned int> indices;  
    mesh_t() {};
};

//=======================================================================================================================================================================================================================
// Cubical Marching Squares isosurface extraction algorithm
//=======================================================================================================================================================================================================================

template<typename scalar_field_t> struct AlgCMS
{
    //===================================================================================================================================================================================================================
    // algorithm data
    //===================================================================================================================================================================================================================
    scalar_field_t scalar_field;
    Array3D<float> m_sampleData;                                                // The sampling 1D array masked in an Array3D wrapper class
    Array3D<edge_block_t> m_edgeData;                                           // The edgeblock 1D array masked in an Array3D wrapper class EdgeBlock has 3 edges
    std::vector<vertex_t> vertices;                                             // The vertex array, storing all the vertices in the mesh
    octree_t<scalar_field_t>* octree;                                           // the octree of the current function

    glm::ivec3 dimensions;                                                       // the samples in xyz

    float delta;                                                                // the dimensions of the bbox as a vec3

    cell_t* octree_root;                                                        // a pointer to the root of the octree

    unsigned int min_level, max_level;                                          // base grid subdivision level and maximum octree depth/level
    
    float complex_surface_threshold;                                            // The user defined threshold of what should be regarded as a complex surface within a cell
    unsigned int zero_search_iterations;                                        // the linear interpolation quality, e.g. the maximum number of recursions

    //===================================================================================================================================================================================================================
    // constructor
    //===================================================================================================================================================================================================================
    AlgCMS(unsigned int min_level, unsigned int max_level)
        : min_level(min_level), max_level(max_level)
    {
        int p2 = 1 << max_level;
        dimensions = glm::ivec3(p2 + 1);    
        zero_search_iterations = 5;
        complex_surface_threshold = 0.6f;
        delta = 2.0f / p2;
        
        m_sampleData.resize(dimensions);                                         // Resizing the samplingData array and proceeding with the sampling
        m_edgeData.resize(dimensions);

        octree = new octree_t<scalar_field_t>(dimensions, m_sampleData, min_level, max_level, delta, complex_surface_threshold);
    
        glm::ivec3 index;
        glm::vec3 position;

        position.x = -1.0f;
        for(index.x = 0; index.x <= p2; ++index.x)
        {
            position.y = -1.0f;
            for(index.y = 0; index.y <= p2; ++index.y)
            {
                position.z = -1.0f;
                for(index.z = 0; index.z <= p2; ++index.z)
                {
                    m_sampleData[index] = scalar_field(position);
                    position.z += delta;
                }
                position.y += delta;
            }
            position.x += delta;
        }
    }    
                        
    //===================================================================================================================================================================================================================
    // destructor, destroys the octree instance
    //===================================================================================================================================================================================================================
    ~AlgCMS()
        { if (octree) delete octree; }

    //===================================================================================================================================================================================================================
    // the main function which extracts a surface out of sampling data, saving it into a given mesh object
    //===================================================================================================================================================================================================================
    void extractSurface(mesh_t& mesh)
    {
        octree->buildOctree();                                                  // call the function that would recursively generate the octree
        octree_root = octree->root;                                             // get the octree root cell
        generateSegments(octree_root);                                          // traverse through the octree and generate segments for all LEAF cells
        editTransitionalFaces();                                                // resolve transitional faces
        traceComponent();                                                       // trace the strips into segments and components
        tessellationTraversal(octree_root, mesh);                               // traverse the tree again meshing all the components
        mesh.vertices = vertices;                                               // copy the vertices onto the mesh
    }

    glm::vec3 find_zero(unsigned int quality, const point_t& p0, const point_t& p1)
    {
        float alpha = p0.value / (p0.value - p1.value);
        glm::vec3 p = p0.position + alpha * (p1.position - p0.position);        // interpolate

        float value = scalar_field(p);                                          // resample
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
  
    void makeFaceSegments(const glm::ivec3 inds[], face_t* face)
    {
        const uint8_t edges =                                                   // aquire the index of the edges based on the face corner samples
            (m_sampleData[inds[0]] < 0 ? 1 : 0) |
            (m_sampleData[inds[1]] < 0 ? 2 : 0) |
            (m_sampleData[inds[2]] < 0 ? 4 : 0) |
            (m_sampleData[inds[3]] < 0 ? 8 : 0);
  
        const int8_t e0a = EDGE_MAP[edges][0][0];                               // the edges of the first strip
        const int8_t e0b = EDGE_MAP[edges][0][1];
  
        if (e0a != -1)                                                          // if edge has data generate primary strip
            makeStrip(e0a, e0b, inds, face, 0);
  
        const int8_t e1a = EDGE_MAP[edges][1][0];                               // the edges of the second strip
        const int8_t e1b = EDGE_MAP[edges][1][1];
  
        if (e1a != -1)                                                          // if both strips are on a face
            makeStrip(e1a, e1b, inds, face, 1);
    }

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
            median.position += vertices[component[i]].position;

        median.position /= float(numOfInds);
        
        float value = scalar_field(median.position);
        median.normal = glm::normalize(gradient(median.position, value));
        median.position -= value * median.normal;

        component.push_back(vertices.size());
        vertices.push_back(median);

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
    glm::vec3 gradient(const glm::vec3& position)
        { return gradient(position, scalar_field(position)); }

    glm::vec3 gradient(const glm::vec3& position, const float& value)
    {
        float gradient_delta = 0.5f * delta;
        float dx = scalar_field(glm::vec3(position.x + gradient_delta, position.y, position.z));
        float dy = scalar_field(glm::vec3(position.x, position.y + gradient_delta, position.z));
        float dz = scalar_field(glm::vec3(position.x, position.y, position.z + gradient_delta));
        return glm::vec3(dx - value, dy - value, dz - value);
    }
    
    //===================================================================================================================================================================================================================
    // traverses the octree and tessellates all the components on each LEAF cell
    //===================================================================================================================================================================================================================
    void tessellationTraversal(cell_t* cell, mesh_t& mesh)
    {
        if(!cell) return;                                                       // skip empty nodes
  
        if(cell->state == BRANCH)                                               // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                tessellationTraversal(cell->children[i], mesh);
            return;
        }
        
        for(unsigned int i = 0; i < cell->components.size(); ++i)               // the node is a leaf, tessellate segment
            tessellateComponent(mesh, cell->components[i]);
    }

    //===================================================================================================================================================================================================================
    // finds all the edges that are located inbetween the two given points, stores them in the provided vector
    // returns value of the direction in which they advance 0 - right(x), 1 - up(y), 2 - front(z)
    //===================================================================================================================================================================================================================
    int getEdgesBetwixt(Range& o_range, const glm::ivec3& pt0, const glm::ivec3& pt1) const
    {
        int direction = -1;                                                     // 0 - right(x), 1 - up(y), 2 - front(z)
        int diffX = abs(pt0.x - pt1.x);
        int diffY = abs(pt0.y - pt1.y);
        int diffZ = abs(pt0.z - pt1.z);
    
        if(diffX > 0)
        {
            int first = std::min(pt0.x, pt1.x);
            int last = std::max(pt0.x, pt1.x);
            o_range = Range(first, last);
            direction = 0;
        }
        else if(diffY > 0)
        {
            int first = std::min(pt0.y, pt1.y);
            int last = std::max(pt0.y, pt1.y);
            o_range = Range(first, last);
            direction = 1;
        }
        else if(diffZ > 0)
        {
            int first = std::min(pt0.z, pt1.z);
            int last = std::max(pt0.z, pt1.z);
            o_range = Range(first, last);
            direction = 2;
        }
    
        assert((direction >= 0) && (direction <= 2));
        return direction;
    }

    //===================================================================================================================================================================================================================
    // finds the exact place of the isovalue crossing point by tracking down the sign change, returning the index of the first point (smaller)
    //===================================================================================================================================================================================================================
    int exactSignChangeIndex(const Range& range, int& dir, glm::ivec3& ind0, glm::ivec3& ind1) const
    {
        glm::ivec3 firstIndex;                                                  // check for going from smaller to higher
        if(ind0[dir] == range.m_lower)
            firstIndex = ind0;
        else if(ind1[dir] == range.m_lower)
            firstIndex = ind1;
  
        if(fabs(range.m_lower - range.m_upper) == 1)                            // if there are only two indices, return the first one
            return firstIndex[dir];

        glm::ivec3 indexer = firstIndex;                                        // loop through all samples on the cell edge and find the sign change
        for(int i = range.m_lower; i < range.m_upper; ++i)
        {
            indexer[dir] = i;
            float thisValue = m_sampleData[indexer];

            indexer[dir] = i + 1;                                               // increment the indexer so we get the value of the next pt on the edge
            float nextValue = m_sampleData[indexer];

            if(thisValue * nextValue <= 0.0f)                                   // check current value against next one, if negative, edge found return sign change index
                return i;
        }

        assert(true);
        return -1;                                                              // returns error value (no sign change found)
    }

    //===================================================================================================================================================================================================================
    // creates a new vertex on the edge of the exact crossing point it calls the findCrossingPoint
    //===================================================================================================================================================================================================================
    void makeVertex(strip_t& strip, const int& dir, const glm::ivec3& crossingIndex0, const glm::ivec3& crossingIndex1, int index)
    {
        glm::vec3 pos0 = glm::vec3(-1.0f) + delta * glm::vec3(crossingIndex0);
        float val0 = m_sampleData[crossingIndex0];
        point_t pt0(pos0, val0);
    
        glm::vec3 pos1 = glm::vec3(-1.0f) + delta * glm::vec3(crossingIndex1);
        float val1 = m_sampleData[crossingIndex1];
        point_t pt1(pos1, val1);
    
        glm::vec3 zero_point = find_zero(zero_search_iterations, pt0, pt1);     // find the exact position and normal at the crossing point
        glm::vec3 normal = glm::normalize(gradient(zero_point));
    
        vertex_t vert(zero_point, normal);                                      // create a vertex
    
        strip.data[index] = vertices.size();                              // place the data onto the currect strip
        strip.block[index] = crossingIndex0;
        strip.dir[index] = dir;

        vertices.push_back(vert);
    
        edge_block_t edge_block = m_edgeData[crossingIndex0];                   // put the data onto the global 3D array of edges
        if(edge_block.empty)
            edge_block.empty = false;
        assert(edge_block.edge_indices[dir] == -1);
        edge_block.edge_indices[dir] = vertices.size() - 1;
        m_edgeData[crossingIndex0] = edge_block;
    }

    //===================================================================================================================================================================================================================
    // creates a strip and populates both of it's sides by calling populate strip
    //===================================================================================================================================================================================================================
    void makeStrip(int edge0, int edge1, const glm::ivec3 inds[], face_t* face, int strip_index)
    {
        assert((edge0 != -1) && (edge1 != -1));
        strip_t strip(false, edge0, edge1);
        populate_strip(strip, inds, 0);                                         // 1st edge of First Strip - e0a
        populate_strip(strip, inds, 1);                                         // 2nd edge of First Strip - e0b

        // todo :: Check for face sharp features here

        face->strips[strip_index] = strip;                                      // populate current face with the created strip
        face->skip = false;
    }

    //===================================================================================================================================================================================================================
    // populates a given strip by adding a new vertex on the exact isosurface crossing point or copying an existing one
    //===================================================================================================================================================================================================================
    void populate_strip(strip_t& strip, const glm::ivec3 inds[], int index)
    {
        const int8_t faceEdge = strip.edge[index];                              // get the edge on the currently examined face
        glm::ivec3 ind_0 = inds[VERTEX_MAP[faceEdge][0]];
        glm::ivec3 ind_1 = inds[VERTEX_MAP[faceEdge][1]];
    
        Range range;                                                            // get the range and the direction (of an edge block) which the edge represents
        int dir = getEdgesBetwixt(range, ind_0, ind_1);
        assert(abs(ind_0[dir] - ind_1[dir]) > 0);
        assert((ind_0[dir] == range.m_lower) || (ind_0[dir] == range.m_upper));
        assert((ind_1[dir] == range.m_lower) || (ind_1[dir] == range.m_upper));
    
        int signChange = exactSignChangeIndex(range, dir, ind_0, ind_1);        // find the exact sign change on that bigger edge range, getting the index of the sample, just before the sign change = edge of change
        assert((signChange >= range.m_lower) && (signChange < range.m_upper));
    
        glm::ivec3 crossingIndex_0 = ind_0;                                     // set the exact two point indices between the zero crossing
        glm::ivec3 crossingIndex_1 = ind_0;
        crossingIndex_0[dir] = signChange;
        crossingIndex_1[dir] = signChange + 1;
        assert(m_sampleData[crossingIndex_0] * m_sampleData[crossingIndex_1] <= 0.0f);
    
        bool duplicate = false;                                                 // check for duplicate vertices on the same edge
        if(!m_edgeData[crossingIndex_0].empty)                                  // check global datastructor edgeblock
        {
            if(m_edgeData[crossingIndex_0].edge_indices[dir] != -1)             // check exact global edge
            {
                strip.data[index] = m_edgeData[crossingIndex_0].edge_indices[dir];
                strip.block[index] = crossingIndex_0;
                strip.dir[index] = dir;
                duplicate = true;
            }
        }
    
        if(!duplicate)                                                          // if there is no previous vertex registered to that edge, proceed to find it.
            makeVertex(strip, dir, crossingIndex_0, crossingIndex_1, index);
    }

    //===================================================================================================================================================================================================================
    // starts from a given node, traverses through the octree and generates segments for all LEAF cells
    //===================================================================================================================================================================================================================
    void generateSegments(cell_t* cell)
    {
        if(!cell) return;                                                       // skip empty nodes
  
        if(cell->state == BRANCH)                                               // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                generateSegments(cell->children[i]);
            return;
        }

        for(int f = 0; f < 6; ++f)                                              // for all the faces in this LEAF cell
        {
            glm::ivec3 indices[4];                                              // the node is a leaf, generate segment   
            for(int v = 0; v < 4; ++v)                                          // convert face vertex index to cell vertex index
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
    
        for(unsigned int i = 0; i < cells.size(); ++i)                          // loop through all cells and all faces and find every transitional face
            for(int j = 0; j < 6; ++j)
                if(cells[i]->faces[j]->state == TRANSIT_FACE)
                    resolveTransitionalFace(cells[i]->faces[j]);                // pass it for getting the data from it's twin
    }

    //===================================================================================================================================================================================================================
    // for a given transitional face, collects all segments
    //===================================================================================================================================================================================================================
    void segmentFromTwin(face_t* face, std::vector<unsigned int>& component, int lastData, int& currentEdge)
    {
        assert(face->twin->state == BRANCH_FACE);
        assert(face->twin->transitSegs.size() > 0);
  
        for(unsigned int i = 0; i < face->twin->transitSegs.size(); ++i)        // Looping through all transitional Face separate segments
        {
            if(face->twin->strips[i].skip == false)
            {
                if(lastData == face->twin->strips[i].data[0])
                {
                    assert((face->twin->transitSegs[i][0] == (unsigned)face->twin->strips[i].data[0]) ||
                           (face->twin->transitSegs[i][0] == (unsigned)face->twin->strips[i].data[1]));

                    for(unsigned int s = 1; s < face->twin->transitSegs[i].size(); ++s)
                        component.push_back(face->twin->transitSegs[i][s]);
                    currentEdge = face->twin->strips[i].edge[1];
                    face->twin->strips[i].skip = true;
                }
                else if(lastData == face->twin->strips[i].data[1])
                {
                    for(unsigned s=face->twin->transitSegs[i].size() - 1; s > 0; --s)
                        component.push_back(face->twin->transitSegs[i][s - 1]);
                    currentEdge = face->twin->strips[i].edge[0];
                    face->twin->strips[i].skip = true;
                }
            }
        }
    }

    //===================================================================================================================================================================================================================
    // collects all the strips of a cell into a single array, populates another array with transitional segments in case the cell has transitional faces
    //===================================================================================================================================================================================================================
    void collectStrips(cell_t* c, std::vector<strip_t>& o_cellStrips, std::vector<std::vector<unsigned int>>& o_transitSegs)
    {
        for(int f = 0; f < 6; ++f)                                              // Looping through all faces of the cell
        {
            if(c->faces[f]->state == LEAF_FACE)                                 // If it is a leaf face, just copy the full strips
            {
                for(unsigned int i = 0; i < c->faces[f]->strips.size(); ++i)
                {
                    if(c->faces[f]->strips[i].data[0] != -1)                    // Check there is valid data
                        o_cellStrips.push_back(c->faces[f]->strips[i]);         // Create a temp strip and store the cell edges in it
                }
            }
            else if(c->faces[f]->state == TRANSIT_FACE)                         // For a Transitional Face, we must take the transit segment too as it contains all the data (vertices in between the start and end of strip)
            {
                if(!c->faces[f]->twin)                                          // Check if there is a valid twin
                    break;
    
                assert(c->faces[f]->twin->strips.size() == c->faces[f]->twin->transitSegs.size());
          
                for(unsigned int i = 0; i < c->faces[f]->twin->strips.size(); ++i)            
                {
                    if(c->faces[f]->twin->strips[i].data[0] != -1)              // Push the current strips and transit
                    {
                        o_transitSegs.push_back(c->faces[f]->twin->transitSegs[i]);
                        o_cellStrips.push_back(c->faces[f]->twin->strips[i]);
                    }
                }
            }
        }
        assert(o_cellStrips.size() > 0);                                        // A Leaf cell must have at least 3 strips to form a component unless it is has just a single looping component from a transit face
    }


    //===================================================================================================================================================================================================================
    // takes all the strips of a given cell and links them together to form components
    //===================================================================================================================================================================================================================
    void linkStrips(std::vector<unsigned int>& components, std::vector<strip_t>& strips, std::vector<std::vector<unsigned int>>& transitSegs)
    {
        assert(components.size() == 0);

        int addedInIteration;
        bool backwards;
    
        components.push_back(strips[0].data[0]);                                // add a new value to the beginning

        do
        {
            addedInIteration = 0;
        
            for(unsigned int i = 0; i < strips.size(); ++i)                     // Loop through all current strips
            {
                unsigned int s_d0 = strips[i].data[0];
                unsigned int s_d1 = strips[i].data[1];
    
                if(components.back() == s_d0)
                {
                    backwards = false;
                    bool transit = false;
              
                    if(transitSegs.size() > 0)                              // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(components, transitSegs, strips[i], transit, addedInIteration, backwards);
              
                    if(!transit)                                            // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.push_back(s_d1);
                        ++addedInIteration;
                    }
                }
                else if(components.back() == s_d1)
                {
                    backwards = true;
                    bool transit = false;
              
                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(components, transitSegs, strips[i], transit, addedInIteration, backwards); 
              
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.push_back(s_d0);
                        ++addedInIteration;
                    }
                }
                else
                    continue;                                                       // skip to next iteration
          
                strips.erase(strips.begin() + i);                                   // Delete the currently added strip
            }
        
            if(components.front() == components.back())                                     // Check whether the component closes on itself
                components.erase(components.begin());                                       // delete first vertex as it is duplex with last
        }
        while(addedInIteration > 0);
    
        do
        {
            addedInIteration = 0;
    
            for(unsigned int i = 0; i < strips.size(); ++i)                         // Loop through all current strips
            {
                unsigned int s_d0 = strips[i].data[0];
                unsigned int s_d1 = strips[i].data[1];
    
                if(components.front() == s_d0)
                {
                    bool transit = false;
    
                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, checks for matching segment and insert from twin if found
                        insertDataFromTwin(components, transitSegs, strips[i], transit, addedInIteration, false);
    
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.insert(components.begin(), s_d1);
                        ++addedInIteration;
                    }
                }
                else if(components.front() == s_d1)
                {
                    bool transit = false;
    
                    if(transitSegs.size() > 0)                                  // If there are no transitSegs, no point in checking, check for matching segment and insert from twin if found
                        insertDataFromTwin(components, transitSegs, strips[i], transit, addedInIteration, true);
              
                    if(!transit)                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.insert(components.begin(), s_d0);
                        ++addedInIteration;
                    }
                }
                else
                    continue;                                                       // skip to next iteration
          
                strips.erase(strips.begin() + i);                                   // delete the currently added strip
            }
    
        
            if(components.front() == components.back())                                     // check whether the component closes on itself
                components.erase(components.begin());                                       // delete first vertex as it is duplex with last    
        }
        while(addedInIteration > 0);
    
        assert(components.size() >= 3);
    }

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
    void insertDataFromTwin(std::vector<unsigned int>& components, std::vector<std::vector<unsigned int>>& segments, strip_t& strip, bool& transit, int& addedInIter, bool backwards)
    {
        for(unsigned int i = 0; i < segments.size(); ++i)                           // loop through all the transitional strips and find the one corresponding to this strip
        {
            if(compareStripToSeg(strip, segments[i]))                               // check if the strip's data matches the segment
            {
                if(backwards)
                {
                    for(int j = segments[i].size() - 1; j > 0; --j)
                        components.push_back(segments[i][j - 1]);
                }
                else
                {
                    for(unsigned int j = 1; j < segments[i].size(); ++j)
                        components.push_back(segments[i][j]);
                }

                segments.erase(segments.begin() + i);
                ++addedInIter;
                transit = true;
                break;
            }
        }
    }

    //===================================================================================================================================================================================================================
    // takes a transitional face and collects all the strips from it's twin linking them those that can get linked together
    //===================================================================================================================================================================================================================
    void resolveTransitionalFace(face_t* face)
    {
        assert(octree->cells[face->twin->cell_index]->state == BRANCH);             // Check if twin face belongs to a Branch cell as it should!
        assert(face->twin->state != TRANSIT_FACE);
  
        std::vector< std::vector<unsigned int> > transitSegs;                       // Get twin and traverse all it's children collecting all non-empty strips
        std::vector<strip_t> allStrips;                                             // todo :: get only addresses??
        traverseFace(face->twin, allStrips);
      
        if(allStrips.size() == 0)                                                   // If there are no strips on twin face
        {
            face->state = LEAF_FACE;
            return;
        }
    
        do
        {
            std::vector<unsigned int> vertInds;
            strip_t longStrip;
            vertInds.push_back(allStrips[0].data[0]);
            vertInds.push_back(allStrips[0].data[1]);
            longStrip = allStrips[0];
            allStrips.erase(allStrips.begin());
            int addedInIteration;
    
            do                                                                      // Loop through all the segments, removing the visited ones until there are no more segments / there is a loop / or there are 0 new segments added
            {
                addedInIteration = 0;
                for(unsigned int i = 0; i < allStrips.size(); ++i)                  // Checking Forward
                {
                    if(vertInds[vertInds.size()-1] == (unsigned int) allStrips[i].data[0])
                    {
                        vertInds.push_back(allStrips[i].data[1]);
                        longStrip.changeBack(allStrips[i], 1);                      // adding info to longStrip end
                        ++addedInIteration;
                    }
                    else if(vertInds[vertInds.size()-1] == (unsigned int) allStrips[i].data[1])
                    {
                        vertInds.push_back(allStrips[i].data[0]);
                        longStrip.changeBack(allStrips[i], 0);                      // adding info to longStrip end
                        ++addedInIteration;
                    }
                    else
                        continue;
    
                    allStrips.erase(allStrips.begin() + i);                         // delete the currently added strip
    
                    if(vertInds[0] == vertInds[vertInds.size()-1])
                    {
                        vertInds.erase(vertInds.begin());                           // delete last vertex as it is duplex
                        longStrip.loop = true;
                    }
                }
    
            }
            while((allStrips.size() > 0) && (addedInIteration > 0) && (longStrip.loop == false));
            
            if((longStrip.loop == false)&&(allStrips.size() > 0))                   // Continue if it is a full or looping strip
            {
                do                                                                  // Check Backward
                {
                    addedInIteration = 0;
    
                    for(unsigned int i = 0; i < allStrips.size(); ++i)
                    {
                        if(vertInds[0] == (unsigned) allStrips[i].data[0])
                        {
                            vertInds.insert(vertInds.begin(), allStrips[i].data[1]);
                            longStrip.changeFront(allStrips[i], 1);                 // adding info to longStrip beginning
                            ++addedInIteration;
                        }
                        else if(vertInds[0] == (unsigned) allStrips[i].data[1])
                        {
                            vertInds.insert(vertInds.begin(), allStrips[i].data[0]);
                            longStrip.changeFront(allStrips[i], 0);                 // adding info to longStrip beginning
                            ++addedInIteration;
                        }
                        else
                            continue;
    
                        allStrips.erase(allStrips.begin()+i);                       // delete the currently added strip
    
                        if(vertInds[0] == vertInds[vertInds.size() - 1])
                        {
                            vertInds.erase(vertInds.begin());                       // delete last vertex as it is duplex
                            longStrip.loop = true;
                        }
                    }
                }
                while((allStrips.size() > 0) && (addedInIteration > 0) && (longStrip.loop == false));
            }
        
            longStrip.skip = false;                                                 // Push the segment onto the face seg array
            face->twin->strips.push_back(longStrip);
            transitSegs.push_back(vertInds);
        }
        while(allStrips.size() != 0);
      
        if(transitSegs.size() != 0)                                                 // Load them segments onto the twin face
            face->twin->transitSegs = transitSegs;
    
        transitSegs.clear();                                                        // Clear the vectors
        allStrips.clear();
    }

    //===================================================================================================================================================================================================================
    // loops through all levels of the octree from the leafs up to the root and looping through all cells linking their strips into components
    //===================================================================================================================================================================================================================
    void traceComponent()
    {
        std::vector<cell_t*> cells = octree->cells;
    
        for(unsigned int i = 0; i < max_level; ++i)                                 // trace the strips into segments and components
            for(unsigned int j = 0; j < cells.size(); ++j)                          // loop through all cells and link components for all LEAF cells
                if(cells[j]->level == max_level - i)
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