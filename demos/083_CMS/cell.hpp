#ifndef _cms_cell_included_9810568431078235067138560347562107836458974356893746
#define _cms_cell_included_9810568431078235067138560347562107836458974356893746

#include <vector>
#include <cstdint>
#include "address.hpp"
#include "vertex.hpp"
#include "range.hpp"
#include "vec3.hpp"
#include "point.hpp"
#include "index3d.hpp"
#include "face.hpp"
#include "util.hpp"

namespace cms
{

enum CellState 
{ 
    BRANCH = 0,                                                                                 // BRANCH may also mean EMPTY
    LEAF = 1
};

struct cell_t
{
    int id;
    enum CellState state;                                                                       // the state of the current cell - based on State enumerator

    cell_t* parent;                                                                             // pointer to this cell's parent cell
    cell_t* children[8];                                                                        // pointer array of the branching cells
    cell_t* neighbours[6];
    face_t* faces[6];                                                                           // array of cell faces

    uint8_t level;                                                                              // the level of subdivision of the cell
    uint8_t max_level;                                                                          // the maximum subdivision of the octree (needed for Address)

    std::vector<int8_t> raw_address;                                                            // The address of the cell
    address_t address;

    Index3D index;                                                                              // the index of the point at the 000 corner of the cell
    Index3D offset;                                                                             // the discrete dimensions of the cell based on the samples
    Index3D point_indices[8];                                                                   // Indices of the samples at the corners of the cell
    int8_t position_in_parent;                                                                  // Position within parent
    
    std::vector<std::vector<unsigned int>> m_components;                                        // The array of components for this cell

    //===================================================================================================================================================================================================================
    // constructor and destructor
    //===================================================================================================================================================================================================================
    
    // int id the cell's unique identifier
    // CellState state - the cell state
    // Cell* parent - a pointer to the parent cell (null if root)
    // uint8_t level - the level of subdivision
    // Index3D index - the 000 corner of the cell in the samples array
    // Index3D offset - the dimensions of the cell in sample slabs xyz

    cell_t(int id, CellState state, cell_t* parent, uint8_t level, Index3D index, Index3D offset, int8_t position_in_parent)
        : id(id), state(state), parent(parent), level(level),
          index(index), offset(offset), position_in_parent(position_in_parent)
    {
        for(int i = 0; i < 8; ++i)                                                                  // Initialising the 8 octree children to NULL
            children[i] = 0;
        
        for(int i = 0; i < 6; ++i)                                                                  // Generating the 6 cell faces / HEAP /
            faces[i] = new face_t(i, id);
        
        if(parent)                                                                                  // Initialise the address
            address.set(parent->address.m_rawAddress, position_in_parent + 1);
        else
            address.reset();
        
        for(int i = 0; i < 6; ++i)                                                                  // Clear the neighbour array
            neighbours[i] = 0;
    }

    ~cell_t()                                                                                         // destructor :: looping and destroying all children individually
    {
        for(int i = 0; i < 6; ++i)                                                                  // delete the six faces of the cell
        {
            if(faces[i])
                delete faces[i];
        }
    }

    void pushComponent(std::vector<unsigned int> i_comp)                                            // push_back component onto the component vector of the cell
        { m_components.push_back(i_comp); }                                                         
                                                                                                    
    std::vector<std::vector<unsigned int>> getComponents()                                          // Retrieve the component array of the cell :: todo fix with offset
        { return m_components; }                                                                    
                                                                                                    
};

} // namespace cms

#endif // _cms_cell_included_9810568431078235067138560347562107836458974356893746
