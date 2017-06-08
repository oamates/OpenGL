#include <cmath>

#include "array3d.hpp"
#include "edge.hpp"

namespace cms
{

template <class T>
Array3D<T>::Array3D()
{
  for(int i=0;i<3;++i)
  {
    m_indices[i] = 1;
    m_bbox[i] = Range(-1.f, 1.f);
  }
}

template <class T>
Array3D<T>::Array3D(Range bbox[3], int xSlab, int ySlab, int zSlab)
{
  for(int i=0;i<3;++i)
    m_bbox[i] = bbox[i];

  m_indices.m_x = xSlab;
  m_indices.m_y = ySlab;
  m_indices.m_z = zSlab;
  m_data.resize(m_indices.m_x*m_indices.m_y*m_indices.m_z);
}

template <class T>
Array3D<T>::~Array3D()
{}

//-----------------------------------------------------

template <class T>
void Array3D<T>::operator()(const int x, const int y, const int z, T value)
{
  assert(x < m_indices.m_x);
  assert(y < m_indices.m_y);
  assert(z < m_indices.m_z);

  m_data[(x*m_indices.m_y+y)*m_indices.m_z+z] = value;
}

/// todo operator(Index3D xyz, T value) ??? and for getValueAt ???

//-----------------------------------------------------

template <class T>
int Array3D<T>::size() const
{
  return m_data.size();
}

//-----------------------------------------------------

template <class T>
int Array3D<T>::sizeX() const
{
  return m_indices.m_x;
}

//-----------------------------------------------------

template <class T>
int Array3D<T>::sizeY() const
{
  return m_indices.m_y;
}

//-----------------------------------------------------

template <class T>
int Array3D<T>::sizeZ() const
{
  return m_indices.m_z;
}

//-----------------------------------------------------

template <class T>
void Array3D<T>::setValueAt(int x, int y, int z, T value)
{
  assert(x < m_indices.m_x);
  assert(y < m_indices.m_y);
  assert(z < m_indices.m_z);

  m_data[(x*m_indices.m_y+y)*m_indices.m_z+z] = value;
}

//-----------------------------------------------------

template <class T>
void Array3D<T>::setValueAt(Index3D xyz, T value)
{
  assert(xyz.m_x < m_indices.m_x);
  assert(xyz.m_y < m_indices.m_y);
  assert(xyz.m_z < m_indices.m_z);

  m_data[(xyz.m_x*m_indices.m_y+xyz.m_y)*m_indices.m_z+xyz.m_z] = value;
}

//-----------------------------------------------------

template <class T>
T Array3D<T>::getValueAt(int x, int y, int z) const
{
  assert(x < m_indices.m_x);
  assert(y < m_indices.m_y);
  assert(z < m_indices.m_z);

  return m_data[(x * m_indices.m_y + y) * m_indices.m_z + z];
}

//-----------------------------------------------------

template <class T>
T Array3D<T>::getValueAt(Index3D xyz) const
{
  assert(xyz.m_x < m_indices.m_x);
  assert(xyz.m_y < m_indices.m_y);
  assert(xyz.m_z < m_indices.m_z);

  return m_data[(xyz.m_x * m_indices.m_y + xyz.m_y) * m_indices.m_z + xyz.m_z];
}

//-----------------------------------------------------

template <class T>
int Array3D<T>::getIndexAt(int x, int y, int z) const
{
  assert(x < m_indices.m_x);
  assert(y < m_indices.m_y);
  assert(z < m_indices.m_z);

  return (x * m_indices.m_y + y) * m_indices.m_z + z;
}

//-----------------------------------------------------

template <class T>
int Array3D<T>::getIndexAt(Index3D xyz) const
{
  assert(xyz.m_x < m_indices.m_x);
  assert(xyz.m_y < m_indices.m_y);
  assert(xyz.m_z < m_indices.m_z);

  return (xyz.m_x*m_indices.m_y+xyz.m_y)*m_indices.m_z+xyz.m_z;
}

//-----------------------------------------------------

template <class T>
Vec3 Array3D<T>::getPositionAt(int x, int y, int z) const
{
  Vec3 pos;

  const float tx = static_cast<float>(x)/static_cast<float>(m_indices.m_x-1);
  pos.m_x = m_bbox[0].m_lower + (m_bbox[0].m_upper - m_bbox[0].m_lower)*tx;

  const float ty = static_cast<float>(y)/static_cast<float>(m_indices.m_y-1);
  pos.m_y = m_bbox[1].m_lower + (m_bbox[1].m_upper - m_bbox[0].m_lower)*ty;

  const float tz = static_cast<float>(z)/static_cast<float>(m_indices.m_z-1);
  pos.m_z = m_bbox[2].m_lower + (m_bbox[2].m_upper - m_bbox[0].m_lower)*tz;

  assert((pos.m_x >= m_bbox[0].m_lower)&&(pos.m_x <= m_bbox[0].m_upper));
  assert((pos.m_y >= m_bbox[1].m_lower)&&(pos.m_y <= m_bbox[1].m_upper));
  assert((pos.m_z >= m_bbox[2].m_lower)&&(pos.m_z <= m_bbox[2].m_upper));

  return pos;
}

//-----------------------------------------------------

template <class T>
Vec3 Array3D<T>::getPositionAt(Index3D xyz) const
{
  Vec3 pos;

  const float tx = static_cast<float>(xyz.m_x)/static_cast<float>(m_indices.m_x-1);
  pos.m_x = m_bbox[0].m_lower + (m_bbox[0].m_upper - m_bbox[0].m_lower)*tx;

  const float ty = static_cast<float>(xyz.m_y)/static_cast<float>(m_indices.m_y-1);
  pos.m_y = m_bbox[1].m_lower + (m_bbox[1].m_upper - m_bbox[0].m_lower)*ty;

  const float tz = static_cast<float>(xyz.m_z)/static_cast<float>(m_indices.m_z-1);
  pos.m_z = m_bbox[2].m_lower + (m_bbox[2].m_upper - m_bbox[0].m_lower)*tz;

  assert((pos.m_x >= m_bbox[0].m_lower)&&(pos.m_x <= m_bbox[0].m_upper));
  assert((pos.m_y >= m_bbox[1].m_lower)&&(pos.m_y <= m_bbox[1].m_upper));
  assert((pos.m_z >= m_bbox[2].m_lower)&&(pos.m_z <= m_bbox[2].m_upper));

  return pos;
}

//-----------------------------------------------------

template <class T>
void Array3D<T>::resize(int xSlab, int ySlab, int zSlab)
{
  m_indices.m_x = xSlab;
  m_indices.m_y = ySlab;
  m_indices.m_z = zSlab;
  m_data.resize(m_indices.m_x*m_indices.m_y*m_indices.m_z);
}

//-----------------------------------------------------

template <class T>
void Array3D<T>::resize(Index3D slabs)
{
  m_indices.m_x = slabs.m_x;
  m_indices.m_y = slabs.m_y;
  m_indices.m_z = slabs.m_z;
  m_data.resize(m_indices.m_x*m_indices.m_y*m_indices.m_z);
}

//-----------------------------------------------------

template <class T>
void Array3D<T>::setBBox(Range bbox[])
{
  for(int i=0;i<3;++i)
    m_bbox[i] = bbox[i];
}



/// Explicitly instantiating all relevant templates
template class Array3D<float>;
template class Array3D<EdgeBlock>;

} //namespace cms
