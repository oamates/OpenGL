#include <cstdint>

namespace cms
{

/*
  USING LEFT HANDED SYSTEM (... i know...)

    Data tables for marching squares


    Vertices have the following IDs
   0        2
    --------
    |      |
    |      |       ^-Z
    |      |       |
    --------       --> X
   1        3

    (divided by two from d[i] indices, since we're
     looking at a flat ASDF and we don't care about z)


    Edges are numbered as follows:
       0
    --------
    |      |
  2 |      | 3     ^-Z
    |      |       |
    --------       --> X
       1


*/

/*

  Vertex and Edge Index Map

        2-------0------6
       /.             /|
      10.           11 |
     /  2           /  3
    /   .          /   |     ^ Y
   3-------5------7    |     |
   |    0 . . 1 . |. . 4     --> X
   |   .          |   /
   6  8           7  9
   | .            | /
   |.             |/
   1-------4------5


      Face Index Map

          -----
          | 0 |
      -----------------     ^ -z
      | 5 | 2 | 4 | 3 |     |
      -----------------     ---> x
          | 1 |
          -----

      Face Index Layout
        o--------------o
       /.             /|
      / .            / |
     /  .    3      /  |
    /   .      (0) /   |     ^ Y
   o--------------o  5 |     |
   |(4) . . . . . |. . o     --> X
   |   .   1      |   /
   |  .           |  /
   | .      (2)   | /
   |.             |/
   o--------------o



  Cell Point and Subcell Layout

      (2)o--------------o(6)
        /.             /|
       / .            / |
      /  .           /  |
     /   .          /   |     ^ Y
 (3)o--------------o(7) |     |
    | (0). . . . . |. . o(4)  --> X
    |   .          |   /
    |  .           |  /
    | .            | /
    |.             |/
 (1)o--------------o(5)


 */

// For a given set of filled corners, this array defines
// the cell edges from which we draw interior edges
//
static const int8_t EDGE_MAP[16][2][2] = {
    {{-1, -1}, {-1, -1}}, // ----
    {{0, 2}, {-1, -1}},   // ---0
    {{2, 1}, {-1, -1}},   // --1-
    {{0, 1}, {-1, -1}},   // --10
    {{3, 0}, {-1, -1}},   // -2--
    {{3, 2}, {-1, -1}},   // -2-0
    {{3, 0}, { 2,  1}},   // -21- //ambig
    {{3, 1}, {-1, -1}},   // -210

    {{1, 3}, {-1, -1}},   // 3---
    {{1, 3}, { 0,  2}},   // 3--0 //ambig
    {{2, 3}, {-1, -1}},   // 3-1-
    {{0, 3}, {-1, -1}},   // 3-10
    {{1, 0}, {-1, -1}},   // 32--
    {{1, 2}, {-1, -1}},   // 32-0
    {{2, 0}, {-1, -1}},   // 321-
    {{-1,-1}, {-1, -1}}   // 3210
};


// Indexed by edge number, returns vertex index
//
static const int8_t VERTEX_MAP[4][2] = {
    {0, 2},
    {3, 1},
    {1, 0},
    {2, 3}
};

//----------------------------------------------


// FACE_VERTEX[i] gives the four vertices (as 3-bit corner indices)
// that define face i on an octree cell
//
 static const uint8_t FACE_VERTEX[6][4] = {
     {2, 0, 6, 4}, // face 0
     {1, 3, 5, 7}, // face 1
     {0, 1, 4, 5}, // face 2
     {6, 7, 2, 3}, // face 3
     {2, 3, 0, 1}, // face 4
     {4, 5, 6, 7}, // face 5
 };


/// Added by George Rassovsky
// Given an edge it gives back the two
// cell vertices that connect it
// @ at pos 0 and 1 - the order will always
// be in the positive direction...
//
static const int8_t EDGE_VERTICES[12][2] = {
  {2, 6}, // edge 0
  {0, 4}, // edge 1
  {0, 2}, // edge 2
  {4, 6}, // edge 3
  {1, 5}, // edge 4
  {3, 7}, // edge 5
  {1, 3}, // edge 6
  {5, 7}, // edge 7
  {0, 1}, // edge 8
  {4, 5}, // edge 9
  {2, 3}, // edge 10
  {6, 7}  // edge 11
};


} //namespace cms
