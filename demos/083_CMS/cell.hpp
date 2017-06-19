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

    Range m_x, m_y, m_z;                                                                        // the public dimensions of the cell through ranges in xyz
    std::vector<int8_t> m_rawAddress;                                                           // The address of the cell
    Address m_address;

    Index3D index;                                                                              // the index of the point at the 000 corner of the cell
    Index3D offset;                                                                             // the discrete dimensions of the cell based on the samples
    Index3D m_pointInds[8];                                                                     // Indices of the samples at the corners of the cell
    int8_t m_posInParent;                                                                       // Position within parent
    
    std::vector<std::vector<unsigned int>> m_components;                                        // The array of components for this cell

    //===================================================================================================================================================================================================================
    // constructor and destructor
    //===================================================================================================================================================================================================================
    
    // int id the cell's unique identifier
    // CellState state - the cell state
    // Cell* parent - a pointer to the parent cell (null if root)
    // uint8_t level - the level of subdivision
    // Index3D i_c000 - the 000 corner of the cell in the samples array
    // Index3D i_offset - the dimensions of the cell in sample slabs xyz

    cell_t(int id, CellState state, cell_t* parent, uint8_t level, Index3D index, Index3D offset, int8_t i_posInPar)
        : id(id), state(state), parent(parent), level(level),
          index(index), offset(offset), m_posInParent(i_posInPar)
    {
        for(int i = 0; i < 8; ++i)                                                                  // Initialising the 8 octree children to NULL
            children[i] = 0;
        
        for(int i = 0; i < 6; ++i)                                                                  // Generating the 6 cell faces / HEAP /
            faces[i] = new face_t(i, id);
        
        if(parent)                                                                                  // Initialise the address
            m_address.set(parent->m_address.getRaw(), m_posInParent + 1);
        else
            m_address.reset();
        
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

    //============== Accessors and Mutators =============
/*
    void setC000(Index3D i_c000)                                                                    // Setting the corner 000 3d index
        { m_c000 = i_c000; }
    
    const Index3D& getC000() const                                                                  // Getting a read-only 3d index of the corner at 000
        { return m_c000; }

    void setOffsets(Index3D i_offsets)                                                              // Setting the xyz offset (dimensions) of the cell in discrete samples
        { m_offsets = i_offsets; }
  
    const Index3D& getOffsets() const                                                               // Getting the xyz offset sample (dimensions) of the cell
        { return m_offsets; }
*/  
    void setPointInds(Index3D i_pointInds[])                                                        // Setting the 3D Index of the corner points of the cell
    {
        for(int i = 0; i < 8; ++i)
            m_pointInds[i] = i_pointInds[i];
    }
  
    const Index3D* getPointInds() const                                                             // Getting the array of indices to corner points of a cell
        { return m_pointInds; }
  
    void pushComponent(std::vector<unsigned int> i_comp)                                            // push_back component onto the component vector of the cell
        { m_components.push_back(i_comp); }                                                         
                                                                                                    
    std::vector<std::vector<unsigned int>> getComponents()                                          // Retrieve the component array of the cell :: todo fix with offset
        { return m_components; }                                                                    
                                                                                                    
    int8_t getPosInParent()                                                                         // Return a int8_t of the position of the cell in relation to it's parent
        { return m_posInParent; }

};

} // namespace cms

#endif // _cms_cell_included_9810568431078235067138560347562107836458974356893746
