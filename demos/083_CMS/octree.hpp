#ifndef _cms_octree_included_90137583560738560378465037563452634578264596850164
#define _cms_octree_included_90137583560738560378465037563452634578264596850164

#include <map>

#include "cell.hpp"
#include "array3d.hpp"
#include "isosurface.hpp"

namespace cms
{

//=======================================================================================================================================================================================================================
// Stores the possible types of cell-cell intersection, the name is the face of the second cell which is in contact (-z, +z, -y, +y, -x, +x)
//=======================================================================================================================================================================================================================
enum CONTACT
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
static const int8_t faceTwinTable[7][2] =
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
// octree :: creating and handling the octree data structure
//=======================================================================================================================================================================================================================
struct Octree
{
    typedef std::vector<Cell*> CellVec;
    typedef std::vector<Face*> FaceVec;

    Cell* m_root;
    std::vector<Cell*> m_cells;
    std::vector<Cell*> m_leafCells;
    std::map<unsigned int, Cell*> m_cellAddresses;
    std::vector<Face*> m_faces;
    Index3D& m_samples;
    Array3D<float>& m_sampleData;
    unsigned int& m_minLvl;
    unsigned int& m_maxLvl;
    Vec3& m_offsets;
    Isosurface* m_fn;
    float& m_complexSurfThresh;

    Octree(Index3D& samples, Array3D<float>& sampleData, unsigned int& minLvl, unsigned int& maxLvl, 
           Vec3& offsets, Isosurface* fn, float& complexSurfThresh) 
        : m_samples(samples), m_sampleData(sampleData), m_minLvl(minLvl), m_maxLvl(maxLvl),
          m_offsets(offsets), m_fn(fn), m_complexSurfThresh(complexSurfThresh)
    {}

    ~Octree()
    {
        for(unsigned int i = 0; i < m_cells.size(); ++i)                            // Deleting cells
            if(m_cells[i]) 
                delete m_cells[i];
    }

    //===================================================================================================================================================================================================================
    // main function :: creates the root cell and calls subdivideCell function on it, recursively creating the octree
    //===================================================================================================================================================================================================================
    void buildOctree()
    {
        makeStructure();                                                            // create the octree structure by establishing the root and recursing onwards
        populateHalfFaces();                                                        // create the half-face structure for all cells
        setFaceRelationships();                                                     // create the Face parent-children relationships
        markTransitionalFaces();                                                    // flag all transitional faces
    }
    
    Cell* getRoot()                                                                 // Returns a pointer to the root cell of the octree
        { return m_root; }
    
    CellVec getAllCells() const
        { return m_cells; }
    
    Cell* getCellAt(int _i) const
        { return m_cells[_i]; }
    
    //===================================================================================================================================================================================================================
    // basic octree generation functions
    //===================================================================================================================================================================================================================
    
    void makeStructure();                                                           // Creating the root and then starting off the tree by calling the recursive functions which build it - thus forming the structure
    void subdivideCell(Cell* i_parent);                                             // A recursive function which checks if the current cell needs subdividing and recursively doing so if necessary called by buildOctree()
    void acquireCellInfo(Cell* c);                                                  // Obtains aditional cell info, which is stored in the cell, i.e.: the sample point indices at the 8 corners, and it's dimensions
    bool checkForSurface(Cell* c);                                                  // Check if there is a chance for a surface in that cell
    bool checkForSubdivision(Cell* c);                                              // the fucntion which perfoms the checks for subdivision
    bool checkForEdgeAmbiguity(Cell* c);                                            // the function which checks for more than one sign change on an edge
    bool checkForComplexSurface(Cell* c);                                           // checks for a complex surface based on the complexSurfaceThreshold
    void findGradient(Vec3& o_gradient, const Index3D& i_array3dInds);              // finds the gradients at any position in space using the forword difference
    
    //===================================================================================================================================================================================================================
    // half-face assignment functions
    //===================================================================================================================================================================================================================
    
    void findNeighbours(Cell* c);
    void populateHalfFaces();                                                       // The function that loops through all the cells and for each neighbouring cells sets their twin values on their faces thus making the half-face structure
    void setFaceTwins(Cell* a, Cell* b, CONTACT);                                   // Setting the half-face structure of Cell A and Cell B where the contact variable names the face of Cell B into contact!
    
    //===================================================================================================================================================================================================================
    // Parent-Children Face Relationship Functions
    //===================================================================================================================================================================================================================
    
    void setFaceRelationships();                                                    // Loops through all the cells and establishes face relationships between parent and child cells
    void markTransitionalFaces();                                                   // Find the transitional faces
};

} // namespace cms

#endif // _cms_octree_included_90137583560738560378465037563452634578264596850164
