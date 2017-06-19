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
    Index3D c000 = Index3D(0, 0, 0);                                            // Establish Root
    Index3D offsets = m_samples - 1;
    root = new cell_t(0, BRANCH, 0, 0, c000, offsets, 0);
    cells.push_back(root);                                                      // Pushing the root as the first element of the cell array
    acquireCellInfo(root);                                                      // Calculating and storing information about the root cell
    subdivideCell(root);                                                        // Create the rest of the base grid recursively
}

void octree_t::acquireCellInfo(cell_t* c)
{
  
    Index3D index = c->index;                                                                       // Extracting values from cell
    Index3D offset = c->offset;

  
    Index3D ptIndices[8];                                                                           // Corner information
    ptIndices[0] = Index3D(index.m_x,              index.m_y,              index.m_z);              // c000
    ptIndices[1] = Index3D(index.m_x,              index.m_y,              index.m_z + offset.m_z); // c001
    ptIndices[2] = Index3D(index.m_x,              index.m_y + offset.m_y, index.m_z);              // c010
    ptIndices[3] = Index3D(index.m_x,              index.m_y + offset.m_y, index.m_z + offset.m_z); // c011
    ptIndices[4] = Index3D(index.m_x + offset.m_x, index.m_y,              index.m_z);              // c100
    ptIndices[5] = Index3D(index.m_x + offset.m_x, index.m_y,              index.m_z + offset.m_z); // c101
    ptIndices[6] = Index3D(index.m_x + offset.m_x, index.m_y + offset.m_y, index.m_z);              // c110
    ptIndices[7] = Index3D(index.m_x + offset.m_x, index.m_y + offset.m_y, index.m_z + offset.m_z); // c111

                                                                                                    // Clamp the ends of the samples to avoid garbage
    for(int i = 0; i < 8; ++i)                                                                      // todo :: optimize check
    {
        if(ptIndices[i].m_x == m_samples.m_x)
            ptIndices[i].m_x -= 1;
        if(ptIndices[i].m_y == m_samples.m_y)
            ptIndices[i].m_y -= 1;
        if(ptIndices[i].m_z == m_samples.m_z)
            ptIndices[i].m_z -= 1;
    }

    c->setPointInds(ptIndices);

    // the info below is only used when exporting the octree to a script and those calculations should really be done only then, 
    // no need to store all that data otherwise. Setting the exact positions of the corners in 3D space
    Range rangeX;
    rangeX.m_lower = m_sampleData.getPositionAt(ptIndices[0]).m_x;
    rangeX.m_upper = m_sampleData.getPositionAt(ptIndices[7]).m_x;
    c->m_x = rangeX;
    
    Range rangeY;
    rangeY.m_lower = m_sampleData.getPositionAt(ptIndices[0]).m_y;
    rangeY.m_upper = m_sampleData.getPositionAt(ptIndices[7]).m_y;
    c->m_y = rangeY;
    
    Range rangeZ;
    rangeZ.m_lower = m_sampleData.getPositionAt(ptIndices[0]).m_z;
    rangeZ.m_upper = m_sampleData.getPositionAt(ptIndices[7]).m_z;
    c->m_z = rangeZ;
}

void octree_t::subdivideCell(cell_t* parent)
{
    unsigned int parent_level = parent->level;
    int this_level = parent_level + 1;

    Index3D offsets;

    offsets[0] = ((m_samples[0] - 1) / util::intPower(2, this_level));                             // change because octree starts from 0
    offsets[1] = ((m_samples[1] - 1) / util::intPower(2, this_level));
    offsets[2] = ((m_samples[2] - 1) / util::intPower(2, this_level));

    int parIndX = parent->index.m_x;
    int parIndY = parent->index.m_y;
    int parIndZ = parent->index.m_z;

    for(int i = 0; i < 8; ++i)
    {
        Index3D c000;

        switch(i)
        {
            case 0:
                c000.m_x = parIndX;
                c000.m_y = parIndY;
                c000.m_z = parIndZ;
            break;
            case 1:
                c000.m_x = parIndX;
                c000.m_y = parIndY;
                c000.m_z = parIndZ + offsets[2];
            break;
            case 2:
                c000.m_x = parIndX;
                c000.m_y = parIndY + offsets[1];
                c000.m_z = parIndZ;
            break;
            case 3:
                c000.m_x = parIndX;
                c000.m_y = parIndY + offsets[1];
                c000.m_z = parIndZ + offsets[2];
            break;
            case 4:
                c000.m_x = parIndX + offsets[0];
                c000.m_y = parIndY;
                c000.m_z = parIndZ;
            break;
            case 5:
                c000.m_x = parIndX + offsets[0];
                c000.m_y = parIndY;
                c000.m_z = parIndZ + offsets[2];
            break;
            case 6:
                c000.m_x = parIndX + offsets[0];
                c000.m_y = parIndY + offsets[1];
                c000.m_z = parIndZ;
            break;
            case 7:
                c000.m_x = parIndX + offsets[0];
                c000.m_y = parIndY + offsets[1];
                c000.m_z = parIndZ + offsets[2];
            break;
        }

        assert(m_sampleData.getIndexAt(c000) < m_sampleData.size());

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
        m_cellAddresses[c->m_address.getFormatted()] = c;                                       // Assigning cells to addresses :: todo  will this work here (recursive) better 
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
    const Index3D *p = c->getPointInds();                                                       // Get a pointer to the index of the c000 corner of this point
  
    int inside = 0;                                                                             // Check if all the corners are inside then discard
    for(int i = 0; i < 8; ++i)
    {
        if(m_sampleData.getValueAt(*(p + i)) < 0.0f)
            ++inside;
    }
                                                                        
    return (inside != 8) && (inside != 0);                                                      // See if cell is inside the function
}

bool octree_t::checkForEdgeAmbiguity(cell_t* c)
{
    bool edgeAmbiguity = false;                                                                 // Initialise return value
    const Index3D *indPtr = c->getPointInds();                                                  // Getting the index of the c000 point of the current cell
  
    for(int i = 0; i < 12; ++i)                                                                 // Loop through all the edges of the cell
    {
        int cellPtA = EDGE_VERTICES[i][0];                                                      // Getting the start and end cell points of this edge
        int cellPtB = EDGE_VERTICES[i][1];
        Index3D ptA = indPtr[cellPtA];                                                          // Getting the start and end sample indices of this edge
        Index3D ptB = indPtr[cellPtB];
        int lastIndex = m_sampleData.getIndexAt(ptB);
        Index3D prevIndex = ptA;                                                                // Setting the initial index to the start point index
        int crossingPoints = 0;                                                                 // Resetting the crossing point of this edge to zero
        uint8_t edgeDirection = EDGE_DIRECTION[i];                                              // Get the edge direction from the static table
        Index3D index = ptA;

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
  
    const Index3D* p = c->getPointInds();                                                       // Get a pointer to the index of the c000 corner of this point
  
    for(int i = 0; i < 7; ++i)                                                                  // Loop through all the cell points and check current point against all the rest remaining
    {
        Index3D indA = *(p + i);
        Vec3 normalA;
        findGradient(normalA, indA);
        normalA.normalize();

        for(int j = i + 1; j < 8; ++j)
        {
            Index3D indB = *(p + j);
            Vec3 normalB;
            findGradient(normalB, indB);
            normalB.normalize();

            if(normalA.dot(normalB) < m_complexSurfThresh)
                complexSurface = true;
        }
    }
    return complexSurface;                                                                      // Return result of check for a comples surface in this cell
}

void octree_t::findGradient(Vec3& o_gradient, const Index3D& i_array3dInds)
{
  
    Vec3 pos = m_sampleData.getPositionAt(i_array3dInds);                                       // Finding and storing the xyz position of the sample and it's local bbox
    Vec3 dimensions;
    for(int i = 0; i < 3; ++i)
        dimensions[i] = 0.5f * m_offsets[i];
                                                                                                // Calculating the Forward Difference
    float dx = (*m_fn)(pos.m_x + dimensions.m_x, pos.m_y,                  pos.m_z);
    float dy = (*m_fn)(pos.m_x,                  pos.m_y + dimensions.m_y, pos.m_z);
    float dz = (*m_fn)(pos.m_x,                  pos.m_y,                  pos.m_z + dimensions.m_z);
    float val = m_sampleData.getValueAt(i_array3dInds);
    o_gradient = Vec3(dx - val, dy - val, dz - val);
}

void octree_t::findNeighbours(cell_t* cellA)
{
    if(cellA->id == 0) return;                                                                  // Dismiss the root as it doesn't have neighbours
    Address tempAddress[6];                                                                     // Create an array of 6 addresses with a size of the max octree depth
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
                tempNeighbourAddress[i][slot] = cellA->m_address.getRaw()[slot];
            else
            {
                uint8_t slotVal = cellA->m_address.getRaw()[slot];                              // Get the value: For this cell (cellA), at depth (slot)
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
        tempAddress[i].populateAddress(tempNeighbourAddress[i]);                                // Populate actual address
    }
  
    for(int i = 0; i < 6; ++i)                                                                  // Actually find and assign the neighbour if such exists at the given address
    {
        unsigned int addressKey = tempAddress[i].getFormatted();
        cell_t* cellB = m_cellAddresses[addressKey];
        
        if(cellB)                                                                               // Proceed if there is such a neighbouring cell
        {
            CONTACT contact = (CONTACT)i;
      
            if(i & 1)                                                                           // todo :: Temporary save the neighbours addresses in the order:
                cellA->neighbours[contact - 1] = cellB;
            else
                cellA->neighbours[contact + 1] = cellB;
      
            setFaceTwins(cellB, cellA, contact);                                                // Set face twins of the neighbouring cells based on their contact face
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

        int location = cell->getPosInParent();

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