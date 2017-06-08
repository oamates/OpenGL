#include "vertex.hpp"

namespace cms
{


Vertex::Vertex() :
  m_pos(Vec3(0,0,0)),
  m_normal(Vec3(0,0,0))
{
}

//----------------------------------------------------------------

Vertex::Vertex( Vec3 i_pos, Vec3 i_normal ) :
  m_pos(i_pos),
  m_normal(i_normal)
{
}

//----------------------------------------------------------------

void Vertex::setPos( Vec3 i_pos )
{
  m_pos = i_pos;
}

//----------------------------------------------------------------

const Vec3& Vertex::getPos() const
{
  return m_pos;
}

//----------------------------------------------------------------

void Vertex::setNormal( Vec3 i_normal )
{
  m_normal = i_normal;
}

//----------------------------------------------------------------

const Vec3& Vertex::getNormal() const
{
  return m_normal;
}

//----------------------------------------------------------------

void Vertex::print() const
{
  std::cout<<"Point: " << m_pos << std::endl <<
             "Normal: " << m_normal << std::endl;
}


} // namespace cms
