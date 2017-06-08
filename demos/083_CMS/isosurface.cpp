#include "isosurface.hpp"

namespace cms
{

Isosurface::Isosurface()
{
  m_isoLevel = 0.f;
  loaded = false;
}

Isosurface::~Isosurface()
{}

//-------------------------------------------------------

void Isosurface::setIsolevel(Real i_isoLevel)
{
  m_isoLevel = i_isoLevel;
}

//-------------------------------------------------------

const Isosurface::Real& Isosurface::getIsolevel() const
{
  return m_isoLevel;
}

//-------------------------------------------------------

void Isosurface::setNegativeInside(bool _negInside)
{
  m_negativeInside = _negInside;
}

//-------------------------------------------------------

bool Isosurface::isNegativeInside() const
{
  return m_negativeInside;
}

} //namespace cms
