#include "edge.hpp"

namespace cms
{

Edge::Edge() :
  m_empty(true), m_vertIndex(-1)
{
  m_dir = -1;
}

Edge::~Edge()
{
}

//------------------------------------------------------------

void Edge::setEmpty(bool i_empty)
{
  m_empty = i_empty;
}

//------------------------------------------------------------

const bool& Edge::empty() const
{
  return m_empty;
}

//------------------------------------------------------------

void Edge::setVertIndex(int i_vertIndex)
{
  m_vertIndex = i_vertIndex;
}

//------------------------------------------------------------

const int& Edge::getVertIndex() const
{
  return m_vertIndex;
}

} //namespace cms
