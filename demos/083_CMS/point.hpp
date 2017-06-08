#ifndef _cms_point_included_243906370634075863405634576437563978456783457813451
#define _cms_point_included_243906370634075863405634576437563978456783457813451

#include "vec3.hpp"

namespace cms
{

// Stores a position in space, it's function value and gradient
struct Point
{
    Vec3 m_position;
    float m_value;
    Vec3 m_gradient;

    Point()
        : m_position(Vec3(1, 0, 0)), 
          m_value(10.0f)
    {}

    Point(Vec3 i_position, float i_value) 
        : m_position(i_position), m_value(i_value)
    {}

    Point(Vec3 i_position, float i_value, Vec3 i_gradient)
        : m_position(i_position), m_value(i_value), m_gradient(i_gradient)
    {}

    ~Point() {};

    void setPosition( Vec3 i_position )
        { m_position = i_position; }
    const Vec3&  getPosition() const
        { return m_position; }

    void setValue( float i_value )
        { m_value = i_value; }
    const float& getValue() const
        { return m_value; }

    void setGradient(Vec3 i_gradient) 
        { m_gradient = i_gradient; }

    const Vec3&  getGradient() const 
        { return m_gradient; }

};


} // namespace cms

#endif // _cms_point_included_243906370634075863405634576437563978456783457813451