#ifndef _cms_octree_included_90137583560738560378465037563452634578264596850164
#define _cms_octree_included_90137583560738560378465037563452634578264596850164

#include <map>

#include "cell.hpp"
#include "array3d.hpp"
#include "isosurface.hpp"

namespace cms
{

// Stores the possible types of cell-cell intersection
// the name is the face of the second cell which is in contact (-z,+z,-y,+y,-x,+x)

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

// The table assigning the twin faces of each face of a cell

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

// Octree :: creating and handling the octree data structure

struct Octree
{
    // Octree class type-definitions
    typedef std::vector<Cell*> CellVec;
    typedef std::vector<Face*> FaceVec;
    typedef Array3D<float> A3DFloat;

    Cell* m_root;
    std::vector<Cell*> m_cells;
    std::vector<Cell*> m_leafCells;
    std::map<unsigned int, Cell*> m_cellAddresses;
    std::vector<Face*> m_faces;
    Index3D& m_samples;
    A3DFloat& m_sampleData;
    unsigned int& m_minLvl;
    unsigned int& m_maxLvl;
    Vec3& m_offsets;
    Isosurface* m_fn;
    float& m_complexSurfThresh;


    Octree(Index3D& samples,
           A3DFloat& sampleData,
           unsigned int& minLvl,
           unsigned int& maxLvl,
           Vec3& offsets,
           Isosurface* fn,
           float& complexSurfThresh);

    ~Octree();


    // The function that is called for creating the Octree from the samples
    // It creates the root cell and calls subdivideCell function on it, recursively creating the octree
    void buildOctree();

    // Returns a pointer to the root cell of the octree
    Cell*       getRoot();
    
    CellVec     getAllCells() const;
    
    Cell*       getCellAt( int _i ) const;
    
    //---- Basic Octree Generation Functions ----//
    
    // Creating the root and then starting off the tree by calling
    // the recursive functions which build it - thus forming the structure
    void makeStructure();
    
    // A recursive function which checks if the current cell needs
    // subdividing and recursively doing so if necessary called by buildOctree()
    void subdivideCell( Cell* i_parent );
    
    // Obtains aditional cell info, which is stored in the cell,
    // i.e.: the sample point indices at the 8 corners, and it's dimensions
    void acquireCellInfo(Cell* c);
    
    // Check if there is a chance for a surface in that cell
    bool checkForSurface(Cell* c);
    
    // the fucntion which perfoms the checks for subdivision
    bool checkForSubdivision(Cell* c);
    
    // the function which checks for more than one sign change on an edge
    bool checkForEdgeAmbiguity(Cell* c);
    
    // checks for a complex surface based on the complexSurfaceThreshold
    bool checkForComplexSurface(Cell* c);
    
    // finds the gradients at any position in space using the forword difference
    void findGradient(Vec3& o_gradient, const Index3D& i_array3dInds);
    
    
    //---- Half-Face Assignment Functions ----//
    
    void findNeighbours(Cell* c);
    
    // The function that loops through all the cells and for each neighbouring cells sets 
    // their twin values on their faces thus making the half-face structure
    void populateHalfFaces();
    
    // Setting the half-face structure of Cell A and Cell B
    // where the contact variable names the face of Cell B into contact!
    void setFaceTwins(Cell* a, Cell* b, CONTACT);
    
    
    //---- Parent-Children Face Relationship Functions ----//
    
    // Loops through all the cells and establishes face relationships between parent and child cells
    void setFaceRelationships();
    
    // Find the transitional faces
    void markTransitionalFaces();
    
};

} // namespace cms

#endif // _cms_octree_included_90137583560738560378465037563452634578264596850164
