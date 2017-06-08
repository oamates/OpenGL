
#ifndef CMS_POINT_H
#define CMS_POINT_H

#include "vec3.hpp"


namespace cms
{


/// @class Point
/// @brief Stores a position in space and it's function value
class Point
{
public:
               Point();

               Point( Vec3  i_position ,
                      float i_value    );

               Point( Vec3  i_position ,
                      float i_value    ,
                      Vec3  i_gradient );

               ~Point();

  void         setPosition( Vec3 i_position );
  const Vec3&  getPosition() const;

  void         setValue( float i_value );
  const float& getValue() const;

  void         setGradient( Vec3 i_gradient );
  const Vec3&  getGradient() const;


private:
  Vec3         m_position;
  float        m_value;
  Vec3         m_gradient;
};


} //namespace cms

#endif //CMS_POINT_H
