#ifndef _cms_edge_included_9143684378567835672065123560781235687346587431658714
#define _cms_edge_included_9143684378567835672065123560781235687346587431658714

#include "vec3.hpp"
#include "index3d.hpp"

namespace cms
{

// Edge structure storing vertex index

struct Edge
{
    Index3D m_block;                                            // a 3D index of the edge block to which that edge belongs
    int m_dir;                                                  // the direction this edge is facing on the edgeblock :: 0 - x, 1 - y, 2 - z
    bool m_empty;                                               // the empty boolean flag
    int m_vertIndex;                                            // the vertex index stored on this edge if applicable

    Edge()
        : m_dir(-1), m_empty(true), m_vertIndex(-1)
    {}

    ~Edge() {}
  
    void setVertIndex(int i_vertIndex)                          // Setting a vertex index on the edge
        { m_vertIndex = i_vertIndex; }
    
    const int& getVertIndex() const                             // Getting a read-only vertex index from the edge
        { return m_vertIndex; }
  
    void setEmpty(bool i_empty)                                 // setting the bool flag, for occupied edge
        { m_empty = i_empty; }


  
    const bool& empty() const                                   // a bool flag denoting whether the edge has data
        { return m_empty; }

 
};

// structure EdgeBlock holding 3 edges with ordering x y z - right up front

struct EdgeBlock
{
    bool m_empty;                                               // the empty bool flag
    Index3D m_edgeInds;                                         // the 3d edgeblock index in the edge array

    EdgeBlock()
        : m_empty(true), m_edgeInds(-1)
    {}

    EdgeBlock(bool empty)
        : m_empty(empty), m_edgeInds(-1)
    {}

    EdgeBlock(int right, int up, int front)
        : m_empty(false), m_edgeInds(right, up, front)
    {}

    EdgeBlock(bool empty, int right, int up, int front)
        : m_empty(empty), m_edgeInds(right, up, front)
    {}
  
    int& operator[] (const int& _i)                             // Overloaded [] operator
        { return (&m_edgeInds.m_x)[_i]; }

};

} // namespace cms

#endif // _cms_edge_included_9143684378567835672065123560781235687346587431658714