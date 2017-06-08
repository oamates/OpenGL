#ifndef CMS_ALGCMS_H
#define CMS_ALGCMS_H

#include <vector>
#include <list>

#include "vertex.hpp"
#include "isosurface.hpp"
#include "mesh.hpp"
#include "point.hpp"
#include "edge.hpp"
#include "range.hpp"
#include "cell.hpp"
#include "array3d.hpp"
#include "index3d.hpp"
#include "octree.hpp"

namespace cms
{

/// @class AlgCMS
/// @brief The actual Cubical Marching Squares isosurface extraction algorithm
/// @brief Inherits from the abstract base class Isosurface
/// @todo finish preservation of 2d sharp features and face disambiguation functions
///
class AlgCMS : public Isosurface
{
  /// @brief AlgCMS class type definitions
  typedef std::vector<Strip> StripVec;
  typedef std::vector<Vertex> VertexVec;
  typedef Array3D<EdgeBlock> A3DEdgeBlock;
  typedef Array3D<float> A3DFloat;
  
public:

//================== Constutor(s) and Destructor ====================
  /// @brief Empty constructor
  /// @warning need be set first, before using!
            AlgCMS();


  /// @brief Function (isosurf) only constructor
  /// the resolution is set to defualt 2-6 (4-128)
  /// the bbox is defaulted to: -1 - 1 in xyz
            AlgCMS( Isosurface* i_fn );


  /// @brief A semi-full constructor
  /// supplying the isosurface, bbox and only the max depth of the octree
  /// the min depth (base) will be be defaulted to 2
             AlgCMS(       Isosurface *i_fn    ,
                  const Range i_container[] ,
                        uint i_octreeDepth  );


  /// @brief Full constructor
  /// @param taking an isosurface, bbox, octree min and max depth
             AlgCMS(       Isosurface* i_fn          ,
                  const Range       i_container[] ,
                        uint        i_octreeBase  ,
                        uint        i_octreeDepth );


  /// @brief copy constructor
             AlgCMS( const AlgCMS& i_copy );


  /// @brief Destructor
  /// destorying the octree instance
              ~AlgCMS();


//================== Public Interface Functions ====================

  /// @brief the overloaded function call operator from Isosurface
  Real        operator()( Real x ,
                          Real y ,
                          Real z ) const;


  /// @brief Setting manually the minimum and maximum Octree levels
  /// the minimum would be clamped at 2; a normal range is [3 - 8]
  void        setOctreeLevels( uint i_min ,
                               uint i_max );


  /// @brief Returns the min([0]) and max([1]) levels of the octree
  /// @param takes a reference to an unsigned int array with size >2
  void        getOctreeLevels( uint* o_lvls );


  /// @brief Set the sampling quality manually for X, Y and Z dimensions
  /// @param Three floating point values for X, Y and Z
  void        setSamples( int i_xSamp ,
                          int i_ySamp ,
                          int i_zSamp );


  /// @brief Returns the sampling quality values for X, Y and Z
  /// @param takes in a array of int with size >3
  void        getSamples( int* o_samps ) const;


  /// @brief Setting the interpolation quality level (Default is 5)
  void        setZeroApproximation( uint i_zeroApproximation );


  /// @brief Returning the interpolation quality level
  int         getZeroApproximation() const;


  /// @brief Setting the value which would be used to check
  /// for a complex surface in a cell (-1...1)
  void        setComplexSurfThresh( float i_complexSurfThresh );


  /// @brief Returning the value used for the check of a complex surface
  float       getComplexSurfThresh() const;


  /// @brief The main function which would sample the function and extract
  /// a surface out of it, saving it into a given mesh object
  bool         extractSurface( Mesh& o_mesh );


  /// @brief The sampling function used by the extractSurface function
  /// it could be called separately but not necessary
  bool         sampleFunction();


  /// @brief Returns the snap to median value
  bool         snapMedian() const;


  /// @brief Sets the snap median to surface value
  void         setSnapMedian( bool snapMedian );


  //=============== Debugging and Visualisation Tools =================

  /// @brief Exporting a Py script at the specified path
  /// contains Maya Python commands which create boxes representing oc-cells
  /// justLeafs (optional) - if true will cull out all BRANCH and empty cells
  bool         exportOctreeToMaya( const std::string& i_fName           ,
                                         bool         justLeafs = false ) const;


  /// @brief Exporting a Py script (for blender) at the specified path
  /// works same as maya script but for blender
  /// @warning requires blender 2.72 or higher cause of 'radius' keyword
  bool         exportOctreeToBlender( const std::string& i_fName           ,
                                            bool         justLeafs = false ) const;


  /// @brief a testing function which is used to test whether a cell id belongs to the
  /// desired cells to mesh
  bool         isInDesired( int _id );


  /// @brief the desired cells need to be traversed individually
  void         fixDesiredChildren();


  /// @brief an additional traversal if there are 'desired' cells
  void         traverseForDesired( Cell* c );


  /// @brief a vector of indices to the cells which are desired to be meshed
  intVec       m_desiredCells;


private:

//================== Private Members ========================

  // ---=== Datastructures ===---

  /// @brief The sampling 1D array masked in an Array3D wrapper class
  A3DFloat     m_sampleData;


  /// @brief The edgeblock 1D array masked in an Array3D wrapper class
  /// EdgeBlock has 3 edges
  A3DEdgeBlock m_edgeData;


  /// @brief The vertex array, wtoring all the vertices in the mesh-to-be
  VertexVec    m_vertices;


  /// @brief The octree of the current function
  Octree*      m_octree;


  // ---=== Member Variables ===---

  /// @brief a Mesh object which will get populated,
  /// once the algorithm is done, and store the verts, inds, normals
  Mesh         m_mesh;


  /// @brief A ptr to the specified Isosurface
  Isosurface*  m_fn;


  /// @brief the samples in xyz
  Index3D      m_samples;


  /// @brief a flag denoting if the function has been samlped
  bool         m_sampled;


  /// @brief the bbox of the function
  Range        m_container[3];


  /// @brief the individual dimenstions in 3D space
  float        m_xMax,
               m_xMin,
               m_yMax,
               m_yMin,
               m_zMax,
               m_zMin;


  /// @brief the dimenstions of the bbox as a vec3
  Vec3         m_offsets;


  /// @brief a pointer to the root of the octree
  Cell*        m_octreeRoot;


  // ---=== Parameters ===---

  /// @brief The level to which the base grid
  /// is to be subdivided (foundation of the octree)
  uint         m_octMinLvl;


  /// @brief The maximum level (depth) of the octree
  uint         m_octMaxLvl;


  /// @brief The user defined threshold of what should
  /// be regarded as a complex surface within a cell
  float        m_complexSurfThresh;


  /// @brief the linear interpolation quality
  /// e.g. the maximum number of recursions
  uint         m_zeroApproximation;


  /// @brief A flag which the user can set, whether the median point
  /// in each triangle fan should be interpolated (snapped) onto the isosurf
  bool         m_snapMedian;



  // ---=== Private Functions ===---

  /// @brief initialisation of the number of samples based on the octree levels
  /// this should be separate from the initialize function
  void         initSamples();


  /// @brief Called by all the constructors,
  /// it initializes some member variables to their default values.
  void         initialize();


  /// @brief comment
  Vec3         findCrossingPoint(       uint quality ,
                                  const Point& pt0   ,
                                  const Point& pt1   );
  
  
  /// @brief todo comment
  void         makeFaceSegments( const Index3D inds[] ,
                                       Face* i_face   );


  /// @brief todo comment
  void         tessellateComponent( Mesh&    o_mesh    ,
                                    uintVec& component );


  /// @brief todo comment
  void         makeTri( Mesh&    o_mesh          ,
                        uintVec& i_threeVertInds );
  
  
  /// @brief todo comment
  void         makeTriFan( Mesh&    o_mesh         ,
                           uintVec& i_cellVertInds );


  /// @brief todo comment
  void         makeTriSeq( Mesh&    o_mesh         ,
                           uintVec& i_cellVertInds );


  /// @brief todo comment
  void         findGradient(       Vec3& o_gradient   ,
                             const Vec3& i_dimensions ,
                             const Vec3& i_position   );
  
  
  /// @brief todo comment
  void         findGradient(       Vec3&  o_gradient   ,
                             const Vec3&  i_dimensions ,
                             const Vec3&  i_position   ,
                             const float& i_value      );


  /// @brief A recursive function that will take the Root Cell and traverse the tree
  /// extracting the surface from each LEAF
  void         segmentsTraversal( Cell* c );


  /// @brief Traverse the octree and tessellate all the components on each LEAF cell
  /// @param takes in a cell pointer (currently traversed) and a reference to the final mesh
  void         tessellationTraversal( Cell* c , 
                                      Mesh& m );


  /// @brief Finds all the edges that are located inbetween the two given points.
  /// it stores the array3d indices of those edges in the provided vector
  /// and returns a int value of the direction in which they advance 0-right(x), 1-up(y), 2-front(z)
  int           getEdgesBetwixt(       Range& o_range ,
                                 const Index3D& pt0   ,
                                 const Index3D& pt1   ) const;


  /// @brief Finds the exact place of the isovalue crossing point by tracking
  /// down the sign change, returning the index of the first point (smaller)
  int           exactSignChangeIndex( const Range&   range ,
                                            int&     dir   ,
                                            Index3D& ind0  ,
                                            Index3D& ind1  ) const;


  /// @brief Creates a new vertex on the edge of the exact crossing point
  /// it calls the findCrossingPoint
  void           makeVertex(       Strip&   o_strip        ,
                             const int&     dir            ,
                             const Index3D& crossingIndex0 ,
                             const Index3D& crossingIndex1 ,
                                   int      _i             );


  /// @brief a function which creates a Strip and
  /// populates both of it's sides by calling populate strip
  void           makeStrip(       int     edge0    ,
                                  int     edge1    ,
                            const Index3D inds[]   ,
                                  Face*   i_face   ,
                                  int     stripInd );


  /// @brief the function which populates a given strip
  /// by adding a new vertex on the exact isosurface crossing point
  /// or copying an existing one onto the strip, if that edge is previously populated
  void           populateStrip(       Strip&  o_s,
                                const Index3D inds[],
                                      int     index );

  //------------

  /// @brief Performing the main stages of the algorithm.
  /// Not strictly following the AlgCMS as described by (Ho et al. 2005)
  /// But staying loyal to the main ideas
  void           cubicalMarchingSquaresAlg();


  /// @brief Starts from a given node and traverses through the octree
  /// and generate segments for all LEAF cells.
  /// @param Takes in a pointer to a cell, which would normally be the root
  void           generateSegments( Cell* c );


  /// @brief For a given transitional face, collect all
  void           segmentFromTwin( Face*    face        ,
                                  uintVec& o_comp      ,
                                  int      lastData    ,
                                  int&     currentEdge );

  /// @brief Loading all the vertices onto the mesh
  /// @param A regference to the mesh that has to be populated with verts
  void           createMesh( Mesh& o_mesh );


  //---Tracing Components---
  /// @brief collect all the strips of a cell into a single array
  /// also populate another array with transitional segments in the case
  /// that the cell had transitional faces
  void            collectStrips( Cell*       c             ,
                                 StripVec&   o_cellStrips  ,
                                 uintVecVec& o_transitSegs );


  /// @brief Taking all the strips of a given cell and linking them together to form components
  void            linkStrips( uintVec&    o_comp      ,
                              StripVec&   strips      ,
                              uintVecVec& transitSegs );


  /// @brief Used to compare whether a given strip and an existing segment match
  /// by checking the first and last value of the segment against the strip data
  /// @return true if they match and false if they don't
  bool             compareStripToSeg( Strip&   str ,
                                      uintVec& seg );


  /// @brief Inserts data from twin of a transitional face. Using provided segments.
  /// @param all parameters are references to arrays and flags, which get writen and returned
  void             insertDataFromTwin(       uintVec&    o_comp           ,
                                             uintVecVec& segs             ,
                                             Strip&      str              ,
                                             bool&       transit          ,
                                             int&        addedInIterconst ,
                                       const bool&       backwards        );


  /// @brief Loops through all transitional faces and calls the resolve function on them
  void              editTransitionalFaces();


  /// @brief Takes a transitional face and collects all the strrips from it's twin
  /// linking them together, those that can get linked.
  /// @param a transitional face
  void              resolveTransitionalFace( Face* face );


  /// @brief Looping through all levels of the octree from the deepest up to the root
  /// and looping through all cells linking their strips into components.
  void              traceComponent();


  /// @brief
  void              traverseFace( Face*     face          ,
                                  StripVec& transitStrips );
};

} //namespace cms

#endif //CMS_ALGCMS_H
