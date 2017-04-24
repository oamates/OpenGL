#include "noise_simd.hpp"

// DISABLE WHOLE PROGRAM OPTIMIZATION for this file when using MSVC

// Depending on the compiler this file may need to have SSE4.1 code generation compiler flags enabled
#ifdef FN_COMPILE_SSE41
#define SIMD_LEVEL_H FN_SSE41
#include "internal.hpp"
#include <smmintrin.h> //SSE4.1

#define SIMD_LEVEL FN_SSE41
#include "internal.cpp"
#endif