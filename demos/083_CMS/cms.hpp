#ifndef _cms_alg_included_78946375463727856102735651025612510235605612852735682
#define _cms_alg_included_78946375463727856102735651025612510235605612852735682

#include <vector>

#include <glm/glm.hpp>

#include "edge.hpp"
#include "cell.hpp"
#include "array3d.hpp"
#include "octree.hpp"
#include "tables.hpp"

namespace cms
{

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
    int depth;

    scalar_field_t scalar_field;
    Array3D<float> field_values;                                                 // The sampling 1D array masked in an Array3D wrapper class
    Array3D<edge_block_t> m_edgeData;                                           // The edgeblock 1D array masked in an Array3D wrapper class EdgeBlock has 3 edges
    std::vector<vertex_t> vertices;                                             // The vertex array, storing all the vertices in the mesh
    octree_t<scalar_field_t>* octree;                                           // the octree of the current function

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
        depth = 1 << max_level;
        zero_search_iterations = 5;
        complex_surface_threshold = 0.6f;
        delta = 2.0f / depth;
        
        field_values.resize(depth + 1);                                         // Resizing the samplingData array and proceeding with the sampling
        m_edgeData.resize(depth + 1);

        octree = new octree_t<scalar_field_t>(depth, field_values, min_level, max_level, delta, complex_surface_threshold);
    
        glm::ivec3 index;
        glm::vec3 position;

        position.x = -1.0f;
        for(index.x = 0; index.x <= depth; ++index.x)
        {
            position.y = -1.0f;
            for(index.y = 0; index.y <= depth; ++index.y)
            {
                position.z = -1.0f;
                for(index.z = 0; index.z <= depth; ++index.z)
                {
                    field_values[index] = scalar_field(position);
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
        octree_root = octree->build();                                          // recursively generate the octree and get the octree root cell
        generate_segments(octree_root);                                         // traverse through the octree and generate segments for all LEAF cells
        resolve();                                                              // resolve transitional faces
        traceComponent();                                                       // trace the strips into segments and components
        tessellate_cell(octree_root, mesh);                                     // traverse the tree again meshing all the components
        mesh.vertices = vertices;                                               // copy the vertices onto the mesh
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
    void tessellate_cell(cell_t* cell, mesh_t& mesh)
    {
        if(!cell) return;                                                       // skip empty nodes
  
        if(!cell->leaf)                                               // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                tessellate_cell(cell->children[i], mesh);
            return;
        }
        
        for(unsigned int i = 0; i < cell->components.size(); ++i)               // the node is a leaf, tessellate segment
        {
            std::vector<unsigned int>& component = cell->components[i];

            int size = component.size();
            if(size == 3)                                                      // three vertices - just output a plain triangle
            {
                for(int i = 0; i < 3; ++i)
                    mesh.indices.push_back(component[i]);
                return;
            }                                    

            glm::vec3 position = glm::vec3(0.0f);
            for(unsigned int i = 0; i < component.size(); ++i)
                position += vertices[component[i]].position;

            position /= float(size);
        
            float value = scalar_field(position);
            glm::vec3 normal = glm::normalize(gradient(position, value));
            position -= value * normal;

            component.push_back(vertices.size());
            vertices.push_back(vertex_t(position, normal));

            for(unsigned int i = 0; i < component.size() - 2; ++i)                  // -2 because median index is at (size-1) and we stich end to begin later
            {
                mesh.indices.push_back(component[component.size() - 1]);
                mesh.indices.push_back(component[i]);
                mesh.indices.push_back(component[i + 1]);
            }
    
            mesh.indices.push_back(component[component.size() - 1]);                // Connecting the last and the first
            mesh.indices.push_back(component[component.size() - 2]);
            mesh.indices.push_back(component[0]);
        }
    }

    //===================================================================================================================================================================================================================
    // populates a given strip by adding a new vertex on the exact isosurface crossing point or copying an existing one
    //===================================================================================================================================================================================================================
    void populate_strip(strip_t& strip, const glm::ivec3 inds[], int index)
    {
        const int8_t faceEdge = strip.edge[index];                              // get the edge on the currently examined face
        glm::ivec3 ind_0 = inds[VERTEX_MAP[faceEdge][0]];
        glm::ivec3 ind_1 = inds[VERTEX_MAP[faceEdge][1]];
    
        int p, q;
        int dir;
        glm::ivec3 firstIndex = ind_0;

        if(ind_0.x != ind_1.x)
        {
            p = std::min(ind_0.x, ind_1.x);
            q = std::max(ind_0.x, ind_1.x);
            firstIndex.x = p;
            dir = 0;
        }
        else if(ind_0.y != ind_1.y)
        {
            p = std::min(ind_0.y, ind_1.y);
            q = std::max(ind_0.y, ind_1.y);
            firstIndex.y = p;
            dir = 1;
        }
        else
        {
            p = std::min(ind_0.z, ind_1.z);
            q = std::max(ind_0.z, ind_1.z);
            firstIndex.z = p;
            dir = 2;
        }

        int signChange = firstIndex[dir];

        if(p + 1 != q)
        {
            for(int i = p; i < q; ++i)
            {
                firstIndex[dir] = i;
                float thisValue = field_values[firstIndex];

                firstIndex[dir] = i + 1;                                               // increment the indexer so we get the value of the next pt on the edge
                float nextValue = field_values[firstIndex];

                if(thisValue * nextValue <= 0.0f)
                {
                    signChange = i;
                    break;
                }                                   // check current value against next one, if negative, edge found return sign change index
            }
        }

    
        glm::ivec3 ci0 = ind_0;                                     // set the exact two point indices between the zero crossing
        glm::ivec3 ci1 = ind_0;
        ci0[dir] = signChange;
        ci1[dir] = signChange + 1;
    
        if(!m_edgeData[ci0].empty)                                  // check for duplicate vertices on the same edge, if found we are done
        {
            if(m_edgeData[ci0].edge_indices[dir] != -1)             // check exact global edge
            {
                strip.data[index] = m_edgeData[ci0].edge_indices[dir];
                strip.block[index] = ci0;
                strip.dir[index] = dir;
                return;
            }
        }

        //===============================================================================================================================================================================================================
        // creates a new vertex on the edge of the exact crossing point it calls the findCrossingPoint
        //===============================================================================================================================================================================================================
        glm::vec3 pos0 = glm::vec3(-1.0f) + delta * glm::vec3(ci0);
        float val0 = field_values[ci0];
    
        glm::vec3 pos1 = glm::vec3(-1.0f) + delta * glm::vec3(ci1);
        float val1 = field_values[ci1];
    
        glm::vec3 zero_point;

        for(int i = 0; i < zero_search_iterations; ++i)
        {
            float alpha = val0 / (val0 - val1);
            zero_point = pos0 + alpha * (pos1 - pos0);
            float value = scalar_field(zero_point);
            bool first_half = (value < 0.0f) ^ (val0 < 0.0f);

            if(first_half)
            {
                pos1 = zero_point;
                val1 = value;
            }
            else
            {
                pos0 = zero_point;
                val0 = value;
            }
        }

        glm::vec3 normal = glm::normalize(gradient(zero_point));
    
        strip.data[index] = vertices.size();                              // place the data onto the currect strip
        strip.block[index] = ci0;
        strip.dir[index] = dir;

        vertices.push_back(vertex_t(zero_point, normal));
    
        edge_block_t edge_block = m_edgeData[ci0];                   // put the data onto the global 3D array of edges
        edge_block.empty = false;

        edge_block.edge_indices[dir] = vertices.size() - 1;
        m_edgeData[ci0] = edge_block;
    }

    //===================================================================================================================================================================================================================
    // starts from a given node, traverses through the octree and generates segments for all LEAF cells
    //===================================================================================================================================================================================================================
    void generate_segments(cell_t* cell)
    {
        if(!cell) return;                                                       // skip empty nodes
  
        if(!cell->leaf)                                               // if the node is a BRANCH go deeper
        {
            for(int i = 0; i < 8; ++i)
                generate_segments(cell->children[i]);
            return;
        }

        for(int f = 0; f < 6; ++f)                                              // for all the faces in this LEAF cell
        {
            glm::ivec3 indices[4];                                              // the node is a leaf, generate segment   
            for(int v = 0; v < 4; ++v)                                          // convert face vertex index to cell vertex index
            {
                const uint8_t vindex = FACE_VERTEX[f][v];
                indices[v] = cell->corners[vindex];
            }
            cell->faces[f]->strips.resize(2);
            face_t* face = cell->faces[f];

            const uint8_t edges =                                                       // aquire the index of the edges based on the face corner field values
                (field_values[indices[0]] < 0 ? 1 : 0) | (field_values[indices[1]] < 0 ? 2 : 0) |
                (field_values[indices[2]] < 0 ? 4 : 0) | (field_values[indices[3]] < 0 ? 8 : 0);
  
            const int8_t e0a = EDGE_MAP[edges][0][0];                                   // the edges of the first strip  
            if (e0a != -1)                                                              // if edge has data generate primary strip
            {
                const int8_t e0b = EDGE_MAP[edges][0][1];
                strip_t strip(false, e0a, e0b);
                populate_strip(strip, indices, 0);                                         // 1st edge of first strip - e0a
                populate_strip(strip, indices, 1);                                         // 2nd edge of first strip - e0b
                face->strips[0] = strip;                                                // populate current face with the created strip
                face->skip = false;
            }

            const int8_t e1a = EDGE_MAP[edges][1][0];                                   // the edges of the second strip
            if (e1a != -1)                                                              // if both strips are on a face
            {
                const int8_t e1b = EDGE_MAP[edges][1][1];
                strip_t strip(false, e1a, e1b);
                populate_strip(strip, indices, 0);                                         // 1st edge of second strip - e0a
                populate_strip(strip, indices, 1);                                         // 2nd edge of second Strip - e0b
                face->strips[1] = strip;                                                // populate current face with the created strip
                face->skip = false;
            }
        }
    }

    //===================================================================================================================================================================================================================
    // loops through all transitional faces and calls the resolve function on them
    //===================================================================================================================================================================================================================
    void resolve()
    {
        std::vector<cell_t*> cells = octree->cells;
        for(unsigned int i = 0; i < cells.size(); ++i)                          // loop through all cells and all faces and find every transitional face
            for(int j = 0; j < 6; ++j)
                if(cells[i]->faces[j]->state == TRANSIT_FACE)
                    resolve_face(cells[i]->faces[j]);                // pass it for getting the data from it's twin
    }

    //===================================================================================================================================================================================================================
    // takes a transitional face and collects all the strips from it's twin linking those that can get linked together
    //===================================================================================================================================================================================================================
    void resolve_face(face_t* face)
    {
        std::vector< std::vector<unsigned int>> transit_segments;                       // Get twin and traverse all it's children collecting all non-empty strips
        std::vector<strip_t> strips;                                             // todo :: get only addresses??
        traverse_face(face->twin, strips);
      
        if(!strips.size())                                                   // If there are no strips on twin face
        {
            face->state = LEAF_FACE;
            return;
        }
    
        do
        {
            std::vector<unsigned int> indices;
            strip_t long_strip;
            indices.push_back(strips[0].data[0]);
            indices.push_back(strips[0].data[1]);
            long_strip = strips[0];
            strips.erase(strips.begin());
            int added;
    
            do                                                                      // Loop through all the segments, removing the visited ones until there are no more segments / there is a loop / or there are 0 new segments added
            {
                added = 0;
                for(unsigned int i = 0; i < strips.size(); ++i)                  // Checking Forward
                {
                    if(indices.back() == strips[i].data[0])
                    {
                        indices.push_back(strips[i].data[1]);
                        long_strip.changeBack(strips[i], 1);                      // adding info to longStrip end
                        ++added;
                    }
                    else if(indices.back() == strips[i].data[1])
                    {
                        indices.push_back(strips[i].data[0]);
                        long_strip.changeBack(strips[i], 0);                      // adding info to longStrip end
                        ++added;
                    }
                    else
                        continue;
    
                    strips.erase(strips.begin() + i);                         // delete the currently added strip
    
                    if(indices[0] == indices.back())
                    {
                        indices.erase(indices.begin());                           // delete last vertex as it is duplex
                        long_strip.loop = true;
                    }
                }
    
            }
            while(strips.size() && added && !long_strip.loop);
            
            if(!long_strip.loop && strips.size())                   // Continue if it is a full or looping strip
            {
                do                                                                  // Check Backward
                {
                    added = 0;
    
                    for(unsigned int i = 0; i < strips.size(); ++i)
                    {
                        if(indices[0] == strips[i].data[0])
                        {
                            indices.insert(indices.begin(), strips[i].data[1]);
                            long_strip.changeFront(strips[i], 1);                 // adding info to longStrip beginning
                            ++added;
                        }
                        else if(indices[0] == strips[i].data[1])
                        {
                            indices.insert(indices.begin(), strips[i].data[0]);
                            long_strip.changeFront(strips[i], 0);                 // adding info to longStrip beginning
                            ++added;
                        }
                        else
                            continue;
    
                        strips.erase(strips.begin() + i);                       // delete the currently added strip
    
                        if(indices[0] == indices.back())
                        {
                            indices.erase(indices.begin());                       // delete last vertex as it is duplex
                            long_strip.loop = true;
                        }
                    }
                }
                while(strips.size() && added && !long_strip.loop);
            }
        
            long_strip.skip = false;                                                 // Push the segment onto the face seg array
            face->twin->strips.push_back(long_strip);
            transit_segments.push_back(indices);
        }
        while(strips.size());
      
        if(transit_segments.size())                                                 // Load them segments onto the twin face
            face->twin->transit_segments = transit_segments;
    }


    //===================================================================================================================================================================================================================
    // for a given transitional face, collects all segments
    //===================================================================================================================================================================================================================
    void segmentFromTwin(face_t* face, std::vector<unsigned int>& component, int lastData, int& currentEdge)
    {
        for(unsigned int i = 0; i < face->twin->transit_segments.size(); ++i)        // Looping through all transitional Face separate segments
        {
            if(!face->twin->strips[i].skip)
            {
                if(lastData == face->twin->strips[i].data[0])
                {
                    for(unsigned int s = 1; s < face->twin->transit_segments[i].size(); ++s)
                        component.push_back(face->twin->transit_segments[i][s]);
                    currentEdge = face->twin->strips[i].edge[1];
                    face->twin->strips[i].skip = true;
                }
                else if(lastData == face->twin->strips[i].data[1])
                {
                    for(unsigned int s = face->twin->transit_segments[i].size() - 1; s > 0; --s)
                        component.push_back(face->twin->transit_segments[i][s - 1]);
                    currentEdge = face->twin->strips[i].edge[0];
                    face->twin->strips[i].skip = true;
                }
            }
        }
    }

    //===================================================================================================================================================================================================================
    // takes all the strips of a given cell and links them together to form components
    //===================================================================================================================================================================================================================
    void linkStrips(std::vector<unsigned int>& components, std::vector<strip_t>& strips, std::vector<std::vector<unsigned int>>& transit_segments)
    {
        int added;
    
        components.push_back(strips[0].data[0]);                                // add a new value to the beginning

        do
        {
            added = 0;
        
            for(unsigned int i = 0; i < strips.size(); ++i)                     // Loop through all current strips
            {
                unsigned int s_d0 = strips[i].data[0];
                unsigned int s_d1 = strips[i].data[1];
    
                if(components.back() == s_d0)
                {
                    if(!insert_twin_data(components, transit_segments, strips[i], added, false))                                            // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.push_back(s_d1);
                        ++added;
                    }
                }
                else if(components.back() == s_d1)
                {
                    if(!insert_twin_data(components, transit_segments, strips[i], added, true))                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.push_back(s_d0);
                        ++added;
                    }
                }
                else
                    continue;                                                       // skip to next iteration
          
                strips.erase(strips.begin() + i);                                   // Delete the currently added strip
            }
        
            if(components.front() == components.back())                                     // Check whether the component closes on itself
                components.erase(components.begin());                                       // delete first vertex as it is duplex with last
        }
        while(added);
    
        do
        {
            added = 0;
    
            for(unsigned int i = 0; i < strips.size(); ++i)                         // Loop through all current strips
            {
                unsigned int s_d0 = strips[i].data[0];
                unsigned int s_d1 = strips[i].data[1];
    
                if(components.front() == s_d0)
                {
                    if(!insert_twin_data(components, transit_segments, strips[i], added, false))                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.insert(components.begin(), s_d1);
                        ++added;
                    }
                }
                else if(components.front() == s_d1)
                {
                    if(!insert_twin_data(components, transit_segments, strips[i], added, true))                                                // If the strip does not belong to a transitional face just get the next value from the strip
                    {
                        components.insert(components.begin(), s_d0);
                        ++added;
                    }
                }
                else
                    continue;                                                       // skip to next iteration
          
                strips.erase(strips.begin() + i);                                   // delete the currently added strip
            }
    
        
            if(components.front() == components.back())                                     // check whether the component closes on itself
                components.erase(components.begin());                                       // delete first vertex as it is duplex with last    
        }
        while(added);
    }

    //===================================================================================================================================================================================================================
    // inserts data from twin of a transitional face using provided segments
    //===================================================================================================================================================================================================================
    bool insert_twin_data(std::vector<unsigned int>& components, std::vector<std::vector<unsigned int>>& segments, const strip_t& strip, int& added, bool backwards)
    {
        unsigned int s0 = strip.data[0];
        unsigned int s1 = strip.data[1];
        for(unsigned int i = 0; i < segments.size(); ++i)                           // loop through all the transitional strips and find the one corresponding to this strip
        {
            std::vector<unsigned int>& segment = segments[i];
            if(((segment.front() == s0) && (segment.back() == s1)) || ((segment.front() == s1) && (segment.back() == s0)))
            {
                if(backwards)
                {
                    for(int j = segment.size() - 1; j > 0; --j)
                        components.push_back(segment[j - 1]);
                }
                else
                {
                    for(unsigned int j = 1; j < segment.size(); ++j)
                        components.push_back(segment[j]);
                }

                segments.erase(segments.begin() + i);
                ++added;
                return true;
            }
        }
        return false;
    }

    //===================================================================================================================================================================================================================
    // loops through all levels of the octree from the leafs up to the root and looping through all cells linking their strips into components
    //===================================================================================================================================================================================================================
    void traceComponent()
    {
        std::vector<cell_t*>& cells = octree->cells;
    
        for(unsigned int level = 0; level < max_level; ++level)                                 // trace the strips into segments and components
            for(unsigned int c = 0; c < cells.size(); ++c)                                      // loop through all cells and link components for all LEAF cells
            {
                cell_t& cell = *(cells[c]);
                if((cell.level == max_level - level) && cell.leaf)                             // trace the segments to form components
                {                                              
                    std::vector<strip_t> cell_strips;
                    std::vector<std::vector<unsigned int>> transit_segments;
                    std::vector<unsigned int> component;

                    for(int f = 0; f < 6; ++f)                                              // collect all the strips from that cell
                    {
                        if(cell.faces[f]->state == LEAF_FACE)                                 // If it is a leaf face, just copy the full strips
                        {
                            for(unsigned int i = 0; i < cell.faces[f]->strips.size(); ++i)
                            {
                                if(cell.faces[f]->strips[i].data[0] != -1)                    // Check there is valid data
                                    cell_strips.push_back(cell.faces[f]->strips[i]);         // Create a temp strip and store the cell edges in it
                            }
                        }
                        else if(cell.faces[f]->state == TRANSIT_FACE)                         // for a transitional face, we must take the transit segment too as it contains all the data (vertices in between the start and end of strip)
                        {
                            if(!cell.faces[f]->twin) break;                                   // check if there is a valid twin
    
                            for(unsigned int i = 0; i < cell.faces[f]->twin->strips.size(); ++i)            
                            {
                                if(cell.faces[f]->twin->strips[i].data[0] != -1)              // Push the current strips and transit
                                {
                                    transit_segments.push_back(cell.faces[f]->twin->transit_segments[i]);
                                    cell_strips.push_back(cell.faces[f]->twin->strips[i]);
                                }
                            }
                        }
                    }
                    
                    while(cell_strips.size())                                    // link the strips into components
                    {
                        linkStrips(component, cell_strips, transit_segments);
                        cells[c]->components.push_back(component);
                        component.clear();
                    }
                }
            }
    }

    void traverse_face(face_t* face, std::vector<strip_t>& transit_strips)
    {
        if(!face) return;
    
        if(face->state == BRANCH_FACE)                                              // if it is a BRANCH face, traverse through all it's children
        {
            for(int i = 0; i < 4; ++i)
                traverse_face(face->children[i], transit_strips);
            return;
        }

        for(unsigned int i = 0; i < face->strips.size(); ++i)                       // it is a LEAF face, check all its strips and collect all valid
            if(!face->strips[i].skip)
                transit_strips.push_back(face->strips[i]);
    }

};

} // namespace cms

#endif // _cms_alg_included_78946375463727856102735651025612510235605612852735682