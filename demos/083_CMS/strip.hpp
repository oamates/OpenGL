#ifndef CMS_STRIP_H
#define CMS_STRIP_H

#include "index3d.hpp"


namespace cms
{


/// @struct Strip
/// @brief Holding indices to two vertices
/// and two face edges
///
struct Strip{

  /// @brief Empty Constructor
  /// Sets everything to 'false' and -1, resulting in an empty strip
          Strip();

  /// @brief Edge Constructor
  /// @param the skip boolean - set it to true if there is valid data on the strip
  /// @param edge0 and edge1 are the two edges on which the strip lies
          Strip( bool _skip  ,
                 int  _edge0 ,
                 int  _edge1 );

  // i is the last
  void    changeBack( Strip& s ,
                      int    i );

  //i is the first
  void    changeFront( Strip& s,
                       int    i );

  bool    skip;
  bool    loop;
  int     edge[2]; // local cell edge indices
  int     data[2]; // global datastruct edge indices

  Index3D block[2]; //3D Index of the EdgeBlock into the global datastruct
  int     dir[2]; //Direction of the edge on the edge block
};


} //namespace cms

#endif //CMS_STRIP_H
