#include "index3d.hpp"

namespace cms
{

Index3D::Index3D() :
  m_x(-1), m_y(-1), m_z(-1)
{
}

Index3D::Index3D(const int _i) :
  m_x(_i), m_y(_i), m_z(_i)
{
}

Index3D::Index3D(const int _x, const int _y, const int _z) :
  m_x(_x), m_y(_y), m_z(_z)
{
}


//----------------------------------------------------------

Index3D::Index3D( const Index3D& _ind  ) :
  m_x(_ind.m_x), m_y(_ind.m_y), m_z(_ind.m_z)
{
}

//----------------------------------------------------------

int& Index3D::operator []( const int& _i )
{
  return (&m_x)[_i];
}

//----------------------------------------------------------

Index3D& Index3D::operator =( const Index3D& i_ind  )
{
  m_x = i_ind.m_x;
  m_y = i_ind.m_y;
  m_z = i_ind.m_z;
  return *this;
}

//----------------------------------------------------------

Index3D Index3D::operator -( const int& _i  )
{
  Index3D temp(*this);
  temp.m_x -= _i;
  temp.m_y -= _i;
  temp.m_z -= _i;

  return temp;
}


//----------------------------------------------------------

Index3D Index3D::operator-( const Index3D& _ind )
{
  Index3D temp(*this);
  temp.m_x -= _ind.m_x;
  temp.m_y -= _ind.m_y;
  temp.m_z -= _ind.m_z;
  return temp;
}

//----------------------------------------------------------

Index3D Index3D::operator+( const int& _i   )
{
  Index3D temp(*this);
  temp.m_x += _i;
  temp.m_y += _i;
  temp.m_z += _i;
  return temp;
}

//----------------------------------------------------------

Index3D Index3D::operator+( const Index3D& _ind )
{
  Index3D temp(*this);
  temp.m_x += _ind.m_x;
  temp.m_y += _ind.m_y;
  temp.m_z += _ind.m_z;
  return temp;
}

//----------------------------------------------------------

void Index3D::operator -=( const int& _i    )
{
  m_x -= _i;
  m_y -= _i;
  m_z -= _i;
}

//----------------------------------------------------------

void Index3D::operator +=(const int& _i)
{
  m_x += _i;
  m_y += _i;
  m_z += _i;
}

//----------------------------------------------------------

bool Index3D::operator==(const Index3D& _ind ) const
{
  return (
          (_ind.m_x == m_x)  &&
          (_ind.m_y == m_y)  &&
          (_ind.m_z == m_z)
         );
}

//----------------------------------------------------------

bool Index3D::operator!=(const Index3D& _ind  ) const
{
  return (
          (_ind.m_x != m_x) ||
          (_ind.m_y != m_y) ||
          (_ind.m_z != m_z)
         );
}

//----------------------------------------------------------

std::ostream& operator<<(std::ostream& _output, const Index3D& _i)
{
  return _output<<"["<<_i.m_x<<", "<<_i.m_y<<", "<<_i.m_z<<"]";
}

//----------------------------------------------------------

} //namespace cms
