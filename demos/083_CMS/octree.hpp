#ifndef _cms_octree_included_90137583560738560378465037563452634578264596850164
#define _cms_octree_included_90137583560738560378465037563452634578264596850164

#include <map>

#include "cell.hpp"
#include "array3d.hpp"
#include "tables.hpp"
#include "log.hpp"

namespace cms
{

//=======================================================================================================================================================================================================================
// possible types of cell-cell intersection, the name is the face of the second cell which is in contact (-z, +z, -y, +y, -x, +x)
//=======================================================================================================================================================================================================================
enum
{
    BACK    = 0,
    FRONT   = 1,
    BOTTOM  = 2,
    TOP     = 3,
    LEFT    = 4,
    RIGHT   = 5,
    NONE    = 6
};

//=======================================================================================================================================================================================================================
// the table assigning the twin faces of each face of a cell
//=======================================================================================================================================================================================================================
const int8_t faceTwinTable[7][2] =
{
    {BACK  , FRONT },
    {FRONT , BACK  },
    {BOTTOM, TOP   },
    {TOP   , BOTTOM},
    {LEFT  , RIGHT },
    {RIGHT , LEFT  },
    {NONE  , NONE  }
};

//=======================================================================================================================================================================================================================
// face relationship table :: given the position of a cell within its parent returns the 3 faces of that cell that touch the parent
//=======================================================================================================================================================================================================================
const int8_t FACE_RELATIONSHIP_TABLE[8][3] = 
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

const uint8_t SUB_FACE_TABLE[8][3] = 
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
const uint8_t EDGE_DIRECTION[12] = {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2};

//=======================================================================================================================================================================================================================
// cell neighbour table[axis][cell ID] within parent, neighbour slots start at 1 and not 0!
//=======================================================================================================================================================================================================================
const uint8_t NEIGHBOUR_ADDRESS_TABLE[3][8] =
{
  // 0  1  2  3  4  5  6  7   Cell IDs
    {2, 1, 4, 3, 6, 5, 8, 7}, // Z (BACK & FRONT)
    {3, 4, 1, 2, 7, 8, 5, 6}, // Y (TOP & BOTTOM)
    {5, 6, 7, 8, 1, 2, 3, 4}  // X (LEFT & RIGHT)
};

//=======================================================================================================================================================================================================================
// octree :: creating and handling the octree data structure
//=======================================================================================================================================================================================================================
template<typename scalar_field_t> struct octree_t
{
    int depth;

    scalar_field_t scalar_field;
    cell_t* root;
    std::vector<cell_t*> cells;
    std::vector<cell_t*> leafs;
    std::vector<face_t*> faces;

    std::map<unsigned int, cell_t*> cell_hash_map;

    Array3D<float>& field_values;
    unsigned int min_level;
    unsigned int max_level;
    float delta;
    float complex_surface_threshold;

    octree_t(int depth, Array3D<float>& field_values, unsigned int min_level, unsigned int max_level, 
             float delta, float complex_surface_threshold) 
        : depth(depth), field_values(field_values), min_level(min_level), max_level(max_level),
          delta(delta), complex_surface_threshold(complex_surface_threshold)
    {}

    ~octree_t()
    {
        for(unsigned int i = 0; i < cells.size(); ++i)                              // deleting cells
            if(cells[i]) 
                delete cells[i];
    }

    //===================================================================================================================================================================================================================
    // main function :: creates the root cell and calls subdivide function on it, recursively creating the octree
    //===================================================================================================================================================================================================================
    cell_t* build()
    {
        root = new cell_t(0, 0, glm::ivec3(0), depth, 0);                           // establish root
        cells.push_back(root);                                                      // Pushing the root as the first element of the cell array
        subdivide(root);                                                            // Create the rest of the base grid recursively
        populateHalfFaces();                                                        // create the half-face structure for all cells
        setFaceRelationships();                                                     // create the Face parent-children relationships
        markTransitionalFaces();                                                    // flag all transitional faces
        return root;
    }
    
    //===================================================================================================================================================================================================================
    // checks if the current cell needs subdivision and recursively does so if necessary
    //===================================================================================================================================================================================================================
    void subdivide(cell_t* parent)
    {
        unsigned int level = parent->level + 1;

        int offset = depth >> level;
        int iX = parent->index.x;
        int iY = parent->index.y;
        int iZ = parent->index.z;

        glm::ivec3 cell_corners[] =
        {
            glm::ivec3(iX,          iY,          iZ),
            glm::ivec3(iX,          iY,          iZ + offset),
            glm::ivec3(iX,          iY + offset, iZ),
            glm::ivec3(iX,          iY + offset, iZ + offset),
            glm::ivec3(iX + offset, iY,          iZ),
            glm::ivec3(iX + offset, iY,          iZ + offset),
            glm::ivec3(iX + offset, iY + offset, iZ),
            glm::ivec3(iX + offset, iY + offset, iZ + offset)
        };

        for(int i = 0; i < 8; ++i)
        {
            cell_t* cell = new cell_t(parent, level, cell_corners[i], offset, i);     // Create new Cell on he heap
            cells.push_back(cell);
            parent->children[i] = cell;
        
            if(level < min_level)                                                               // If base octree level still not reached => subdivide
                subdivide(cell);
            else if(level < max_level)                                                         // If the next level would be the min and max octree levels => check for subdiv
            {
                if(checkForEdgeAmbiguity(cell) || checkForComplexSurface(cell))                                                          // Check if the cell should be subdivided due to a complex surface or edge ambiguity
                    subdivide(cell);
                else
                {
                    if(checkForSurface(cell))                                                          // If not check whether there is any surface at all
                    {
                        cell->leaf = true;
                        leafs.push_back(cell);
                    }
                }
            }
            else
            {
                if(checkForSurface(cell))
                {
                    cell->leaf = true;
                    leafs.push_back(cell);
                }
            }
            cell_hash_map[cell->address.hash()] = cell;                                         // Assigning cells to addresses :: todo  will this work here (recursive) better 
        }
    }

    //===================================================================================================================================================================================================================
    // checks if there is a chance for a surface in that cell
    //===================================================================================================================================================================================================================    
    bool checkForSurface(cell_t* cell)
    {
        int inside = 0;                                                             // Check if all the corners are inside then discard
        for(int i = 0; i < 8; ++i)
            if(field_values[cell->corners[i]] < 0.0f)
                ++inside;
                                                                        
        return (inside != 8) && (inside != 0);                                      // See if cell is inside the function
    }

    //===================================================================================================================================================================================================================
    // checks for more than one sign change on the edge
    //===================================================================================================================================================================================================================
    bool checkForEdgeAmbiguity(cell_t* cell) const
    {
        for(int i = 0; i < 12; ++i)                                                                 // Loop through all the edges of the cell
        {
            int cellPtA = EDGE_VERTICES[i][0];                                                      // Getting the start and end cell points of this edge
            int cellPtB = EDGE_VERTICES[i][1];
            glm::ivec3 ptA = cell->corners[cellPtA];                                          // Getting the start and end sample indices of this edge
            glm::ivec3 ptB = cell->corners[cellPtB];
            int lastIndex = field_values.linear_index(ptB);
            glm::ivec3 prevIndex = ptA;                                                             // Setting the initial index to the start point index
            int crossingPoints = 0;                                                                 // Resetting the crossing point of this edge to zero
            uint8_t edgeDirection = EDGE_DIRECTION[i];                                              // Get the edge direction from the static table
            glm::ivec3 index = ptA;

            while(index[edgeDirection] <= ptB[edgeDirection])
            {
                if(field_values[prevIndex] * field_values[index] < 0.0f)
                    ++crossingPoints;

                if(crossingPoints > 1) return true;

                prevIndex = index;
                ++index[edgeDirection];
            }
        }
        return false;                                                                       // Return result of check for two crossing points on any edge in this cell
    }

    //===================================================================================================================================================================================================================
    // checks for complex surface based on the complexSurfaceThreshold
    //===================================================================================================================================================================================================================
    bool checkForComplexSurface(cell_t* cell) const
    {
        for(int i = 0; i < 7; ++i)                                                                  // Loop through all the cell points and check current point against all the rest remaining
        {
            glm::ivec3 indexA = cell->corners[i];
            glm::vec3 normalA = glm::normalize(gradient(indexA));

            for(int j = i + 1; j < 8; ++j)
            {
                glm::ivec3 indexB = cell->corners[j];
                glm::vec3 normalB = glm::normalize(gradient(indexB));

                if(glm::dot(normalA, normalB) < complex_surface_threshold)
                    return true;
            }
        }
        return false;                                                                           
    }

    //===================================================================================================================================================================================================================
    // finds the gradient at any position in space using forward finite difference
    //===================================================================================================================================================================================================================
    glm::vec3 gradient(const glm::ivec3& index) const
    {
        glm::vec3 position = glm::vec3(-1.0f) + delta * glm::vec3(index);                                       // Finding and storing the xyz position of the sample and it's local bbox
        float gradient_delta = 0.5f * delta;
                                                                                                // Calculating the Forward Difference
        float dx = scalar_field(glm::vec3(position.x + gradient_delta, position.y, position.z));
        float dy = scalar_field(glm::vec3(position.x, position.y + gradient_delta, position.z));
        float dz = scalar_field(glm::vec3(position.x, position.y, position.z + gradient_delta));

        float val = field_values[index];
        return glm::vec3(dx - val, dy - val, dz - val);
    }
    
    //===================================================================================================================================================================================================================
    // half-face assignment functions
    //===================================================================================================================================================================================================================
    void findNeighbours(cell_t* cell)
    {
        address_t tempAddress[6];                                                                   // Create an array of 6 addresses with a size of the max octree depth
        std::vector<uint8_t> tempNeighbourAddress[6];                                               // An array of the six neighbours' addresses, each having an address size equivelent to the maximum octree depth
        
        for(unsigned int i = 0; i < 6; ++i)                                                         // Fill with zeros up to the size of the addresses
            tempNeighbourAddress[i].resize(max_level);
        
        for(int i = 0; i < 6; ++i)                                                                  // Looping through possible neighbours
        {
            bool sameParent = false;
            for(int slot = max_level - 1; slot >= 0; --slot)                                         // Looping through every address space because [grandfather, father, child...]
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
                    
                    // if searching for +X, +Y or +Z neighbour it should always have a greater slot value
                    // if searching for -X, -Y or -Z neighbour it should always have a smaller slot value,
                    // otherwise it means it belongs to a different parent
                    if(((i & 1) && (slotVal < tempNeighbourAddress[i][slot])) || ((!(i & 1)) && (slotVal > tempNeighbourAddress[i][slot])))    
                        sameParent = true;                                                          // if it has the same parent then proceed and copy the remaining address slots from the current address as they will be the same
                }
            }
            tempAddress[i].m_rawAddress = tempNeighbourAddress[i];                                  // Populate actual address
        }
        
        for(int i = 0; i < 6; ++i)                                                                  // Actually find and assign the neighbour if such exists at the given address
        {
            unsigned int hash = tempAddress[i].hash();
            cell_t* neighbour_cell = cell_hash_map[hash];
            
            if(neighbour_cell)                                                                      // Proceed if there is such a neighbouring cell
            {
                if(i & 1)                                                                           // todo :: Temporary save the neighbours addresses in the order:
                    cell->neighbours[i - 1] = neighbour_cell;
                else
                    cell->neighbours[i + 1] = neighbour_cell;

                int valA = faceTwinTable[i][0];                                               // assigning each face's twin based on the contact type
                int valB = faceTwinTable[i][1];
                cell->faces[valA]->twin = neighbour_cell->faces[valB];
                neighbour_cell->faces[valB]->twin = cell->faces[valA];

            }
        }
    }

    //===================================================================================================================================================================================================================
    // Parent-Children face relationship function : loops through all the cells and establishes face relationships between parent and child cells
    //===================================================================================================================================================================================================================
    void setFaceRelationships()
    {
        for(cell_t* cell : cells)
        {
            if(cell == 0 || cell == root) continue;                                         // continue if the cell is null

            int location = cell->position_in_parent;

            for(int side = 0; side < 3; ++side)
            {
                int8_t c = FACE_RELATIONSHIP_TABLE[location][side];
                uint8_t sub_face = SUB_FACE_TABLE[location][side];
                cell->faces[c]->parent = cell->parent->faces[c];
                cell->parent->faces[c]->children[sub_face] = cell->faces[c];
            }
    
            if(cell->leaf)                                                         // if this is a leaf cell then set all its half-faces as LEAFs
            {
                for(int i = 0; i < 6; ++i)
                    cell->faces[i]->state = LEAF_FACE;
            }
        }
    }

    //===================================================================================================================================================================================================================
    // finds and marks transitional faces
    //===================================================================================================================================================================================================================
    void markTransitionalFaces()
    {
        int transCounter = 0;
  
        for(unsigned int i = 0; i < leafs.size(); ++i)                                              // Loop through all leaf (straddling) cells
        {
            for(int j = 0; j < 6; ++j)                                                              // Loop through all faces of such a cell
            {
                face_t* face = leafs[i]->faces[j];
      
                if((face->twin) && (face->twin->children[0]))                                             // Check against null ptr
                {
                    leafs[i]->faces[j]->state = TRANSIT_FACE;
                    ++transCounter;
                }
            }
        }
    }

    //===================================================================================================================================================================================================================
    // loops through all the cells and for each neighbouring cells sets their twin values on their faces thus making the half-face structure
    //===================================================================================================================================================================================================================
    void populateHalfFaces()                                                       
        { for(cell_t* cell : cells) findNeighbours(cell); }                                 // todo :: optimise because this will set some neighbours twice

};

} // namespace cms

#endif // _cms_octree_included_90137583560738560378465037563452634578264596850164