#ifndef _cms_alg_included_78946375463727856102735651025612510235605612852735682
#define _cms_alg_included_78946375463727856102735651025612510235605612852735682

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

// AlgCMS :: The actual Cubical Marching Squares isosurface extraction algorithm
// Inherits from the abstract base class Isosurface
// todo :: finish preservation of 2d sharp features and face disambiguation functions

struct AlgCMS : public Isosurface
{
    // AlgCMS class type definitions
    typedef std::vector<Strip> StripVec;
    typedef std::vector<Vertex> VertexVec;
    typedef Array3D<EdgeBlock> A3DEdgeBlock;
    typedef Array3D<float> A3DFloat;

    //================== Constutor(s) and Destructor ====================
    AlgCMS();

    // Function (isosurf) only constructor, the resolution is set to defualt 2-6 (4-128)
    // the bbox is defaulted to: [-1..1] in xyz
    AlgCMS(Isosurface* i_fn);

    // A semi-full constructor supplying the isosurface, bbox and only the max depth of the octree
    // the min depth (base) will be be defaulted to 2
    AlgCMS(Isosurface *i_fn, const Range i_container[], unsigned int i_octreeDepth);

    // Full constructor, taking an isosurface, bbox, octree min and max depth
    AlgCMS(Isosurface* i_fn, const Range i_container[], unsigned int i_octreeBase, unsigned int i_octreeDepth);

    // copy constructor
    AlgCMS( const AlgCMS& i_copy );

    // Destructor, destorying the octree instance
    ~AlgCMS();


    //================== Public Interface Functions ====================

    // the overloaded function call operator from Isosurface
    Real operator() (Real x, Real y, Real z) const;

    // Setting manually the minimum and maximum Octree levels
    // the minimum would be clamped at 2; a normal range is [3 - 8]
    void setOctreeLevels(unsigned int i_min, unsigned int i_max);

    // Returns the min([0]) and max([1]) levels of the octree
    // params :: takes a reference to an unsigned int array with size >2
    void getOctreeLevels(unsigned int* o_lvls);

    // Set the sampling quality manually for X, Y and Z dimensions
    // params ::  Three floating point values for X, Y and Z
    void setSamples(int i_xSamp, int i_ySamp, int i_zSamp);

    // Returns the sampling quality values for X, Y and Z
    // params ::  takes in a array of int with size >3
    void getSamples(int* o_samps) const;

    // Setting the interpolation quality level (Default is 5)
    void setZeroApproximation(unsigned int i_zeroApproximation);

    // Returning the interpolation quality level
    int getZeroApproximation() const;

    // Setting the value which would be used to check
    // for a complex surface in a cell (-1...1)
    void setComplexSurfThresh(float i_complexSurfThresh);

    // Returning the value used for the check of a complex surface
    float getComplexSurfThresh() const;

    // The main function which would sample the function and extract
    // a surface out of it, saving it into a given mesh object
    bool extractSurface(Mesh& o_mesh);

    // The sampling function used by the extractSurface function
    // it could be called separately but not necessary
    bool sampleFunction();

    // Returns the snap to median value
    bool snapMedian() const;

    // Sets the snap median to surface value
    void setSnapMedian(bool snapMedian);


    //=============== Debugging and Visualisation Tools =================

    // a testing function which is used to test whether a cell id belongs to the
    // desired cells to mesh
    bool isInDesired(int _id);

    // the desired cells need to be traversed individually
    void fixDesiredChildren();

    // an additional traversal if there are 'desired' cells
    void traverseForDesired( Cell* c );

    // a vector of indices to the cells which are desired to be meshed
    std::vector<int>       m_desiredCells;

    //================== Private Members ========================

    // The sampling 1D array masked in an Array3D wrapper class
    A3DFloat     m_sampleData;

    // The edgeblock 1D array masked in an Array3D wrapper class EdgeBlock has 3 edges
    A3DEdgeBlock m_edgeData;

    // The vertex array, wtoring all the vertices in the mesh-to-be
    VertexVec    m_vertices;

    // The octree of the current function
    Octree* m_octree;

    // ---=== Member Variables ===---

    // a Mesh object which will get populated, once the algorithm is done, and store the verts, inds, normals
    Mesh         m_mesh;

    // A ptr to the specified Isosurface
    Isosurface*  m_fn;

    // the samples in xyz
    Index3D      m_samples;

    // a flag denoting if the function has been samlped
    bool m_sampled;

    // the bbox of the function
    Range m_container[3];

    // the individual dimenstions in 3D space
    float m_xMax, m_xMin,
          m_yMax, m_yMin,
          m_zMax, m_zMin;

    // the dimenstions of the bbox as a vec3
    Vec3 m_offsets;

    // a pointer to the root of the octree
    Cell* m_octreeRoot;

    // ---=== Parameters ===---

    // The level to which the base grid is to be subdivided (foundation of the octree)
    unsigned int m_octMinLvl;

    // The maximum level (depth) of the octree
    unsigned int m_octMaxLvl;

    // The user defined threshold of what should
    // be regarded as a complex surface within a cell
    float m_complexSurfThresh;

    // the linear interpolation quality, e.g. the maximum number of recursions
    unsigned int         m_zeroApproximation;

    // A flag which the user can set, whether the median point
    // in each triangle fan should be interpolated (snapped) onto the isosurf
    bool         m_snapMedian;

    // ---=== Private Functions ===---

    // initialisation of the number of samples based on the octree levels
    // this should be separate from the initialize function
    void initSamples();


    // Called by all the constructors, initializes some member variables to their default values.
    void initialize();

    Vec3 findCrossingPoint(unsigned int quality, const Point& pt0, const Point& pt1);
  
    void makeFaceSegments(const Index3D inds[], Face* i_face);

    void tessellateComponent(Mesh& o_mesh, std::vector<unsigned int>& component);

    void makeTri(Mesh& o_mesh, std::vector<unsigned int>& i_threeVertInds);
  
    void makeTriFan(Mesh& o_mesh, std::vector<unsigned int>& i_cellVertInds);

    void makeTriSeq(Mesh& o_mesh, std::vector<unsigned int>& i_cellVertInds);

    void findGradient(Vec3& o_gradient, const Vec3& i_dimensions, const Vec3& i_position);
  
    void findGradient(Vec3& o_gradient, const Vec3& i_dimensions, const Vec3& i_position, const float& i_value);

    // A recursive function that will take the Root Cell and traverse the tree
    // extracting the surface from each LEAF
    void segmentsTraversal( Cell* c );

    // Traverse the octree and tessellate all the components on each LEAF cell
    // params ::  takes in a cell pointer (currently traversed) and a reference to the final mesh
    void tessellationTraversal(Cell* c, Mesh& m);

    // Finds all the edges that are located inbetween the two given points.
    // it stores the array3d indices of those edges in the provided vector
    // and returns a int value of the direction in which they advance 0-right(x), 1-up(y), 2-front(z)
    int getEdgesBetwixt(Range& o_range, const Index3D& pt0, const Index3D& pt1) const;

    // Finds the exact place of the isovalue crossing point by tracking
    // down the sign change, returning the index of the first point (smaller)
    int exactSignChangeIndex(const Range& range, int& dir, Index3D& ind0, Index3D& ind1) const;

    // Creates a new vertex on the edge of the exact crossing point it calls the findCrossingPoint
    void makeVertex(Strip& o_strip, const int& dir, const Index3D& crossingIndex0, const Index3D& crossingIndex1, int _i);

    // a function which creates a Strip and
    // populates both of it's sides by calling populate strip
    void makeStrip(int edge0, int edge1, const Index3D inds[], Face* i_face, int stripInd);

    // the function which populates a given strip
    // by adding a new vertex on the exact isosurface crossing point
    // or copying an existing one onto the strip, if that edge is previously populated
    void populateStrip(Strip& o_s, const Index3D inds[], int index);

    //------------

    // Performing the main stages of the algorithm.
    // Not strictly following the AlgCMS as described by (Ho et al. 2005)
    // But staying loyal to the main ideas
    void cubicalMarchingSquaresAlg();


    // Starts from a given node and traverses through the octree
    // and generate segments for all LEAF cells.
    // params ::  Takes in a pointer to a cell, which would normally be the root
    void generateSegments(Cell* c);


    // For a given transitional face, collect all
    void segmentFromTwin(Face* face, std::vector<unsigned int>& o_comp, int lastData, int& currentEdge);

    // Loading all the vertices onto the mesh
    // params :: A regference to the mesh that has to be populated with verts
    void createMesh(Mesh& o_mesh);

    //---Tracing Components---
    // collect all the strips of a cell into a single array
    // also populate another array with transitional segments in the case
    // that the cell had transitional faces
    void collectStrips(Cell* c, StripVec& o_cellStrips, std::vector<std::vector<unsigned int>>& o_transitSegs);


    // Taking all the strips of a given cell and linking them together to form components
    void linkStrips(std::vector<unsigned int>& o_comp, StripVec& strips, std::vector<std::vector<unsigned int>>& transitSegs);

    // Used to compare whether a given strip and an existing segment match
    // by checking the first and last value of the segment against the strip data
    // returns true if they match and false if they don't
    bool compareStripToSeg(Strip& str, std::vector<unsigned int>& seg);


    // Inserts data from twin of a transitional face. Using provided segments.
    // param :: all parameters are references to arrays and flags, which get writen and returned
    void insertDataFromTwin(std::vector<unsigned int>& o_comp,
                          std::vector<std::vector<unsigned int>>& segs,
                          Strip& str,
                          bool& transit,
                          int& addedInIterconst,
                          const bool& backwards);

    // Loops through all transitional faces and calls the resolve function on them
    void editTransitionalFaces();

    // Takes a transitional face and collects all the strrips from it's twin
    // linking them together, those that can get linked.
    // param :: a transitional face
    void resolveTransitionalFace( Face* face );

    // Looping through all levels of the octree from the deepest up to the root
    // and looping through all cells linking their strips into components.
    void traceComponent();

    void traverseFace(Face* face, StripVec& transitStrips);
};

} // namespace cms

#endif // _cms_alg_included_78946375463727856102735651025612510235605612852735682
