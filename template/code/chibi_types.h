#ifndef _CHIBI_TYPES_H_
#define _CHIBI_TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <float.h>
#include <stdbool.h>
#include <stdarg.h>

#define UNIX_BUILD

#if defined(WIN64_BUILD)
#  define PLATFORM_WINDOWS 1
#  define PLATFORM_UNIX    0
#elif defined(UNIX_BUILD)
#  define PLATFORM_WINDOWS 0
#  define PLATFORM_UNIX    1
#else
#  error Unsupported platform
#endif

#if 0
typedef char8_t   c8;
typedef char16_t  c16;
typedef char32_t  c32;
#endif

typedef uint8_t   byte;
typedef uint16_t  word;
typedef uint32_t  dword;

typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef uint8_t    b8;
typedef uint16_t   b16;
typedef uint32_t   b32;
typedef uint64_t   b64;

typedef float     f32;
typedef double    f64;

typedef uintptr_t uptr;
typedef intptr_t  sptr;

typedef union
{
    struct { s64 Upper, Lower; };
    s64 Bits[2];
} u128;

#define U8_MAX   UINT8_MAX
#define U16_MAX  UINT16_MAX
#define U32_MAX  UINT32_MAX
#define U64_MAX  UINT64_MAX
#define I8_MAX   INT8_MAX
#define I16_MAX  INT16_MAX
#define I32_MAX  INT32_MAX
#define I64_MAX  INT64_MAX
#define F32_MIN -FLT_MAX
#define F32_MAX  FLT_MAX
#define F64_MIN -DBL_MAX
#define F64_MAX  DBL_MAX

#define fn_internal   static
#define fn_inline     static inline

#if defined(_MSC_VER)
#define EXPORT __declspec( dllexport )
#define IMPORT __declspec( dllimport )
#elif defined(__GNUC__)
#define EXPORT __attribute__((visibility("default"))) 
#define IMPORT 
#else 
#error Unsupported
#endif

#define fn_imported  IMPORT
#define fn_exported  EXPORT

#define var_persist   static
#define var_global    static

#define _KB(x) ((x) * 1024llu)
#define _MB(x) (_KB(x) * 1024llu)
#define _GB(x) (_MB(x) * 1024llu)

#define _64KB  _KB(64)
#define _1MB   _MB(1)
#define _2MB   _MB(2)
#define _4MB   _MB(4)
#define _8MB   _MB(8)
#define _16MB  _MB(16)
#define _32MB  _MB(32)
#define _64MB  _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)
#define _1GB   _GB(1)

// A hack to determine if platform is little endian
// TODO(enlynn): Do something that isn't so hacky.
#define LITTLE_ENDIAN_TEST 0x41424344UL 
#define ENDIAN_ORDER  ('ABCD') 
#define IS_LITTLE_ENDIAN (ENDIAN_ORDER==LITTLE_ENDIAN_TEST)

#define forward_align(Base, Alignment) (((u64)(Base) + (u64)(Alignment) - 1) & ~((u64)(Alignment) - 1))
// I doubt this is the most efficient way of doing things, but it is easy to understand and does not run the risk of an underflow
#define BackwardAlign(Base, Alignment) (ForwardAlign(((Base) + 1), Alignment) - (Alignment))
#define DivideAlign(val, align) (((val) + (align) - 1) / (align))
#define DivideCeil(Numerator, Denominator) (Numerator > 0) ? (1 + ((Numerator - 1) / Denominator)) : (Numerator / Denominator)

#define ArrayCount(array)    (sizeof(array) / sizeof(array[0]))
#define Clamp(Val, Min, Max) (((Val) < (Min)) ? (Min) : (((Val) > (Max)) ? (Max) : (Val)))

#define ForRange(Type, VarName, Count)        for (Type VarName = 0;           VarName < (Count); ++VarName)
#define ForRangeReverse(Type, VarName, Count) for (Type VarName = (Count) - 1; VarName >= 0;      --VarName)

fn_inline bool 
u128_compare(u128 Left, u128 Right)
{
    return Left.Lower == Right.Lower && Left.Upper == Right.Upper;
}

// TODO(enlynn): make these inline functions again

// source: https://hbfs.wordpress.com/2008/08/05/branchless-equivalents-of-simple-functions/
fn_inline int 
sign_extend(int x)
{
    union
    {
        s64 w;
        struct { s32 lo, hi; } _p;
    } z;
    z.w = x;
    return z._p.hi;
}

s32 fast_sign_s32(s32 v);
s32 fast_sign_s64(s64 v);

int fast_abs(int x);

fn_inline int 
fast_max(int min, int max)
{
    int result;
    result = min + ((max - min) & ~sign_extend(max - min));
    return result;
}

fn_inline int 
fast_min(int min, int max)
{
    int result;
    result = max + ((min - max) & sign_extend(min - max));
    return result;
}

fn_inline int 
fast_clamp(int Value, int Min, int Max)
{
    return fast_min(Max, fast_max(Value, Min));
}

/**
    * Round up to the next highest power of 2.
    * @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    */
fn_inline u32
next_highest_pow_2_u32(u32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/**
* Round up to the next highest power of 2.
* @source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
*/
fn_inline u64
next_highest_pow_2_u64(u64 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;
    return v;
}

#include <time.h>
fn_inline f64 clock_ms(clock_t Start, clock_t End) {
    return ((f64)(End - Start) / (f64)CLOCKS_PER_SEC) * 1000.0;
}

#endif //_TYPES_H_
