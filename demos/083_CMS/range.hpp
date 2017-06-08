#ifndef CMS_RANGE_H
#define CMS_RANGE_H

#include <cassert>

namespace cms
{


/// @struct Range
/// @brief  A 1D datastructure holding 2 floating point numbers
struct Range{

        Range(){}


        Range( float i_lower ,float i_upper ) :
          m_lower( i_lower ),
          m_upper( i_upper )
        {
          assert( m_lower <= m_upper );
        }


  void  operator()( float i_lower , float i_upper )
  {
    m_lower = i_lower;
    m_upper = i_upper;
  }


  float m_lower;
  float m_upper;
};


} //namespace cms

#endif //CMS_RANGE_H
