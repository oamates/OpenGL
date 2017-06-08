#ifndef _cms_util_included_8012562851023857612356120357620571692057628375681237
#define _cms_util_included_8012562851023857612356120357620571692057628375681237

#include <cstdio>
#include <string>

namespace cms
{

namespace util
{
    
    inline int intPower(int i_num, int i_ind)                                                      // A replacement for the stl pow function, working with int
    {
        int result = i_num;

        if(i_ind == 0) return 1;

        for(int i = 0; i < i_ind - 1; ++i)
            result *= i_num;

        return result;
    }
    
    inline bool isPowerOfTwo (unsigned int x)                                                      // Checking if the given number is a power of two
        { return ((x != 0) && !(x & (x - 1))); }

    
    inline void printTime(int totalSeconds, const char* message)                                   // print the exact time in a formatted way hh:mm:ss with a message in the front
    {
        int ss = totalSeconds % 60;
        int mm = (totalSeconds / 60) % 60;
        int hh = (totalSeconds / 60) / 60;

        printf("\n%s \n%02i:%02i:%02i [hh:mm:ss] \n\n", message, hh, mm, ss);
    }
  
} // namespace util

} // namespace cms

#endif // _cms_util_included_8012562851023857612356120357620571692057628375681237
