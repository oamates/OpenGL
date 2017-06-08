#ifndef CMS_INDEX_H
#define CMS_INDEX_H

#include <iostream>

namespace cms
{

/// @struct Index3D
/// @brief serving as a vec3 for integer values
struct Index3D
{
public:
           Index3D();
           
           Index3D( const int _i );
           
           Index3D( const Index3D& _ind );
           
           Index3D( const int _x,
                    const int _y,
                    const int _z );

  int&     operator[]( const int& _i );

  Index3D& operator=( const Index3D& i_ind );

  Index3D  operator-( const int& _i );

  Index3D  operator-( const Index3D& _ind );

  Index3D  operator+( const int& _i );

  Index3D  operator+( const Index3D& _ind );

  void     operator-=( const int& _i );

  void     operator+=( const int& _i );

  bool     operator==( const Index3D &_ind ) const;

  bool     operator!=( const Index3D &_ind ) const;
  
  // Member variables
  int      m_x;
  int      m_y;
  int      m_z;

};

  /// @brief Overwritting the steaming operator so that a Index3D could be
  /// printed with correct formatting
  std::ostream& operator<<(std::ostream& _output, const Index3D& _i);

} //namespace cms

#endif //CMS_INDEX_H
