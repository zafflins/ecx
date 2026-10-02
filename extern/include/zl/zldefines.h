#pragma once

#include <stdlib.h>

typedef void* ptr;
typedef void none;

typedef float   f32;
typedef double  f64;

typedef signed char           i8;
typedef signed short          i16;
typedef signed int            i32;
typedef signed long long      i64;

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;

typedef enum Result {
    ZL_FATAL =   -1,
    ZL_ERROR =    0,
    ZL_OK =  1
} Result;

#ifdef KB
    #undef KB
#endif
#ifdef MB
    #undef MB
#endif
#ifdef GB
    #undef GB
#endif
#define KB 1000
#define MB (KB * 1000)
#define GB (MB * 1000)

#ifdef KiB
    #undef KiB
#endif
#ifdef MiB
    #undef MiB
#endif
#ifdef GiB
    #undef GiB
#endif
#define KiB 1024
#define MiB (KiB * 1024)
#define GiB (MiB * 1024)

#define PI 3.14159265358979323846
#define RADIANS(d) d * (PI / 180)
#define DEGREES(r) r * (180 / PI)

#define ZL_INVALID_HANDLE UINT64_MAX

#define FPTR(r, n, ...) r (*n)(__VA_ARGS__)

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define MIN3(a, b, c) MIN(a, MIN(b, c))

#define MAX4(a, b, c, d) MAX(a, MAX3(b, c, d))
#define MIN4(a, b, c, d) MIN(a, MIN3(b, c, d))

#define FLIP_BIT(v, b)  (*v ^= (1 << b))
#define SET_BITS(v, b)  (*v |= (1 << b))
#define REM_BITS(v, b)  (*v &= ~(1 << b))
#define GET_BITS(v, b)  ((v & (1 << b)) == (1 << b))

#define MOD(value, limit)   (value % limit)
#define MODF(value, limit)  (fmod((f64)value, limit))
#define SWAP(t, a, b)       ({ t tmp = a; a = b; b = tmp; })
#define CLAMP(v, l, h)      ((v > h) ? h : (v < l) ? l : v)
#define CLAMPT(t, v, l, h)  (((t)v > (t)h) ? (t)h : ((t)v < (t)l) ? (t)l : (t)v)

#define FOR_I(start, stop, step) for (u32 i = start; i < stop; i += step)
#define FOR_J(start, stop, step) for (u32 j = start; j < stop; j += step)
#define FOR_K(start, stop, step) for (u32 k = start; k < stop; k += step)
#define FOR_L(start, stop, step) for (u32 l = start; l < stop; l += step)
#define FOR_U(start, stop, step) for (u32 u = start; u < stop; u += step)
#define FOR_V(start, stop, step) for (u32 v = start; v < stop; v += step)
#define FOR_X(start, stop, step) for (u32 x = start; x < stop; x += step)
#define FOR_Y(start, stop, step) for (u32 y = start; y < stop; y += step)
#define FOR(type, iter, start, stop, step) for (type iter = start; iter < stop; iter += step)


static inline i64 Sumi(u32 n, i32* arr) { i64 s = 0; FOR_I(0, n, 1) s += arr[i]; return s; }
static inline u64 Sumu(u32 n, u32* arr) { u64 s = 0; FOR_I(0, n, 1) s += arr[i]; return s; }
static inline f64 Sumf(u32 n, f32* arr) { f64 s = 0; FOR_I(0, n, 1) s += arr[i]; return s; }


#ifndef NULL
    #ifndef _WIN64
        #define NULL 0
        #define ZL_ALIGN 4
    #else
        #define NULL 0LL
        #define ZL_ALIGN 8
    #endif  /* _WIN64 */
    #else
        #define ZL_ALIGN 8
        #define NULL ((void *)0)
#endif

#ifdef ZL_BUILD_DLL
    #ifdef _MSC_VER
        #define ZL_API __declspec(dllexport)
    #elif defined (__GNUC__) || defined (__clang__)
        #define ZL_API __attribute__((visibility("default")))
    #else
        #define ZL_API
    #endif
#else
    #ifdef _MSC_VER
        #define ZL_API __declspec(dllimport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define ZL_API __attribute__((visibility("default")))
    #else
        #define ZL_API
    #endif
#endif
