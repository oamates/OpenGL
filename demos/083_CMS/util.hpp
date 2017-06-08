#ifndef CMS_UTIL_H
#define CMS_UTIL_H

#include <string>

#include "types.hpp"

namespace cms
{
namespace util
{

  /// @brief A replacement for the stl pow function, working with int
  /// @todo bit-shift powerintPower()
  int intPower(int i_num, int i_ind);


  /// @brief Checking if the given number is a power of two
  /// idea from: http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
  bool isPowerOfTwo (uint x);


  /// @brief print the exact time in a formatted way hh:mm:ss
  /// with a message in the front
  /// @param a integer value of the total number of seconds
  /// @param the message that has to be printed before the time
  void printTime(int totalSeconds, const char* message);


} //namespace util
} //namespace cms


#endif //CMS_UTIL_H
