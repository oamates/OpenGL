#include <cmath>
#include <cstdlib>
#include <fstream>

#include "octree.hpp"
#include "util.hpp"
#include "tables.hpp"

namespace cms
{

//=======================================================================================================================================================================================================================
// Table setting the face relationship
// Given the position of a cell within its parent returns the 3 faces of that cell that touch
// the parent. (they would be the same for the parent)
//=======================================================================================================================================================================================================================

static const CONTACT FACE_RELATIONSHIP_TABLE[8][3] = 
{
    {BACK,  BOTTOM, LEFT },
    {FRONT, BOTTOM, LEFT },
    {BACK,  TOP,    LEFT },
    {FRONT, TOP,    LEFT },
    {BACK,  BOTTOM, RIGHT},
    {FRONT, BOTTOM, RIGHT},
    {BACK,  TOP,    RIGHT},
    {FRONT, TOP,    RIGHT}
};

static const uint8_t SUB_FACE_TABLE[8][3] = 
{
    {0, 0, 0},
    {0, 1, 1},
    {1, 0, 2},
    {1, 1, 3},
    {2, 2, 0},
    {2, 3, 1},
    {3, 2, 2},
    {3, 3, 3}
};


//=======================================================================================================================================================================================================================
// Edge direction table :: given edge index, returns the direction of the edge (0 = x, 1 = y, 2 = z)
//=======================================================================================================================================================================================================================
//                                         0  1  2  3  4  5  6  7  8  9  10 11
static const uint8_t EDGE_DIRECTION[12] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2};


// Cell neighbour table
// See: 'Cell Point and Subcell Layout' in tables header (mind +1 to index)
//
// @param: axis of neighbour [3]
// @param: current cell ID within parent [8]

static const uint8_t NEIGHBOUR_ADDRESS_TABLE[3][8] =
{
  // Beware neighbour slots start at 1 and not 0!
  // 0  1  2  3  4  5  6  7   Cell IDs
    {2, 1, 4, 3, 6, 5, 8, 7}, //Z (BACK & FRONT) NEIGHBOUR
    {3, 4, 1, 2, 7, 8, 5, 6}, //Y (TOP & BOTTOM) NEIGHBOUR
    {5, 6, 7, 8, 1, 2, 3, 4}  //X (LEFT & RIGHT) NEIGHBOUR
};

//=======================================================================================================================================================================================================================
// Basic Octree Generation Functions 
//=======================================================================================================================================================================================================================
void octree_t::makeStructure()
{
    glm::ivec3 c000 = glm::ivec3(0);                                            // Establish Root
    glm::ivec3 offsets = samples - 1;
    root = new cell_t(0, BRANCH, 0, 0, c000, offsets, 0);
    cells.push_back(root);                                                      // Pushing the root as the first element of the cell array
    acquireCellInfo(root);                                                      // Calculating and storing information about the root cell
    subdivideCell(root);                                                        // Create the rest of the base grid recursively
}

void octree_t::acquireCellInfo(cell_t* c)
{  
    glm::ivec3 index = c->index;                                                                       // Extracting values from cell
    glm::ivec3 offset = c->offset;
  
    glm::ivec3 indices[8];                                                                           // Corner information
    indices[0] = glm::ivec3(index.x,            index.y,            index.z);            // c000
    indices[1] = glm::ivec3(index.x,            index.y,            index.z + offset.z); // c001
    indices[2] = glm::ivec3(index.x,            index.y + offset.y, index.z);            // c010
    indices[3] = glm::ivec3(index.x,            index.y + offset.y, index.z + offset.z); // c011
    indices[4] = glm::ivec3(index.x + offset.x, index.y,            index.z);            // c100
    indices[5] = glm::ivec3(index.x + offset.x, index.y,            index.z + offset.z); // c101
    indices[6] = glm::ivec3(index.x + offset.x, index.y + offset.y, index.z);            // c110
    indices[7] = glm::ivec3(index.x + offset.x, index.y + offset.y, index.z + offset.z); // c111
                                                                                                    // Clamp the ends of the samples to avoid garbage
    for(int i = 0; i < 8; ++i)                                                                      // todo :: optimize check
    {
        if(indices[i].x == samples.x) indices[i].x -= 1;
        if(indices[i].y == samples.y) indices[i].y -= 1;
        if(indices[i].z == samples.z) indices[i].z -= 1;
    }

    for(int i = 0; i < 8; ++i)
        c->point_indices[i] = indices[i];
}

void octree_t::subdivideCell(cell_t* parent)
{
    unsigned int parent_level = parent->level;
    int this_level = parent_level + 1;

    glm::ivec3 offsets;

    offsets[0] = ((samples[0] - 1) / util::intPower(2, this_level));                             // change because octree starts from 0
    offsets[1] = ((samples[1] - 1) / util::intPower(2, this_level));
    offsets[2] = ((samples[2] - 1) / util::intPower(2, this_level));

    int parIndX = parent->index.x;
    int parIndY = parent->index.y;
    int parIndZ = parent->index.z;

    for(int i = 0; i < 8; ++i)
    {
        glm::ivec3 c000;

        switch(i)
        {
            case 0:
                c000.x = parIndX;
                c000.y = parIndY;
                c000.z = parIndZ;
            break;
            case 1:
                c000.x = parIndX;
                c000.y = parIndY;
                c000.z = parIndZ + offsets[2];
            break;
            case 2:
                c000.x = parIndX;
                c000.y = parIndY + offsets[1];
                c000.z = parIndZ;
            break;
            case 3:
                c000.x = parIndX;
                c000.y = parIndY + offsets[1];
                c000.z = parIndZ + offsets[2];
            break;
            case 4:
                c000.x = parIndX + offsets[0];
                c000.y = parIndY;
                c000.z = parIndZ;
            break;
            case 5:
                c000.x = parIndX + offsets[0];
                c000.y = parIndY;
                c000.z = parIndZ + offsets[2];
            break;
            case 6:
                c000.x = parIndX + offsets[0];
                c000.y = parIndY + offsets[1];
                c000.z = parIndZ;
            break;
            case 7:
                c000.x = parIndX + offsets[0];
                c000.y = parIndY + offsets[1];
                c000.z = parIndZ + offsets[2];
            break;
        }

        cell_t* c = new cell_t(cells.size(), BRANCH, parent, this_level, c000, offsets, i);     // Create new Cell on he heap
        cells.push_back(c);
        acquireCellInfo(c);
        parent->children[i] = c;
        
        if(this_level < m_minLvl)                                                               // If base octree level still not reached => subdivide
            subdivideCell(c);
        else if((this_level >= m_minLvl) && (this_level < m_maxLvl))                            // If the next level would be the min and max octree levels => check for subdiv
        {
            
            if(checkForSubdivision(c))                                                          // Check if the cell should be subdivided due to a complex surface or edge ambiguity
                subdivideCell(c);
            else
            {
                
                if(checkForSurface(c))                                                          // If not check whether there is any surface at all
                {
                    c->state = LEAF;
                    leafs.push_back(c);
                }
            }
        }
        else
        {
            if(checkForSurface(c))
            {
                c->state = LEAF;
                leafs.push_back(c);
            }
        }
        m_cellAddresses[c->address.getFormatted()] = c;                                         // Assigning cells to addresses :: todo  will this work here (recursive) better 
    }
}

bool octree_t::checkForSubdivision(cell_t* c)
{
    bool edgeAmbiguity = checkForEdgeAmbiguity(c);
    bool complexSurface = checkForComplexSurface(c);
  
    return edgeAmbiguity || complexSurface;                                                     // If either is true, then Subdivide the cell
}

bool octree_t::checkForSurface(cell_t* c)
{
    int inside = 0;                                                                             // Check if all the corners are inside then discard
    for(int i = 0; i < 8; ++i)
    {
        if(m_sampleData.getValueAt(c->point_indices[i]) < 0.0f)
            ++inside;
    }
                                                                        
    return (inside != 8) && (inside != 0);                                                      // See if cell is inside the function
}

bool octree_t::checkForEdgeAmbiguity(cell_t* c)
{
    bool edgeAmbiguity = false;                                                                 // Initialise return value
  
    for(int i = 0; i < 12; ++i)                                                                 // Loop through all the edges of the cell
    {
        int cellPtA = EDGE_VERTICES[i][0];                                                      // Getting the start and end cell points of this edge
        int cellPtB = EDGE_VERTICES[i][1];
        glm::ivec3 ptA = c->point_indices[cellPtA];                                                          // Getting the start and end sample indices of this edge
        glm::ivec3 ptB = c->point_indices[cellPtB];
        int lastIndex = m_sampleData.getIndexAt(ptB);
        glm::ivec3 prevIndex = ptA;                                                                // Setting the initial index to the start point index
        int crossingPoints = 0;                                                                 // Resetting the crossing point of this edge to zero
        uint8_t edgeDirection = EDGE_DIRECTION[i];                                              // Get the edge direction from the static table
        glm::ivec3 index = ptA;

        while(index[edgeDirection] <= ptB[edgeDirection])
        {
            assert(m_sampleData.getIndexAt(index) <= lastIndex);

            if(m_sampleData.getValueAt(prevIndex) * m_sampleData.getValueAt(index) < 0.0f)
                ++crossingPoints;

            if(crossingPoints > 1)
                edgeAmbiguity = true;

            prevIndex = index;
            ++index[edgeDirection];
        }
    }

  
    return edgeAmbiguity;                                                                       // Return result of check for two crossing points on any edge in this cell
}

bool octree_t::checkForComplexSurface(cell_t* c)
{
    bool complexSurface = false;                                                                // Initialise return value
  
    for(int i = 0; i < 7; ++i)                                                                  // Loop through all the cell points and check current point against all the rest remaining
    {
        glm::ivec3 indA = c->point_indices[i];
        glm::vec3 normalA;
        findGradient(normalA, indA);
        normalA = glm::normalize(normalA);

        for(int j = i + 1; j < 8; ++j)
        {
            glm::ivec3 indB = c->point_indices[j];
            glm::vec3 normalB;
            findGradient(normalB, indB);
            normalB = glm::normalize(normalB);//.normalize();

            if(glm::dot(normalA, normalB) < m_complexSurfThresh)
                complexSurface = true;
        }
    }
    return complexSurface;                                                                      // Return result of check for a comples surface in this cell
}

void octree_t::findGradient(glm::vec3& o_gradient, const glm::ivec3& i_array3dInds)
{
  
    glm::vec3 pos = m_sampleData.getPositionAt(i_array3dInds);                                       // Finding and storing the xyz position of the sample and it's local bbox
    glm::vec3 dimensions = 0.5f * offsets;
                                                                                                // Calculating the Forward Difference
    float dx = (*m_fn)(pos.x + dimensions.x, pos.y,                pos.z);
    float dy = (*m_fn)(pos.x,                pos.y + dimensions.y, pos.z);
    float dz = (*m_fn)(pos.x,                pos.y,                pos.z + dimensions.z);
    float val = m_sampleData.getValueAt(i_array3dInds);
    o_gradient = glm::vec3(dx - val, dy - val, dz - val);
}

void octree_t::findNeighbours(cell_t* cell)
{
    if(cell->id == 0) return;                                                                  // Dismiss the root as it doesn't have neighbours
    address_t tempAddress[6];                                                                   // Create an array of 6 addresses with a size of the max octree depth
    std::vector<uint8_t> tempNeighbourAddress[6];                                               // An array of the six neighbours' addresses, each having an address size equivelent to the maximum octree depth
  
    for(unsigned int i = 0; i < 6; ++i)                                                         // Fill with zeros up to the size of the addresses
        tempNeighbourAddress[i].resize(m_maxLvl);
  
    for(int i = 0; i < 6; ++i)                                                                  // Looping through possible neighbours
    {
        bool sameParent = false;
                                                                                                // Looping through every address space
        for(int slot = m_maxLvl - 1; slot >= 0; --slot)                                         // because [grandfather, father, child...]
        {
      
            if(sameParent)                                                                      // If the same parent has been detected, copy the rest of the address from cellA
                tempNeighbourAddress[i][slot] = cell->address.m_rawAddress[slot];
            else
            {
                uint8_t slotVal = cell->address.m_rawAddress[slot];                            // Get the value: For this cell (cellA), at depth (slot)
                int axis = i / 2;                                                               // For i (0..5) should result in: 0 0 1 1 2 2
                
                if(slotVal == 0)                                                                // Check against zero as the table does not support
                    tempNeighbourAddress[i][slot] = 0;
                else
                    tempNeighbourAddress[i][slot] = NEIGHBOUR_ADDRESS_TABLE[axis][slotVal - 1]; // Beware neigh slots start at 1 and not 0 thus the -1? !!!
                
                // if searching for right(+X), top(+Y) or front(+Z) neighbour it should always have a greater slot value
                // if searching for left(-X), bottom(-Y) or back(-Z) neighbour the neightbour should always have a smaller slot value,
                // OTHERWISE it means it belongs to a different parent
                //                 front(+Z) top(+Y) right(+X)                                  back(-Z) bottom(-Y) left(-X)
                if(((i & 1) && (slotVal < tempNeighbourAddress[i][slot])) || ((!(i & 1)) && (slotVal > tempNeighbourAddress[i][slot])))    
                    sameParent = true;                                                          // if it has the same parent then proceed and copy the remaining address slots from the current address as they will be the same
            }
        }
        tempAddress[i].m_rawAddress = tempNeighbourAddress[i];                                  // Populate actual address
    }
  
    for(int i = 0; i < 6; ++i)                                                                  // Actually find and assign the neighbour if such exists at the given address
    {
        unsigned int addressKey = tempAddress[i].getFormatted();
        cell_t* neighbour_cell = m_cellAddresses[addressKey];
        
        if(neighbour_cell)                                                                      // Proceed if there is such a neighbouring cell
        {
            CONTACT contact = (CONTACT)i;
      
            if(i & 1)                                                                           // todo :: Temporary save the neighbours addresses in the order:
                cell->neighbours[contact - 1] = neighbour_cell;
            else
                cell->neighbours[contact + 1] = neighbour_cell;
      
            setFaceTwins(neighbour_cell, cell, contact);                                        // Set face twins of the neighbouring cells based on their contact face
        }
    }

    // todo :: Consider duplicates if all the cells are in a loop ? Propagating?
    // have a bitfield to indicate which neighbours are already set?
}

void octree_t::populateHalfFaces()
{
    for(cell_t* cell : cells) findNeighbours(cell);                                             // todo :: optimise because this will set some neighbours twice
}

void octree_t::setFaceTwins(cell_t* a, cell_t* b, CONTACT contact)
{
    int valA = faceTwinTable[contact][0];                                                       // Assigning each face's twin based on the contact type
    int valB = faceTwinTable[contact][1];
    b->faces[valA]->twin = a->faces[valB];
    a->faces[valB]->twin = b->faces[valA];
    assert(b->faces[contact]->id == b->faces[contact]->twin->twin->id);
}

void octree_t::setFaceRelationships()
{
    for(cell_t* cell : cells)                                                                   // Loop through all the cells of the octree and assign the face relationship b/n parent and child cells
    {
        if(cell == 0 || cell == root) continue;                                                 // Continue if cell is null

        int location = cell->position_in_parent;

        for(int side = 0; side < 3; ++side)
        {
            CONTACT con = FACE_RELATIONSHIP_TABLE[location][side];
            uint8_t posOfSubFace = SUB_FACE_TABLE[location][side];

            cell->faces[con]->parent = cell->parent->faces[con];

            cell->parent->faces[con]->children[posOfSubFace] = cell->faces[con];
        }
    
        if(cell->state == LEAF)                                                                 // If this is a leaf cell then set all its half-faces as LEAFs
        {
            for(int i = 0; i < 6; ++i)
                cell->faces[i]->state = LEAF_FACE;
        }
    }
}

void octree_t::markTransitionalFaces()
{
    int transCounter = 0;
  
    for(unsigned int i = 0; i < leafs.size(); ++i)                                              // Loop through all leaf (straddling) cells
    {
        assert(leafs[i]->state == LEAF);

        for(int j = 0; j < 6; ++j)                                                              // Loop through all faces of such a cell
        {
            face_t* face = leafs[i]->faces[j];
            assert(face->state == LEAF_FACE);
      
            if((face->twin) && (face->twin->children[0]))                                             // Check against null ptr
            {
                assert(face->twin->children[1]);
                assert(face->twin->children[2]);
                assert(face->twin->children[3]);

                leafs[i]->faces[j]->state = TRANSIT_FACE;
                assert(leafs[i]->faces[j]->twin->state != LEAF_FACE);
                ++transCounter;
            }
        }
    }
}

} //namespace cms