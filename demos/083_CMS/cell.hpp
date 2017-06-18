#ifndef _cms_cell_included_9810568431078235067138560347562107836458974356893746
#define _cms_cell_included_9810568431078235067138560347562107836458974356893746

#include <vector>
#include <cstdint>
#include "address.hpp"
#include "vertex.hpp"
#include "range.hpp"
#include "vec3.hpp"
#include "point.hpp"
#include "index3d.hpp"
#include "face.hpp"
#include "util.hpp"

namespace cms
{

enum CellState 
{ 
    BRANCH  = 0,                            // BRANCH may also mean EMPTY
    LEAF    = 1
};

struct Cell
{
    typedef std::vector<int8_t> int8Vec;


    //============== Public Members ================

    Cell* m_neighbours[6];
    int m_id;
    Range m_x, m_y, m_z;                                                                        // the public dimensions of the cell through ranges in xyz
    int8Vec m_rawAddress;                                                                       // The address of the cell
    Address m_address;

    enum CellState m_state;                                                                     // The state of the current cell - based on State enumerator
    Cell* m_parent;                                                                             // A pointer to this cell's parent cell
    uint8_t m_subdivLvl;                                                                        // The level of subdivision of the cell

  
    uint8_t m_maxSubdiv;                                                                        // The maximum subdivision of the octree (needed for Address so pass it down)
    Index3D m_c000;                                                                             // the index of the point at the 000 corner of the cell
    Index3D m_offsets;                                                                          // the discrete dimensions of the cell based on the samples
    float m_width, m_height, m_depth;                                                           // the 3D space dimensions of the cell in cartesian coords
    Index3D m_pointInds[8];                                                                     // Indices of the samples at the corners of the cell
    Vec3 m_centre;                                                                              // The geometric centre of the cell as a 3d vector
    Cell* m_children[8];                                                                        // a ptr array of the branching cells from the current cell
    Face* m_faces[6];                                                                           // The Array of cell faces
    int8_t m_posInParent;                                                                       // Position within parent

    // The array of components for this cell
    std::vector<std::vector<unsigned int>> m_components;



    //============== Constructor(s) and Destructor =============
    
    // Full Constructor
    // int i_id the cell's unique identifier
    // CellState i_state - the cell state - (enumerator)
    // Cell* i_parent - a pointer to the parent cell (null if root)
    // uint i_subdivLvl - the level of subdivision
    // Index3D i_c000 - the 000 corner of the cell in the samples array
    // Index3D i_offset - the dimensions of the cell in sample slabs xyz

    Cell(int _id,
         CellState i_state,
         Cell* i_parent,
         uint8_t i_subdivLvl,
         Index3D i_c000,
         Index3D i_offsets,
         int8_t i_posInPar) :
        m_id(_id), m_state(i_state), m_parent(i_parent), m_subdivLvl(i_subdivLvl),
        m_c000(i_c000), m_offsets(i_offsets), m_posInParent(i_posInPar)
    {
        m_centre = Vec3(0.0f, 0.0f, 0.0f);

        // Initialising the 8 octree children to NULL
        for(int i = 0; i < 8; ++i)
            m_children[i] = 0;

        // Generating the 6 cell faces / HEAP /
        for(int i = 0; i < 6; ++i)
            m_faces[i] = new Face(i, m_id);

        // Initialise the address
        if(m_parent)
            m_address.set(m_parent->m_address.getRaw(), m_posInParent + 1);
        else
            m_address.reset();

        // Clear the neighbour array
        for(int i = 0; i < 6; ++i)
            m_neighbours[i] = 0;
    }

    // destructor :: looping and destroying all children individually
    ~Cell()
    {
        // Delete the six faces of the cell
        for(int i = 0; i < 6; ++i)
        {
            if(m_faces[i])
                delete m_faces[i];
        }
    }

    //============== Accessors and Mutators =============
    
    void setState(CellState i_state)                                                                // Set enumerator cell state
        { m_state = i_state; }
    
    const CellState& getState() const                                                               // Get the cell state enumerator
        { return m_state; }
    
    void setParent(Cell* i_parent)                                                                  // Setting the pointer to the parent cell
        { m_parent = i_parent; }
    
    Cell* getParent() const                                                                         // Getting a pointer to the parent cell
        { return m_parent; }

    void setSubdivLvl(unsigned int i_subdivLvl)                                                     // Setting the level of subdivision of the cell
        { m_subdivLvl = i_subdivLvl; }

    const uint8_t& getSubdivLvl() const                                                             // getting a read-only reference (unsigned int) of the cell depth
        { return m_subdivLvl; }

    void setC000(Index3D i_c000)                                                                    // Setting the corner 000 3d index
        { m_c000 = i_c000; }
    
    const Index3D& getC000() const                                                                  // Getting a read-only 3d index of the corner at 000
        { return m_c000; }

    void setOffsets(Index3D i_offsets)                                                              // Setting the xyz offset (dimensions) of the cell in discrete samples
        { m_offsets = i_offsets; }
  
    const Index3D&   getOffsets() const                                                             // Getting the xyz offset sample (dimensions) of the cell
        { return m_offsets; }
  
    void setPointInds(Index3D i_pointInds[])                                                        // Setting the 3D Index of the corner points of the cell
    {
        for(int i = 0; i < 8; ++i)
            m_pointInds[i] = i_pointInds[i];
    }
  
    const Index3D* getPointInds() const                                                             // Getting the array of indices to corner points of a cell
        { return m_pointInds; }
  
    void setCentre(Vec3 i_centre)                                                                   // Setting the centre of a cell in 3D space
        { m_centre = i_centre; }
  
    const Vec3& getCentre() const                                                                   // Getting a read-only reference to the centre of the cell
        { return m_centre; }
  
    void pushChild(Cell* child, const unsigned int index)                                           // Add child cell point onto the children array, providing a ptr to a child cell and the index at which it should be set
        { m_children[index] = child; }
  
    Cell* getChild( const unsigned int index ) const                                                // Getting a child cell ptr from the specified index
        { return m_children[index]; }                                                               
                                                                                                    
    void setWidth(float i_width)                                                                    // Setting the width of the cell in 3D space
        { m_width = i_width; }                                                                      
                                                                                                    
    const float& getWidth() const                                                                   // Getting the width of the cell in 3D space
        { return m_width; }                                                                         
                                                                                                    
    void setHeight(float i_height)                                                                  // Setting the height of the cell in 3D space
        { m_height = i_height; }                                                                    
                                                                                                    
    const float& getHeight() const                                                                  // Getting the height of the cell in 3D space
        { return m_height; }                                                                        
                                                                                                    
    void setDepth(float i_depth)                                                                    // Setting the depth of the cell in 3D space
        { m_depth = i_depth; }                                                                      
                                                                                                    
    const float& getDepth() const                                                                   // Getting the depth of the cell in 3D space
        { return m_depth; }                                                                         
                                                                                                    
    void pushComponent(std::vector<unsigned int> i_comp)                                            // push_back component onto the component vector of the cell
        { m_components.push_back(i_comp); }                                                         
                                                                                                    
    std::vector<std::vector<unsigned int>> getComponents()                                          // Retrieve the component array of the cell :: todo fix with offset
        { return m_components; }                                                                    
                                                                                                    
    Face* getFaceAt(int position)                                                                   // Get a ptr to a cell face at the specified position
        { return m_faces[position]; }                                                               
                                                                                                    
    int8_t getPosInParent()                                                                         // Return a int8_t of the position of the cell in relation to it's parent
        { return m_posInParent; }

};

} // namespace cms

#endif // _cms_cell_included_9810568431078235067138560347562107836458974356893746
