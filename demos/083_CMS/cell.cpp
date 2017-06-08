#include <cstdint>
#include "cell.hpp"

namespace cms
{


Cell::Cell( int _id,
            CellState i_state,
            Cell* i_parent,
            uint8_t i_subdivLvl,
            Index3D i_c000,   /// @todo index3d values not initialized??
            Index3D i_offsets,
            int8_t i_posInPar ) :
  m_id(_id), m_state(i_state), m_parent(i_parent), m_subdivLvl(i_subdivLvl),
  m_c000(i_c000), m_offsets(i_offsets), m_posInParent(i_posInPar)
{
  m_centre = Vec3(0.f, 0.f, 0.f);

  // Initialising the 8 octree children to NULL
  for(int i=0;i<8;++i)
    m_children[i] = NULL;


  // Generating the 6 cell faces / HEAP /
  for(int i=0;i<6;++i)
  {
    m_faces[i] = new Face(i, m_id);
  }


  // Initialise the address
  if(m_parent)
  {
    m_address.set(m_parent->m_address.getRaw(), m_posInParent+1);
  }
  else
  {
      m_address.reset();
  }

  // Clear the neighbour array
  for(int i=0;i<6;++i)
  {
    m_neighbours[i] = NULL;
  }
}



Cell::~Cell()
{
  // Delete the six faces of the cell
  for(int i=0;i<6;++i)
  {
    if(m_faces[i])
      delete m_faces[i];
  }
}


//----------- Accessors and Mutators --------------


void Cell::setState(CellState i_state)
{
  m_state = i_state;
}


const CellState& Cell::getState() const
{
  return m_state;
}


//----------------------------------


void Cell::setParent(Cell* i_parent)
{
  m_parent = i_parent;
}


Cell* Cell::getParent() const
{
  return m_parent;
}


//----------------------------------


void Cell::setSubdivLvl(uint i_subdivLvl)
{
  m_subdivLvl = i_subdivLvl;
}


const uint8_t& Cell::getSubdivLvl() const
{
  return m_subdivLvl;
}


//----------------------------------


void Cell::setC000(Index3D i_c000)
{
  m_c000 = i_c000;
}


const Index3D& Cell::getC000() const
{
  return m_c000;
}


//----------------------------------


void Cell::setOffsets(Index3D i_offsets)
{
  m_offsets = i_offsets;
}


const Index3D &Cell::getOffsets() const
{
  return m_offsets;
}


//----------------------------------


void Cell::setPointInds(Index3D i_pointInds[])
{
  for(int i=0;i<8;++i)
  {
    m_pointInds[i] = i_pointInds[i];
  }
}


const Index3D *Cell::getPointInds() const
{
  return m_pointInds;
}


//----------------------------------


void Cell::setCentre(Vec3 i_centre)
{
  m_centre = i_centre;
}


const Vec3& Cell::getCentre() const
{
  return m_centre;
}


//----------------------------------


void Cell::pushChild(Cell *child, const unsigned int index)
{
  m_children[index] = child;
}


Cell* Cell::getChild(const unsigned int index) const
{
  return m_children[index];
}


//----------------------------------


void Cell::setWidth(float i_width)
{
  m_width = i_width;
}


const float& Cell::getWidth() const
{
  return m_width;
}


//----------------------------------


void Cell::setHeight(float i_height)
{
  m_height = i_height;
}


const float& Cell::getHeight() const
{
  return m_height;
}


//----------------------------------


void Cell::setDepth(float i_depth)
{
  m_depth = i_depth;
}


const float& Cell::getDepth() const
{
  return m_depth;
}


//----------------------------------


void Cell::pushComponent(std::vector<unsigned int> i_comp)
{
  m_components.push_back(i_comp);
}


std::vector<std::vector<unsigned int> > Cell::getComponents()
{
  return m_components;
}


//----------------------------------


Face* Cell::getFaceAt(int position)
{
  return m_faces[position];
}


//----------------------------------


int8_t Cell::getPosInParent()
{
  return m_posInParent;
}


} //namespace cms
