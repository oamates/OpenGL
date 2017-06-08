#ifndef CMS_VEC3_H
#define CMS_VEC3_H

#include <iostream>

namespace cms
{


/// @brief define EPSILON for floating point comparison
#ifndef EPSILON
  const static float EPSILON = 0.00001f;
#endif


 /// @brief FCompare macro used for floating point comparision functions
  #define FCompare(a,b) \
      ( ((a)-EPSILON)<(b) && ((a)+EPSILON)>(b) )
 /// Modified by - George Rassovsky - end ///



class Vec3
{
public:
  /// @brief Precision of the real for the Vec3 class
  typedef float Real;


public:

  /// @brief copy ctor
  /// @param[in] _v the value to set
  Vec3( const Vec3& _v  ) :
    m_x(_v.m_x),
    m_y(_v.m_y),
    m_z(_v.m_z){;}


  /// @brief initialise the constructor from 3 or 4 Real
  /// @param[in]  _x x value
  /// @param[in]  _y y value
  /// @param[in]  _z z value
  /// @param[in]  _w 1.0f default so acts as a points
  Vec3(Real _x=0.0,  Real _y=0.0, Real _z=0.0  ):
    m_x(_x),
    m_y(_y),
    m_z(_z){;}



  /// @brief sets the Vec3 component from 3 values
  /// @param[in]  _x the x component
  /// @param[in]  _y the y component
  /// @param[in]  _z the z component
  void set( Real _x,  Real _y,  Real _z );
  


  /// @brief set from another  Vec3
  /// @param[in]  _v the Vec3 to set from
  void set( const Vec3& _v );



  /// @brief set from another  Vec3
  /// @param[in]  _v the Vec3 to set from
  void set( const Vec3* _v );
  


  /// @brief return this dotted with _b
  /// @param[in]  _b vector to dot current vector with
  /// @returns  the dot product
  Real dot( const Vec3 &_b  )const;



  /// @brief set the Vec3 as the cross product from 2 other Vec3
  /// @param[in]  _v1 the first vector
  /// @param[in]  _v2 the second vector
  void cross(const Vec3& _v1, const Vec3& _v2 );
  


  /// @brief return the cross product of this cross with b
  /// @param[in]  _b the vector cross this with
  /// @returns  the result of this cross b
  Vec3 cross(const Vec3& _b )const;



  /// @brief clears the Vec3 to 0,0,0
  void null();
  


  /// @brief [] index operator to access the index component of the Vec3
  /// @returns  this[x] as a Real
  Real& operator[]( const int& _i ) ;



  /// @brief Normalize the vector using
  /// \n \f$x=x/\sqrt{x^2+y^2+z^2} \f$
  /// \n \f$y=y/\sqrt{x^2+y^2+z^2} \f$
  /// \n \f$z=z/\sqrt{x^2+y^2+z^2} \f$
  void normalize();
  


  /// @brief calculate the inner product of this vector and vector passed in
  /// @param[in] _v the vector to calculate inner product with
  /// @returns the inner product
  Real inner( const Vec3& _v )const;
  


  /// @brief compute the outer product of this vector and vector
  /// @param[in] _v the vector to calc against
  /// @returns a new vector
  Vec3 outer( const Vec3& _v )const;
  


  /// @brief returns the length of the vector
  /// @returns  \f$\sqrt{x^2+y^2+z^2} \f$
  Real length() const;
  


  /// @brief returns the length squared of the vector (no sqrt so quicker)
  /// @returns  \f$x^2+y^2+z^2 \f$
  Real lengthSquared() const;



  /// @brief += operator add Vec3 v to current Vec3
  /// @param[in]  &_v Vec3 to add
  void operator+=(const Vec3& _v );



  /// @brief -= operator this-=v
  /// @param[in]  &_v Vec3 to subtract
  void operator-=( const Vec3& _v );



  /// @brief this * i for each element
  /// @param[in]  _i the scalar to mult by
  /// @returns Vec3
  Vec3 operator *( Real _i )const;



  /// @brief + operator add Vec3+Vec3
  /// @param[in]  &_v the value to add
  /// @returns the Vec3 + v
  Vec3 operator +(const Vec3 &_v )const;



  /// @brief divide Vec3 components by a scalar
  /// @param[in] _v the scalar to divide by
  /// @returns a Vec3 V(x/v,y/v,z/v,w)
  Vec3 operator/( Real _v )const;



  /// @brief operator div Vec3/Vec3
  /// @param[in]  _v the value to div by
  /// @returns Vec3 / Vec3
  Vec3 operator/( const Vec3& _v )const;



  /// @brief divide this Vec3 components by a scalar
  /// @param[in] _v the scalar to divide by
  /// sets the Vec3 to Vec3 V(x/v,y/v,z/v,w)
  void operator/=( Real _v );
  


  /// @brief multiply this Vec3 components by a scalar
  /// @param[in] _v the scalar to multiply by
  /// sets the Vec3 to Vec3 V(x*v,y*v,z*v,w)
  void operator*=( Real _v );
  


  /// @brief subtraction operator subtract vevtor-Vec3
  /// @param[in]  &_v the value to sub
  /// @returns this - v
  Vec3 operator-( const Vec3 &_v )const;



  /// @brief negate the Vec3 components
  Vec3 operator-() const;



  /// @brief * operator mult vevtor*Vec3
  /// @param[in]  _v the value to mult
  /// @returns new Vec3 this*v
  Vec3 operator*( const Vec3 &_v )const;



  /// @brief assignment operator set the current Vec3 to rhs
  /// @param[in] _v the Vec3 to set
  /// @returns a new Vec3
  Vec3 &operator =( const Vec3 &_v );



  /// @brief assignment operator set the current Vec3 to rhs
  /// @param[in] _v the Real to set
  /// @returns a new Vec3
  Vec3 &operator =(Real _v );



  /// @brief check for equality uses FCompare (from Util.h) as Real values
  /// @param[in] _v the Vec3 to check against
  /// @returns true or false
  bool operator==( const Vec3 &_v )const;
  


  /// @brief not equal check
  /// @param[in] _v the Vec3 to check against
  /// @returns true of false
  bool operator!=( const Vec3 &_v )const;

public :

  /// @brief x component of the Vec3
  Real m_x;


  /// @brief y component of the Vec3
  Real m_y;


  /// @brief z component of the Vec3
  Real m_z;
};




/// @brief insertion operator to print out the Vec3
/// @param[in] _output the stream to write to
/// @param[in] _s the Vec3 to write
std::ostream& operator<<( std::ostream& _output, const Vec3& _s );
 


/// @brief extraction operator to read in  the Vec3
/// @param[in] _input the stream read from
/// @param[in] _s the Vec3 to write
std::istream& operator>>( std::istream& _input, Vec3 &_s );


} //namespace cms

#endif //CMS_VEC3_H
