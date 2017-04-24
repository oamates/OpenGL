#include <cstdio>

#include "utils.hpp"
#include "log.hpp"

namespace utils {

char* file_read(const char* file_name)
{
    FILE* input = fopen(file_name, "rb");
    if (!input) 
    {
        debug_msg("File %s failed to open.", file_name);
        return 0;
    };
    if (fseek(input, 0, SEEK_END) == -1) 
    {
        debug_msg("End of the file %s not found.", file_name);
        return 0;
    };
    long int size = ftell(input);
    if (size == -1) 
    {
        debug_msg("File %s is empty.", file_name);
        return 0;
    };

    if (fseek(input, 0, SEEK_SET) == -1) 
    {
        debug_msg("File %s reading error.", file_name);
        return 0;
    };
    
    char* content = (char*) malloc ((size_t) size + 2);
    fread(content, 1, (size_t)size, input);
    if (ferror(input)) 
    {
        debug_msg("File %s reading error.", file_name);
        free(content);
        return 0;
    }
    fclose(input);
    content[size] = '\n';
    content[size + 1] = '\0';
    return content;
}

namespace timer {

#if defined(__linux)
    #define HAVE_POSIX_TIMER
    #include <time.h>
    #ifdef CLOCK_MONOTONIC
        #define CLOCKID CLOCK_MONOTONIC
    #else
        #define CLOCKID CLOCK_REALTIME
    #endif
#elif defined(__APPLE__)
    #define HAVE_MACH_TIMER
    #include <mach/mach_time.h>
#elif defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    # include <windows.h>
#endif

uint64_t ns()
{
    static bool initialized = false;
#if defined(__APPLE__)
    static mach_timebase_info_data_t info;
    if (!initialized)
    {
        mach_timebase_info(&info);
        initialized = true;
    }
    uint64_t now;
    now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;
    return now;
#elif defined(__linux)
    static struct timespec linux_rate;
    if (!initialized)
    {
        clock_getres(CLOCKID, &linux_rate);
        initialized = true;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
    return now;
#elif defined(_WIN32)
    static LARGE_INTEGER win_frequency;
    if (!initialized)
    {
        QueryPerformanceFrequency(&win_frequency);
        initialized = true;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t) ((1e9 * now.QuadPart)  / win_frequency.QuadPart);
#endif
} 

} // namespace timer

} // namespace utils