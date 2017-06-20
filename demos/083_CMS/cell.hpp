#ifndef _cms_cell_included_9810568431078235067138560347562107836458974356893746
#define _cms_cell_included_9810568431078235067138560347562107836458974356893746

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include "address.hpp"
#include "range.hpp"
#include "face.hpp"

namespace cms
{

struct cell_t
{
    cell_t* parent;                                                                                 // pointer to this cell's parent cell
    cell_t* children[8];                                                                            // pointer array of the branching cells
    cell_t* neighbours[6];
    face_t* faces[6];                                                                               // array of cell faces

    uint8_t level;                                                                                  // the level of subdivision of the cell
    glm::ivec3 index;                                                                               // the index of the point at the 000 corner of the cell

    bool leaf;                                                                                      // flag indicating whether the cell is a leaf

    address_t address;                                                                              // the address of the cell

    int offset;                                                                                     // the discrete dimension of the cell based on the samples
    glm::ivec3 corners[8];                                                                          // indices of the samples at the corners of the cell
    int8_t position_in_parent;                                                                      // position within the parent
    
    std::vector<std::vector<unsigned int>> components;                                              // the array of the components for this cell

    //===================================================================================================================================================================================================================
    // constructor and destructor
    //===================================================================================================================================================================================================================
    
    cell_t(cell_t* parent, uint8_t level, const glm::ivec3& index, int offset, int8_t position_in_parent)
        : leaf(false), parent(parent), level(level), index(index), offset(offset), position_in_parent(position_in_parent)
    {
        for(int i = 0; i < 8; ++i)                                                                  // initialise the 8 octree children to 0
            children[i] = 0;
        
        for(int i = 0; i < 6; ++i)                                                                  // allocate 6 cell faces on heap
            faces[i] = new face_t(i/*, id*/);
        
        if(parent)                                                                                  // initialise the address
            address.set(parent->address.m_rawAddress, position_in_parent + 1);
        else
            address.reset();
        
        for(int i = 0; i < 6; ++i)                                                                  // clear the neighbour array
            neighbours[i] = 0;

        glm::ivec3 index0 = index;
        glm::ivec3 index1 = index + offset;
  
        corners[0] = glm::ivec3(index0.x, index0.y, index0.z);                                      // corner 000
        corners[1] = glm::ivec3(index0.x, index0.y, index1.z);                                      // corner 001
        corners[2] = glm::ivec3(index0.x, index1.y, index0.z);                                      // corner 010
        corners[3] = glm::ivec3(index0.x, index1.y, index1.z);                                      // corner 011
        corners[4] = glm::ivec3(index1.x, index0.y, index0.z);                                      // corner 100
        corners[5] = glm::ivec3(index1.x, index0.y, index1.z);                                      // corner 101
        corners[6] = glm::ivec3(index1.x, index1.y, index0.z);                                      // corner 110
        corners[7] = glm::ivec3(index1.x, index1.y, index1.z);                                      // corner 111                                                                                                    
    }

    ~cell_t()
    {
        for(int i = 0; i < 6; ++i)                                                                  // delete the six faces of the cell
        {
            if(faces[i])
                delete faces[i];
        }
    }
};

} // namespace cms

#endif // _cms_cell_included_9810568431078235067138560347562107836458974356893746
