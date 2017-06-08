#ifndef CMS_ISOSURFACE_H
#define CMS_ISOSURFACE_H

namespace cms
{


/// @class Isosurface.h
/// @brief The base Isosurface class
/// It cannot be instantiated directly, as it has to be extended via Isosurface_t
///
/// The class structure is taken from ism_2_0 library
/// Mathieu Sanchez (2014). ism_2_0.
///
class Isosurface
{
public:
  /// @brief Precision set to float by default
  typedef float Real;

public:

  /// @brief The pure virtual overloaded operator used for sampling the function
  /// implementation is in the template derived class Isosurface_t
  virtual Real operator()(Real x, Real y, Real z) const = 0;

  /// @brief Set the isolevel of the surface
  void setIsolevel(Real i_isoLevel);

  /// @brief Get a read-only reference to the isolevel value
  const Real& getIsolevel() const;

  /// @brief sets the signs of the mesh based on the boolean value supplied
  /// if true - negative inside and positive outside; if false - vice-versa
  void setNegativeInside(bool _negInside);

  /// @brief Returns a boolean of whether the sign is negative inside the surface
  /// If returns false - It is positive inside and negative outside
  bool isNegativeInside() const;


protected:
  /// @brief The default construtor - setting isolevel value to 0
  /// class cannot be instantiated directly as this is a protected ctor
  Isosurface();

  /// @brief The virtual destructor of the the Isosurface
  virtual ~Isosurface();

private:
  /// @brief The iso-level (iso-value) of the surface (zero by def)
  Real m_isoLevel;

  /// @brief A variable defining the inside (and therefore outside)
  /// signs of the isosurface. Allowing for changes to be made, to the functions signs
  /// negative inside by default
  bool m_negativeInside;

  bool loaded; //DEBUG
};


} //namespace cms

#endif //CMS_ISOSURFACE_H
