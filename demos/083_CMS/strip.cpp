#include "strip.hpp"

namespace cms
{

Strip::Strip() :
  skip(true), loop(false)
{
  edge[0] = -1;
  edge[1] = -1;
  data[0] = -1;
  data[1] = -1;
  dir[0] = -1;
  dir[1] = -1;
}

Strip::Strip(bool _skip, int _edge0, int _edge1) :
  skip(_skip), loop(false)
{
  edge[0] = _edge0;
  edge[1] = _edge1;
  data[0] = -1;
  data[1] = -1;
  dir[0] = -1;
  dir[1] = -1;
}

//--------------------------------------------------------------------

// i is the last
void Strip::changeBack(Strip& s, int i)
{
  edge[1] = s.edge[i];
  data[1] = s.data[i];
  dir[1] = s.dir[i];
  block[1] = s.block[i];
}

//--------------------------------------------------------------------

//i is the first
void Strip::changeFront(Strip& s, int i)
{
  edge[0] = s.edge[i];
  data[0] = s.data[i];
  dir[0] = s.dir[i];
  block[0] = s.block[i];
}

} //namespace cms
