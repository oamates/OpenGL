#ifndef CMS_OCTREE_H
#define CMS_OCTREE_H

#include <map>

#include "cell.hpp"
#include "array3d.hpp"
#include "isosurface.hpp"
#include "types.hpp"


namespace cms
{


/// @brief Stores the possible types of cell-cell intersection
/// the name is the face of the second cell which is in contact (-z,+z,-y,+y,-x,+x)
///
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



/// @brief The table assigning the twin faces of each face
/// of a cell
///
static const int8_t faceTwinTable[7][2] =
{
  { BACK  , FRONT  },
  { FRONT , BACK   },
  { BOTTOM, TOP    },
  { TOP   , BOTTOM },
  { LEFT  , RIGHT  },
  { RIGHT , LEFT   },
  { NONE  , NONE   }
};



/// @class Octree
/// @brief The octree class creating and hadling the octree datastructure
///
class Octree
{
  /// @brief Octree class type-definitions
  typedef std::vector<Cell*> CellVec;
  typedef std::vector<Face*> FaceVec;
  typedef Array3D<float> A3DFloat;

public:
  /// @brief Constructor
              Octree( Index3D&    samples           ,
                      A3DFloat&   sampleData        ,
                      uint&       minLvl            ,
                      uint&       maxLvl            ,
                      Vec3&       offsets           ,
                      Isosurface* fn                ,
                      float&      complexSurfThresh );

  /// @brief Destructor
              ~Octree();


  /// @brief The function that is called for creating the Octree from the samples
  /// It creates the root cell and calls subdivideCell function on it, recursively creating the octree
  void        buildOctree();


  /// @brief Returns a pointer to the root cell of the octree
  Cell*       getRoot();


  CellVec     getAllCells() const;


  Cell*       getCellAt( int _i ) const;


  /// @brief Exporting a Maya py script at the specified path
  /// contains Maya Python commands which create boxes representing oc-cells
  bool        exportToMaya( const std::string& i_fName   ,
                                  bool         justLeafs ,
                                  intVec       desired   ) const;


  /// @brief Exporting a blender py script at the specified path
  /// contains Blender Python commands which create cubes representing oc-cells
  /// @warning This script will work with blender >= 2.72 release because or 'radius' command
  bool        exportToBlender(const std::string& i_fName   ,
                                    bool         justLeafs ,
                                    intVec       desired   ) const;


private:

  ///---- Basic Octree Generation Functions ----///

  /// @brief Creating the root and then starting off the tree by calling
  /// the recursive functions which build it - thus forming the structure
  void        makeStructure();


  /// @brief A recursive function which checks if the current cell needs
  /// subdividing and recursively doing so if necessary called by buildOctree()
  void        subdivideCell( Cell* i_parent );


  /// @brief Obtains aditional cell info, which is stored in the cell,
  /// i.e.: the sample point indices at the 8 corners, and it's dimensions
  void        acquireCellInfo( Cell* c  );


  /// @brief Check if there is a chance for a surface in that cell
  bool        checkForSurface( Cell* c );


  /// @brief the fucntion which perfoms the checks for subdivision
  bool        checkForSubdivision( Cell* c );


  /// @brief the function which checks for more than one sign change on an edge
  bool        checkForEdgeAmbiguity(  Cell* c );


  /// @brief checks for a complex surface based on the complexSurfaceThreshold
  bool        checkForComplexSurface( Cell* c );


  /// @brief finds the gradients at any position in space using the forword difference
  void        findGradient(       Vec3&     o_gradient    ,
                            const Index3D&  i_array3dInds );




  ///---- Half-Face Assignment Functions ----///

  void        findNeighbours(Cell* c);

  /// @brief The function that loops through all the cells and
  /// for each neighbouring cells sets their twin values on their faces
  /// thus making the half-face structure
  void        populateHalfFaces();


  /// @brief Setting the half-face structure of Cell A and Cell B
  /// where the contact variable names the face of Cell B into contact!
  void        setFaceTwins( Cell* a ,
                            Cell* b ,
                            CONTACT );


  ///---- Parent-Children Face Relationship Functions ----///


  /// @brief Loops through all the cells and establishes face relationships
  /// between parent and child cells
  void        setFaceRelationships();


  /// @brief Find the transitional faces
  void        markTransitionalFaces();


private:
  Cell*       m_root;

  CellVec     m_cells;

  CellVec     m_leafCells;

  std::map<uint, Cell*> m_cellAddresses;

  FaceVec     m_faces;

  Index3D&    m_samples;

  A3DFloat&   m_sampleData;

  uint&       m_minLvl;

  uint&       m_maxLvl;

  Vec3&       m_offsets;

  Isosurface* m_fn;

  float&      m_complexSurfThresh;
};



} //namespace cms

#endif //CMS_OCTREE_H
