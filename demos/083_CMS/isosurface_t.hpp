#ifndef CMS_ISOSURFACE_T_H
#define CMS_ISOSURFACE_T_H

#include "isosurface.hpp"


namespace cms
{

/// @class Isosurface_t.h
/// @brief The templated isosurface class, which should be used to extend Isosurface
///
/// The class structure is taken from ism_2_0 library
/// Mathieu Sanchez (2014). ism_2_0.
///
template<typename Function>
class Isosurface_t : public Isosurface
{
public:
  /// @brief The default construtor - sets function to NULL (should be set)
  Isosurface_t() : m_fn(0) {}

  /// @brief The constructor taking in a Function
  Isosurface_t(const Function* i_fn) : m_fn(i_fn) {}

  /// @brief The overloaded operator used for sampling the function
  Real operator()(Real x, Real y, Real z) const { return (*m_fn)(x, y, z); }

  /// @brief Sets the member function to the one supplied
  void setFunction(const Function* i_fn) { m_fn = i_fn; }

  /// @brief Returns the base function
  Real getFuntion() const { return m_fn; }

protected:
  const Function* m_fn;
};

/// @todo extend to Isosurface_t_n ~ for template meshing with normals

} //namespace cms

#endif //CMS_ISOSURFACE_T_H

