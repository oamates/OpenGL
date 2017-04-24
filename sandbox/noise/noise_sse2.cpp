#include "noise_simd.hpp"

// DISABLE WHOLE PROGRAM OPTIMIZATION for this file when using MSVC

// Depending on the compiler this file may need to have SSE2 code generation compiler flags enabled
#ifdef FN_COMPILE_SSE2
#define SIMD_LEVEL_H FN_SSE2
#include "internal.hpp"
#include <emmintrin.h> //SSE2

#define SIMD_LEVEL FN_SSE2
#include "internal.cpp"
#endif