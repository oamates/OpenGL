#include <cmath>

#include "vec3.hpp"

namespace cms
{


//---------------------------------------------
void Vec3::set(Real _x,   Real _y,  Real _z )
{
    m_x=_x;
    m_y=_y;
    m_z=_z;
}

//---------------------------------------------
void Vec3::set( const Vec3& _v )
{
   m_x=_v.m_x;
   m_y=_v.m_y;
   m_z=_v.m_z;
}

//---------------------------------------------

void Vec3::set( const Vec3* _v )
{
    m_x=_v->m_x;
    m_y=_v->m_y;
    m_z=_v->m_z;
}

//---------------------------------------------

Vec3::Real Vec3::dot( const Vec3& _v ) const
{
    return m_x * _v.m_x + m_y * _v.m_y + m_z * _v.m_z;
}

//---------------------------------------------

void Vec3::null()
{
    m_x=0.0f;
    m_y=0.0f;
    m_z=0.0f;
}

//---------------------------------------------
Vec3::Real& Vec3::operator[](const int& _i )
{
    return (&m_x)[_i];
}


//---------------------------------------------
Vec3 Vec3::operator-() const
{
    return Vec3(-m_x,-m_y,-m_z);
}


//---------------------------------------------
void Vec3::operator+=(const Vec3& _v   )
{
    m_x+=_v.m_x;
    m_y+=_v.m_y;
    m_z+=_v.m_z;
}

//---------------------------------------------
void Vec3::operator/=(Real _v  )
{
    m_x/=_v;
    m_y/=_v;
    m_z/=_v;
}
//---------------------------------------------
void Vec3::operator*=(  Real _v  )
{
    m_x*=_v;
    m_y*=_v;
    m_z*=_v;
}
//---------------------------------------------
void Vec3::operator-=(const Vec3& _v )
{
    m_x-=_v.m_x;
    m_y-=_v.m_y;
    m_z-=_v.m_z;
}

//---------------------------------------------
Vec3 Vec3::operator/(Real _v  )const
{
    return Vec3(
                m_x/_v,
                m_y/_v,
                m_z/_v
                );
}

//---------------------------------------------
Vec3 Vec3::operator+( const Vec3& _v )const
{
    return Vec3(
                m_x+_v.m_x,
                m_y+_v.m_y,
                m_z+_v.m_z
                );
}

//---------------------------------------------
Vec3 Vec3::operator-( const Vec3& _v )const
{
    return Vec3(
                m_x-_v.m_x,
                m_y-_v.m_y,
                m_z-_v.m_z
                );
}

//---------------------------------------------
bool Vec3::operator==( const Vec3& _v )const
{
    return (
            FCompare(_v.m_x,m_x)  &&
            FCompare(_v.m_y,m_y)  &&
            FCompare(_v.m_z,m_z)
            );
}

//---------------------------------------------

bool Vec3::operator!=( const Vec3& _v )const
{
    return (
            !FCompare(_v.m_x,m_x) ||
            !FCompare(_v.m_y,m_y) ||
            !FCompare(_v.m_z,m_z)
            );
}

//---------------------------------------------

Vec3 Vec3::operator*( const Vec3& _v )const
{
    return Vec3(
                m_x*_v.m_x,
                m_y*_v.m_y,
                m_z*_v.m_z
                );
}

//---------------------------------------------

Vec3 Vec3::operator/( const Vec3& _v )const
{
    return Vec3(
                m_x/_v.m_x,
                m_y/_v.m_y,
                m_z/_v.m_z
                );
}

//---------------------------------------------

Vec3 Vec3::operator *( Real _i )const
{
    return Vec3(
                m_x*_i,
                m_y*_i,
                m_z*_i
                );
}

//---------------------------------------------

Vec3 & Vec3::operator=( const Vec3& _v  )
{
    m_x = _v.m_x;
    m_y = _v.m_y;
    m_z = _v.m_z;
    return *this;
}

//---------------------------------------------

Vec3 & Vec3::operator=( Real _v )
{
  m_x = _v;
  m_y = _v;
  m_z = _v;
  return *this;
}

///@todo
//---------------------------------------------
//Vec3 & Vec3::operator=( const Vec4& _v  )
//{
//  m_x = _v.m_x;
//  m_y = _v.m_y;
//  m_z = _v.m_z;
//  return *this;
//}

//---------------------------------------------
void Vec3::cross( const Vec3& _v1, const Vec3& _v2 )
{
  m_x=_v1.m_y*_v2.m_z-_v1.m_z*_v2.m_y;
  m_y=_v1.m_z*_v2.m_x-_v1.m_x*_v2.m_z;
  m_z=_v1.m_x*_v2.m_y-_v1.m_y*_v2.m_x;
}

//---------------------------------------------

Vec3 Vec3::cross( const Vec3& _v )const
{
  return Vec3(
              m_y*_v.m_z - m_z*_v.m_y,
              m_z*_v.m_x - m_x*_v.m_z,
              m_x*_v.m_y - m_y*_v.m_x
             );
}


//---------------------------------------------

void Vec3::normalize()
{
  Real len=(Real)sqrt(m_x*m_x+m_y*m_y+m_z*m_z);
  m_x/=len;
  m_y/=len;
  m_z/=len;
}

//---------------------------------------------
Vec3::Real Vec3::inner( const Vec3& _v )const
{
  return (
          (m_x * _v.m_x) +
          (m_y * _v.m_y) +
          (m_z * _v.m_z)
         );
}

//---------------------------------------------
Vec3 Vec3::outer(const Vec3 &_v )  const
{
  Real x = (m_y * _v.m_z) - (m_z * _v.m_y);
  Real y = (m_z * _v.m_x) - (m_x * _v.m_z);
  Real z = (m_x * _v.m_y) - (m_y * _v.m_x);

  return Vec3(x,y,z);
}

//---------------------------------------------
Vec3::Real Vec3::length() const
{
  return (Real)sqrt((m_x*m_x)+(m_y*m_y)+(m_z*m_z));
}


//---------------------------------------------
Vec3::Real Vec3::lengthSquared() const
{
  return m_x*m_x + m_y*m_y + m_z*m_z;
}


/// Modified by - George Rassovsky - start
//---------------------------------------------
std::ostream& operator<<( std::ostream& _output, const Vec3& _v )
{
  return _output<<"["<<_v.m_x<<","<<_v.m_y<<","<<_v.m_z<<"]";
}
//---------------------------------------------
std::istream& operator>>(std::istream& _input,   Vec3& _s   )
{
  return _input >> _s.m_x >> _s.m_y >> _s.m_z;
}
/// Modified by - George Rassovsky - end

} //namespace cms

