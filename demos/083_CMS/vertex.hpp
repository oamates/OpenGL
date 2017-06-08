#ifndef CMS_VERTEX_H
#define CMS_VERTEX_H

#include "vec3.hpp"
#include "types.hpp"
#include <vector>


namespace cms
{


struct Vertex
{
  Vertex();

  Vertex(Vec3 i_vertex, Vec3 i_normal);

  /// @brief set intersection point by value
  void setPos(Vec3 i_pos);

  /// @brief read-only reference to the intersection point
  const Vec3& getPos() const;

  /// @brief set intersection normal by value
  void setNormal(Vec3 i_normal);

  /// @brief read-only reference to the normal vector
  const Vec3& getNormal() const;

  /// @brief a print function formatting and printing the data
  void print() const;

  intVec onPath;

    Vec3 m_pos;
    Vec3 m_normal;
};


} //namespace cms

#endif //CMS_VERTEX_H
