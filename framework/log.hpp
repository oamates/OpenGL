#ifndef _logger_included_124900971247253743634896758972312399234333009756612344
#define _logger_included_124900971247253743634896758972312399234333009756612344

#include <cstdlib>

//=======================================================================================================================================================================================================================
// simple multithreaded lock-free file logger
//=======================================================================================================================================================================================================================
extern void impl_gl_error_msg(const char* file_name, const char* function_name, int line);
extern void impl_debug_msg(const char* format, ...);
extern void impl_put_msg(const char* msg);

static constexpr const char * const short_path(const char * const str, const char * const last_slash)
{
    return *str == '\0' ? last_slash :
         ((*str == '/') || (*str == '\\')) ? short_path(str + 1, str + 1) :
                                             short_path(str + 1, last_slash);
}

static constexpr const char * const short_path(const char * const str) 
{ 
    return short_path(str, str);
}

#define DEBUG_DEFAULT_COLOR "\033[0m"
#define DEBUG_RED_COLOR "\033[0;31m"
#define DEBUG_GREEN_COLOR "\033[0;32m"
#define DEBUG_ORANGE_COLOR "\033[0;33m"

//=======================================================================================================================================================================================================================
// the interface :)
//=======================================================================================================================================================================================================================

#define put_msg(str) impl_put_msg(str)

#if defined(_MSC_VER)
    #define gl_error_msg(...)                   impl_gl_error_msg(short_path(__FILE__), __FUNCTION__, __LINE__)
    #define debug_msg(str,...)                  impl_debug_msg("%s : %s : %d : " str "\n", short_path(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define debug_color_msg(color_code,str,...) impl_debug_msg(color_code "%s : %s : %d : " str "\n\033[0m", short_path(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define exit_msg(str,...)                  {impl_debug_msg("%s : %s : %d ERROR : " str "\n", short_path(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__); exit(1);}
#else
    #define gl_error_msg(...)                   impl_gl_error_msg(short_path(__FILE__), __func__, __LINE__)
    #define debug_msg(str,...)                  impl_debug_msg("%s : %s : %d : " str "\n", short_path(__FILE__), __func__, __LINE__, ##__VA_ARGS__)
    #define debug_color_msg(color_code,str,...) impl_debug_msg(color_code "%s : %s : %d : " str "\n\033[0m", short_path(__FILE__), __func__, __LINE__, ##__VA_ARGS__)
    #define exit_msg(str,...)                  {impl_debug_msg("%s : %s : %d ERROR : " str "\n", short_path(__FILE__), __func__, __LINE__, ##__VA_ARGS__); exit(1);}
#endif

#endif // _logger_included_124900971247253743634896758972312399234333009756612344