#ifndef CMS_ARRAY_3D_H
#define CMS_ARRAY_3D_H

#include <vector>
#include "vec3.hpp"
#include "index3d.hpp"
#include "range.hpp"


namespace cms
{

/// @class Arry3D
/// @brief An Array3D wrapper class
/// which acts as a wrapper for a 1D array
/// it is used in this project for 'float' and
/// 'EdgeBlock' types, in the sample and edge array
/// @todo operator()
template <class T> class Array3D
{
public:
  /// @brief Empty constructor - sets bbox to (-1,1) in xyz
       Array3D();

  /// @brief Full constructor, taking the bbox and the samples - resizing the array
       Array3D( Range bbox[3], int xSlab, int ySlab, int zSlab );

  /// @brief Destructor
       ~Array3D();


  void operator()( const int x, const int y, const int z, T value );
//  const T& operator()(const int x, const int y, const int z) const; ///todo

  /// @brief Returns the size of the whole flattened Array3D
  int  size() const;

  /// @brief Returns the size of the X slab in the Array3D
  int  sizeX() const;

  /// @brief Returns the size of the Y slab in the Array3D
  int  sizeY() const;

  /// @brief Returns the size of the Z slab in the Array3D
  int  sizeZ() const;


  /// @brief Used to set the value at a specific position in the Array3D
  /// @param Three integers denoting the location, and a value of type T
  void setValueAt( int x, int y, int z, T value );

  /// @brief Used to set the value at a specific position in the Array3D
  /// @param An Index3D denoting the location, and a value of type T
  void setValueAt( Index3D xyz, T value );


  /// @brief Returns the value from a specific position in the Array3D
  /// @param Three integers denoting the location
  T    getValueAt( int x, int y, int z ) const;

  /// @brief Returns the value from a specific position in the Array3D
  /// @param An Index3D denoting the location
  T    getValueAt( Index3D xyz ) const;


  /// @brief Returning the flattened index in the actual 1D array
  /// @param Three integers denoting the location in the Array3D
  int  getIndexAt( int x, int y, int z ) const;

  /// @brief Returning the flattened index in the actual 1D array
  /// @param An Index3D denoting the location in the Array3D
  int  getIndexAt( Index3D xyz ) const;

  /// @brief Returns a Vec3 of the exact position in 3D space of a specific sample
  /// @param Three integers denoting the location of the sample in the Array3D
  Vec3 getPositionAt( int x, int y, int z ) const;

  /// @brief Returns a Vec3 of the exact position in 3D space of a specific sample
  /// @param An Index3D denoting the location of the sample in the Array3D
  Vec3 getPositionAt( Index3D xyz ) const;


  /// @brief Resize the Array3D
  /// @param three integer values corresponding to X, Y and Z
  void resize( int xSlab, int ySlab, int zSlab );

  /// @brief Resize the Array3D
  /// @param an Index3D giving the sizes in X, Y and Z
  void resize( Index3D slabs );


  /// @brief Setting the BBox of the Array3D
  /// @param an array of ranges (should be size 3)
  /// with the min and max values in X, Y and Z
  void setBBox( Range bbox[] );

private:
  /// @brief The dynamic 1D array of the actual data
  std::vector<T> m_data;

  /// @brief An Index3D storing the size of the 3D array wrapper in X, Y and Z
  Index3D        m_indices;

  /// @brief The BBox of the Array3D stored as an array
  /// of 3 Ranges (-x +x), (-y, +y), (-z, +z)
  Range          m_bbox[3];
};

} //namespace cms


#endif //CMS_ARRAY_3D_H
