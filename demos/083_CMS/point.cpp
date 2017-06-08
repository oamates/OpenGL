#include "point.hpp"

namespace cms
{

Point::Point() :
  m_position(Vec3(1,0,0)), m_value(10.f)
{
}

Point::Point(Vec3 i_position, float i_value) :
  m_position(i_position), m_value(i_value)
{
}

Point::Point(Vec3 i_position, float i_value, Vec3 i_gradient) :
  m_position(i_position), m_value(i_value), m_gradient(i_gradient)
{
}

Point::~Point()
{
}

//------------------------------------------------------------

void Point::setPosition(Vec3 i_position)
{
  m_position = i_position;
}

//------------------------------------------------------------

const Vec3& Point::getPosition() const
{
  return m_position;
}

//------------------------------------------------------------

void Point::setValue(float i_value)
{
  m_value = i_value;
}

//------------------------------------------------------------

const float& Point::getValue() const
{
  return m_value;
}

//------------------------------------------------------------

void Point::setGradient(Vec3 i_gradient)
{
  m_gradient = i_gradient;
}

//------------------------------------------------------------

const Vec3& Point::getGradient() const
{
  return m_gradient;
}

} //namespace cms
