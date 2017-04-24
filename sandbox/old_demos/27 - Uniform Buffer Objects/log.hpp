#ifndef _logger_included_124900971247253743634896758972312399234333009756612344
#define _logger_included_124900971247253743634896758972312399234333009756612344

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#if defined(_MSC_VER)
#define debug_msg(str,...) log_write("%s : %s : %d : " str "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define exit_msg(str,...) {log_write("%s : %s : %d ERROR : " str "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); exit(1);}
#elif defined(__MINGW32__)
#define debug_msg(str,...) log_write("%s : %s : %d : " str "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define exit_msg(str,...) {log_write("%s : %s : %d ERROR : " str "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__); exit(1);}
#endif

#define ASSERT(x)          assert(x)

void log_write(const char *format, ...);
void gl_info();

#endif // _logger_included_124900971247253743634896758972312399234333009756612344
