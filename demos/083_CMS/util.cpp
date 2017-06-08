#include <cstdio>

#include "util.hpp"

namespace cms
{

int util::intPower(int i_num, int i_ind)
{
  int result = i_num;

  if(i_ind == 0)
    return 1;

  for(int i=0;i<i_ind-1;++i)
    result *= i_num;

  return result;
}

//-----------------------------------------------

bool util::isPowerOfTwo (uint x)
{
  return ((x != 0) && !(x & (x - 1)));
}

//-----------------------------------------------

void util::printTime(int totalSeconds, const char* message)
{
  int ss = totalSeconds % 60;
  int mm = (totalSeconds / 60) % 60;
  int hh = (totalSeconds / 60) / 60;

  printf("\n%s \n%02i:%02i:%02i [hh:mm:ss] \n\n", message, hh, mm, ss);
}

} //namespace cms
