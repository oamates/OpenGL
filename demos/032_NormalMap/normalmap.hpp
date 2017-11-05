#ifndef _normal_map_included_23816987561298375023874512038745123785129835412937
#define _normal_map_included_23816987561298375023874512038745123785129835412937

#include "filter_kernel.hpp"

enum FILTER_TYPE
{
    FILTER_SOBEL_3x3, FILTER_SOBEL_5x5,
    FILTER_PREWITT_3x3, FILTER_PREWITT_5x5,
    FILTER_3x3, FILTER_5x5, FILTER_7x7, FILTER_9x9,
    FILTER_SYM_DIFF,
    MAX_FILTER_TYPE
};

static const char* FILTER_TYPE_NAMES[MAX_FILTER_TYPE] = 
{
    "FILTER_SOBEL_3x3",
    "FILTER_SOBEL_5x5",
    "FILTER_PREWITT_3x3",
    "FILTER_PREWITT_5x5",
    "FILTER_3x3",
    "FILTER_5x5",
    "FILTER_7x7",
    "FILTER_9x9",
    "FILTER_SYM_DIFF"
};

enum ALPHA_TYPE
{
    ALPHA_NONE, ALPHA_HEIGHT, ALPHA_INVERSE_HEIGHT, ALPHA_ZERO, ALPHA_ONE, ALPHA_INVERT,
    MAX_ALPHA_TYPE
};

enum CONVERSION_TYPE
{
    CONVERT_NONE, CONVERT_BIASED_RGB, CONVERT_RED, CONVERT_GREEN, CONVERT_BLUE, CONVERT_MAX_RGB, CONVERT_MIN_RGB, 
    CONVERT_COLORSPACE, CONVERT_NORMALIZE_ONLY, CONVERT_DUDV_TO_NORMAL, CONVERT_HEIGHTMAP,
    MAX_CONVERSION_TYPE
};

enum DUDV_TYPE
{
    DUDV_NONE, 
    DUDV_8BIT_SIGNED, DUDV_8BIT_UNSIGNED,
    DUDV_16BIT_SIGNED, DUDV_16BIT_UNSIGNED,
    MAX_DUDV_TYPE
};

struct normalmap_params_t
{
    int filter;
    double minz;
    double scale;
    int wrap;
    int height_source;
    int alpha;
    int conversion;
    int dudv;
    int xinvert;
    int yinvert;
    double contrast;
};

template<typename real_t> void normalmap(unsigned char* rgba_src, unsigned char* rgba_dst, int width, int height, int bpp, const normalmap_params_t& normalmap_params);

#endif // _normal_map_included_23816987561298375023874512038745123785129835412937
