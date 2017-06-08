#ifndef CMS_CELL_H
#define CMS_CELL_H

#include <vector>
#include "address.hpp"
#include "vertex.hpp"
#include "range.hpp"
#include "vec3.hpp"
#include "point.hpp"
#include "index3d.hpp"
#include "face.hpp"
#include "util.hpp"

namespace cms
{

enum CellState 
{ 
    BRANCH  = 0,                            // BRANCH may also mean EMPTY
    LEAF    = 1
};


// Cell is used to construct the octree :: each cell in the octree is a Cell object
// todo :: optimize the component array

struct Cell
{
    typedef std::vector<int8_t> int8Vec;

  //============== Constructor(s) and Destructor =============

  // Full Constructor
  // int i_id the cell's unique identifier
  // CellState i_state - the cell state - (enumerator)
  // Cell* i_parent - a pointer to the parent cell (null if root)
  // uint i_subdivLvl - the level of subdivision
  // Index3D i_c000 - the 000 corner of the cell in the samples array
  // Index3D i_offset - the dimensions of the cell in sample slabs xyz
                  Cell(int i_id,
                        CellState i_state     ,
                        Cell*     i_parent    ,
                        uint8_t   i_subdivLvl ,
                        Index3D   i_c000      ,
                        Index3D   i_offset    ,
                        int8_t    i_posInPar  );


  /// @brief Destructor
  /// looping and destroying all children individually
                   ~Cell();

  //============== Accessors and Mutators =============

  /// @brief Set enumerator cell state
  void             setState( CellState i_state );

  /// @brief Get the cell state enumerator
  const CellState& getState() const;


  /// @brief Setting the pointer to the parent cell
  void             setParent( Cell* i_parent );

  /// @brief Getting a pointer to the parent cell
  Cell*            getParent() const;


  /// @brief Setting the level of subdivision of the cell
  void             setSubdivLvl( unsigned int i_subdivLvl );

  /// @brief getting a read-only reference (unsigned int) of the cell depth
  const uint8_t&   getSubdivLvl() const;


  /// @brief Setting the corner 000 3d index
  void             setC000( Index3D i_c000 );

  /// @brief Getting a read-only 3d index of the corner at 000
  const Index3D&   getC000() const;


  /// @brief Setting the xyz offset (dimensions) of the cell in discrete samples
  void             setOffsets( Index3D i_offsets );

  /// @brief Getting the xyz offset sample (dimensions) of the cell
  const Index3D&   getOffsets() const;


  /// @brief Setting the 3D Index of the corner points of the cell
  void             setPointInds( Index3D i_pointInds[] );

  /// @brief Getting the array of indices to corner points of a cell
  const Index3D*   getPointInds() const;


  /// @brief Setting the centre of a cell in 3D space
  void             setCentre( Vec3 i_centre );

  /// @brief Getting a read-only reference to the centre of the cell
  const Vec3&      getCentre() const;


  /// @brief Add child cell point onto the children array
  /// @param providing a ptr to a child cell and the index at which it should be set
  void             pushChild(       Cell* child , 
                              const unsigned int index  );

  /// @brief Getting a child cell ptr from the specified index
  Cell*            getChild( const unsigned int index ) const;


  /// @brief Setting the width of the cell in 3D space
  void             setWidth( float i_width );

  /// @brief Getting the width of the cell in 3D space
  const float&     getWidth() const;


  /// @brief Setting the height of the cell in 3D space
  void             setHeight( float i_height );

  /// @brief Getting the height of the cell in 3D space
  const float&     getHeight() const;


  /// @brief Setting the depth of the cell in 3D space
  void             setDepth( float i_depth );

  /// @brief Getting the depth of the cell in 3D space
  const float&     getDepth() const;


  /// @brief push_back component onto the component vector of the cell
  void             pushComponent( std::vector<unsigned int> i_comp );

  /// @brief Retrieve the component array of the cell
  std::vector<std::vector<unsigned int>>       getComponents(); /// todo fix with offset


  /// @brief Get a ptr to a cell face at the specified position
  Face*            getFaceAt( int position );


  /// @brief Return a int8_t of the position of the cell in relation to it's parent
  int8_t           getPosInParent();


 public:

  //============== Public Members ================

  //temp todo
  Cell*     m_neighbours[6];

  /// @brief the public ID of the cell
  int        m_id; ///todo Is it used?

  /// @brief the public dimensions of the cell through ranges in xyz
  Range      m_x,
             m_y,
             m_z;

  /// @brief The address of the cell
  int8Vec    m_rawAddress;  ///todo: consider having address as its own type


  Address   m_address; // todo: public for now

private:

  //============== Private Members ================

  /// @brief The state of the current cell - based on State enumerator
  enum CellState m_state;

  /// @brief A pointer to this cell's parent cell
  Cell*      m_parent;

  /// @brief The level of subdivision of the cell
  uint8_t    m_subdivLvl;

  /// @brief The maximum subdivision of the octree (needed for Address so pass it down)
  uint8_t    m_maxSubdiv;

  /// @brief the index of the point at the 000 corner of the cell
  Index3D    m_c000;

  /// @brief the discrete dimensions of the cell based on the samples
  Index3D    m_offsets;

  /// @brief the 3D space dimensions of the cell in cartesian coords
  float      m_width  , // x
             m_height , // y
             m_depth  ; // z

  /// @brief Indices of the samples at the corners of the cell
  Index3D    m_pointInds[8];

  /// @brief The geometric centre of the cell as a 3d vector
  Vec3       m_centre;

  /// @brief a ptr array of the branching cells from the current cell
  Cell*      m_children[8];

  /// @brief The array of components for this cell
  std::vector<std::vector<unsigned int>> m_components; /// todo get rid of vec of vec

  /// @brief The Array of cell faces
  Face*      m_faces[6];

  /// @brief Position within parent
  int8_t     m_posInParent;

};





} //namespace cms


#endif //CMS_CELL_H
