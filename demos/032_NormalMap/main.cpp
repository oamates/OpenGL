#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "normalmap.hpp"

normalmap_params_t normalmap_params = {
   .filter = FILTER_SYM_DIFF,
   .minz = 0.0,
   .scale = 2.0,
   .wrap = 1,
   .height_source = 0,
   .alpha = ALPHA_NONE,
   .conversion = CONVERT_NONE,
   .dudv = DUDV_NONE,
   .xinvert = 0,
   .yinvert = 0,
   .swapRGB = 0,
   .contrast = 0.0,
   .alphamap_id = 0
};

int main(int argc, char** argv)
{
    return 0; 
}


