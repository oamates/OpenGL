#include <cmath>
#include <cstdlib>
#include <fstream>

#include "octree.hpp"
#include "util.hpp"
#include "tables.hpp"

namespace cms
{

// A table setting the face relationship
// Given the position of a cell within its parent
// it returns the 3 faces of that cell that touch
// the parent. (they would be the same for the parent)

static const CONTACT FACE_RELATIONSHIP_TABLE[8][3] = 
{
    {BACK , BOTTOM, LEFT },
    {FRONT, BOTTOM, LEFT },
    {BACK , TOP   , LEFT },
    {FRONT, TOP   ,   LEFT },
    {BACK , BOTTOM, RIGHT},
    {FRONT, BOTTOM, RIGHT},
    {BACK , TOP   ,   RIGHT},
    {FRONT, TOP   ,   RIGHT}
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


// Edge direction table
// Given one of the 12 edges of a cube it returns
// the direction of the edge (0=x, 1=y, 2=z)

static const uint8_t EDGE_DIRECTION[12] =
{
   /* 0  1  2  3  4  5  6  7  8  9  10 11  Index */
      0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2
};


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



Octree::Octree(Index3D& samples,
               Array3D<float>& sampleData,
               unsigned int& minLvl,
               unsigned int& maxLvl,
               Vec3& offsets,
               Isosurface *fn,
               float& complexSurfThresh) :
    m_samples(samples),
    m_sampleData(sampleData),
    m_minLvl(minLvl),
    m_maxLvl(maxLvl),
    m_offsets(offsets),
    m_fn(fn),
    m_complexSurfThresh(complexSurfThresh)
{
}

Octree::~Octree()
{
    for(unsigned int i = 0; i < m_cells.size(); ++i)                        // Deleting cells
        if(m_cells[i]) 
            delete m_cells[i];
}

Cell* Octree::getRoot()
    { return m_root; }

std::vector<Cell*> Octree::getAllCells() const
    { return m_cells; }

Cell* Octree::getCellAt(int _i) const
    { return m_cells[_i]; }


void Octree::buildOctree()
{
    makeStructure();                                                            // Create the octree structure by establishing the root and recursing onwards
    populateHalfFaces();                                                        // Create the half-face structure for all cells
    setFaceRelationships();                                                     // Create the Face parent-children relationships
    markTransitionalFaces();                                                    // Flag all transitional faces
}



//=======================================================================================================================================================================================================================
// Basic Octree Generation Functions 
//=======================================================================================================================================================================================================================

void Octree::makeStructure()
{
  
    Index3D c000 = Index3D(0, 0, 0);                                            // Establish Root
    Index3D offsets = m_samples - 1;
    m_root = new Cell(0, BRANCH, nullptr, 0, c000, offsets, 0);

    m_cells.push_back(m_root);                                                  // Pushing the root as the first element of the cell array
    acquireCellInfo(m_root);                                                    // Calculating and storing information about the root cell
    subdivideCell(m_root);                                                      // Create the rest of the base grid recursively
}

void Octree::acquireCellInfo(Cell* c)
{
  
    Index3D c000 = c->getC000();                                                // Extracting values from cell
    Index3D offsets = c->getOffsets();

  
    Index3D ptIndices[8];                                                                           // Corner information
    ptIndices[0] = Index3D(c000.m_x,               c000.m_y,               c000.m_z);               // c000
    ptIndices[1] = Index3D(c000.m_x,               c000.m_y,               c000.m_z + offsets.m_z); // c001
    ptIndices[2] = Index3D(c000.m_x,               c000.m_y + offsets.m_y, c000.m_z);               // c010
    ptIndices[3] = Index3D(c000.m_x,               c000.m_y + offsets.m_y, c000.m_z + offsets.m_z); // c011
    ptIndices[4] = Index3D(c000.m_x + offsets.m_x, c000.m_y,               c000.m_z);               // c100
    ptIndices[5] = Index3D(c000.m_x + offsets.m_x, c000.m_y,               c000.m_z + offsets.m_z); // c101
    ptIndices[6] = Index3D(c000.m_x + offsets.m_x, c000.m_y + offsets.m_y, c000.m_z);               // c110
    ptIndices[7] = Index3D(c000.m_x + offsets.m_x, c000.m_y + offsets.m_y, c000.m_z + offsets.m_z); // c111

    // Clamp the ends of the samples to avoid garbage
    for(int i = 0; i < 8; ++i) ///@todo optimize check
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
    
    // Dimensions information
    float w = (rangeX.m_upper-rangeX.m_lower);
    c->setWidth(w);
    float h = (rangeY.m_upper-rangeY.m_lower);
    c->setHeight(h);
    float d = (rangeZ.m_upper-rangeZ.m_lower);
    c->setDepth(d);
    
    // Define centre of cell
    Vec3 c000Pos = m_sampleData.getPositionAt(c000);
    Vec3 centre = Vec3(c000Pos.m_x + 0.5f * w,
                       c000Pos.m_y + 0.5f * h,
                       c000Pos.m_z + 0.5f * d);
    c->setCentre(centre);
}



//------------------------------------------------------------



void Octree::subdivideCell(Cell *i_parent)
{
    unsigned int parLvl = i_parent->getSubdivLvl();
    int thisLvl = parLvl + 1;

    Index3D offsets;
    offsets[0] = ((m_samples[0] - 1) / util::intPower(2, thisLvl));                             // change because octree starts from 0
    offsets[1] = ((m_samples[1] - 1) / util::intPower(2, thisLvl));                             // change
    offsets[2] = ((m_samples[2] - 1) / util::intPower(2, thisLvl));                             // change

    int parIndX = i_parent->getC000().m_x;
    int parIndY = i_parent->getC000().m_y;
    int parIndZ = i_parent->getC000().m_z;

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

        Cell* c = new Cell(m_cells.size(), BRANCH, i_parent, thisLvl, c000, offsets, i);        // Create new Cell on he heap
        m_cells.push_back(c);
        acquireCellInfo(c);
        i_parent->pushChild(c, i);
        
        if((unsigned int)thisLvl < m_minLvl)                                                    // If base octree level still not reached => subdivide
            subdivideCell(c);
        else if(((unsigned int)thisLvl >= m_minLvl) && ((unsigned int)thisLvl < m_maxLvl))      // If the next level would be the min and max octree levels => check for subdiv
        {
            
            if(checkForSubdivision(c))                                                          // Check if the cell should be subdivided due to a complex surface or edge ambiguity
                subdivideCell(c);
            else
            {
                
                if(checkForSurface(c))                                                          // If not check whether there is any surface at all
                {
                    c->setState(LEAF);
                    m_leafCells.push_back(c);
                }
            }
        }
        else
        {
            if(checkForSurface(c))
            {
                c->setState(LEAF);
                m_leafCells.push_back(c);
            }
        }
        m_cellAddresses[c->m_address.getFormatted()] = c;                                       // Assigning cells to addresses :: todo  will this work here (recursive) better 
    }
}

bool Octree::checkForSubdivision(Cell* c)
{
    bool edgeAmbiguity = checkForEdgeAmbiguity(c);
    bool complexSurface = checkForComplexSurface(c);
  
    return (edgeAmbiguity || complexSurface);                                                   // If either is true, then Subdivide the cell
}

bool Octree::checkForSurface(Cell* c)
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

bool Octree::checkForEdgeAmbiguity(Cell* c)
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

bool Octree::checkForComplexSurface(Cell* c)
{
    bool complexSurface = false;                                                                // Initialise return value
  
    const Index3D *p = c->getPointInds();                                                       // Get a pointer to the index of the c000 corner of this point
  
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

void Octree::findGradient(Vec3& o_gradient, const Index3D& i_array3dInds)
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


//==================== Half-Face Assignment Functions ===============

void Octree::findNeighbours(Cell* cellA)
{
                                                                                                // Dismiss the root as he doesn't have neighbours
    if(cellA->m_id == 0) return;                                                                // todo: check outside (index start from 1?)
  
    Address tempAddress[6];                                                                     // Create an array of 6 addresses with a size of the max octree depth
    
    std::vector<uint8_t> tempNeighbourAddress[6];                                               // An array of the six neighbours' addresses, each having an address size equivelent to the maximum octree depth

  
    for(unsigned int i = 0; i < 6; ++i)                                                         // Fill with zeros up to the size of the addresses
        tempNeighbourAddress[i].resize(m_maxLvl);

  
    for(int i = 0; i < 6; ++i)                                                                  // Looping through possible neighbours
    {
        bool sameParent = false;

        // Looping through every address space
        for(int slot = m_maxLvl - 1; slot >= 0; --slot) /// because [grandfather, father, child...]
        {
      // If the same parent has been detected, copy the rest of the address from cellA
            if(sameParent)
                tempNeighbourAddress[i][slot] = cellA->m_address.getRaw()[slot];
            else
            {
                // Get the value:
                // For this cell (cellA)
                // At depth (slot)
                uint8_t slotVal = cellA->m_address.getRaw()[slot];
                
                // For i (0..5) should result in: 0 0 1 1 2 2
                int axis = i/2;
                
                // Check against zero as the table does not support
                if(slotVal == 0)
                {
                  tempNeighbourAddress[i][slot] = 0;
                }
                else
                {
                  // Beware neigh slots start at 1 and not 0 thus the -1? !!!
                  tempNeighbourAddress[i][slot] = NEIGHBOUR_ADDRESS_TABLE[axis][slotVal-1];
                }
                
                // if searching for right(+X), top(+Y) or front(+Z) neighbour
                // it should always have a greater slot value
                // if searching for left(-X), bottom(-Y) or back(-Z) neighbour
                // the neightbour should always have a smaller slot value,
                // OTHERWISE it means it belongs to a different parent
                if( ((i%2!=0) && (slotVal < tempNeighbourAddress[i][slot]))  ||  // front(+Z) top(+Y) right(+X)
                    ((i%2==0) && (slotVal > tempNeighbourAddress[i][slot])) )    // back(-Z) bottom(-Y) left(-X)
                {
                  // if it has the same parent then proceed and copy the remaining
                  // address slots from the current address as they will be the same
                  sameParent = true;
                }
            }
        }

        // Populate actual address
        tempAddress[i].populateAddress( tempNeighbourAddress[i] );
    }


  // Actually find and assign the neighbour if such exists at the given address
    for(int i = 0; i < 6; ++i)
    {
        unsigned int addressKey = tempAddress[i].getFormatted();

        Cell* cellB = m_cellAddresses[addressKey];

        // Proceed if there is such a neighbouring cell
        if(cellB)
        {
            CONTACT contact = (CONTACT)i;

      // TODO
      // Temporary save the neighbours addresses in the order:
            if(i % 2 == 0)
                cellA->m_neighbours[contact + 1] = cellB;
            else
                cellA->m_neighbours[contact - 1] = cellB;

      // Set face twins of the neighbouring cells based on their contact face
            setFaceTwins(cellB, cellA, contact);
        }
    }

    // todo
    // Consider duplicates if all the cells are in a loop ?
    // Propagating?
    // have a bitfield to indicate which neighbours are already set?
}


//-----------------------------------------------------------


void Octree::populateHalfFaces()
{
#if CMS_DEBUG_LOG
    debug_msg("Number of cells: %u" << (unsigned int) m_cells.size());
#endif
  
    for(Cell* c : m_cells) findNeighbours(c);                                                              // todo :: optimise because this will set some neighbours twice
}


void Octree::setFaceTwins(Cell* a, Cell* b, CONTACT contact)
{
  
    int valA = faceTwinTable[contact][0];                                               // Assigning each face's twin based on the contact type
    int valB = faceTwinTable[contact][1];

    b->getFaceAt(valA)->twin = a->getFaceAt(valB);
    a->getFaceAt(valB)->twin = b->getFaceAt(valA);


    assert(b->getFaceAt(contact)->id == b->getFaceAt(contact)->twin->twin->id);
}

void Octree::setFaceRelationships()
{
    for(Cell* cell : m_cells)                                                           // Loop through all the cells of the octree and assign the face relationship b/n parent and child cells
    {
        if(cell == nullptr || cell == m_root) continue;                                 // Continue if cell is null

        int location = cell->getPosInParent();

        for(int side = 0; side < 3; ++side)
        {
            CONTACT con = FACE_RELATIONSHIP_TABLE[location][side];
            uint8_t posOfSubFace = SUB_FACE_TABLE[location][side];

            cell->getFaceAt(con)->parent = cell->getParent()->getFaceAt(con);

            cell->getParent()->getFaceAt(con)->children[posOfSubFace] = cell->getFaceAt(con);
        }
    
        if(cell->getState() == LEAF)                                                    // If this is a leaf cell then set all its half-faces as LEAFs
        {
            for(int i = 0; i < 6; ++i)
                cell->getFaceAt(i)->state = LEAF_FACE;
        }
    }
}

void Octree::markTransitionalFaces()
{
    int transCounter = 0;
  
    for(unsigned int i = 0; i < m_leafCells.size(); ++i)                            // Loop through all leaf (straddling) cells
    {
        assert(m_leafCells[i]->getState() == LEAF);

        for(int j = 0;j<6;++j)    // Loop through all faces of such a cell
        {
            Face* f = m_leafCells[i]->getFaceAt(j);
            assert(f->state == LEAF_FACE);
      
            if((f->twin) && (f->twin->children[0]))   // Check against null ptr
            {
                assert(f->twin->children[1]);
                assert(f->twin->children[2]);
                assert(f->twin->children[3]);

                m_leafCells[i]->getFaceAt(j)->state = TRANSIT_FACE;
                assert(m_leafCells[i]->getFaceAt(j)->twin->state != LEAF_FACE);
                ++transCounter;
            }
        }
    }
}

} //namespace cms
