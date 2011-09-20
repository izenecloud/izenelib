#ifndef IZENELIB_UTIL_MATH_H_
#define IZENELIB_UTIL_MATH_H_

#include <types.h>

NS_IZENELIB_UTIL_BEGIN

//////////////////////////////////////////////////////////////////////////////
///http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
///Note:Recommended GCC flags:  -O3 -finline-functions -ffast-math
///Performance comparison references:
///fastlog2 relative accuracy = 2.09352e-05
///fastlog relative accuracy = 2.09348e-05
///fasterlog2 relative accuracy = 0.0130367
///fasterlog relative accuracy = 0.0130367
///fastlog2 million calls per second = 160.141
///fastlog million calls per second = 143.552
///fasterlog2 million calls per second = 218.345
///fasterlog million calls per second = 210.435
///log2f million calls per second = 40.8511

///fastpow2 relative accuracy (positive p) = 1.58868e-05
///fastexp relative accuracy (positive p) = 1.60712e-05
///fasterpow2 relative accuracy (positive p) = 0.0152579
///fasterexp relative accuracy (positive p) = 0.0152574
///fastpow2 relative accuracy (inverse root p) = 1.43517e-05
///fastexp relative accuracy (inverse root p) = 1.7255e-05
///fasterpow2 relative accuracy (inverse root p) = 0.013501
///fasterexp relative accuracy (inverse root p) = 0.0111832
///fastpow2 million calls per second = 153.561
///fastexp million calls per second = 143.311
///fasterpow2 million calls per second = 215.006
///fasterexp million calls per second = 214.44
///expf million calls per second = 4.16527

static inline float fastlog2 (float x)
{
    union { float f;
        uint32_t i;
    } vx = { x };
    union { uint32_t i;
        float f;
    } mx = { (vx.i & 0x007FFFFF) | (0x7e << 23) };
    float y = vx.i;
    y *= 1.0f / (1 << 23);

    return y - 124.22544637f - 1.498030302f * mx.f - 1.72587999f / (0.3520887068f + mx.f);
}

static inline float fastlog (float x)
{
    return 0.69314718f * fastlog2 (x);
}

static inline float fastlog10 (float x)
{
    return 0.301029823f * fastlog2 (x);
}

static inline float fasterlog2 (float x)
{
    union { float f; uint32_t i; } vx = { x };
    float y = vx.i;
    y *= 1.0 / (1 << 23);
    return y - 126.94269504f;
}
 
static inline float fasterlog (float x)
{
    return 0.69314718f * fasterlog2 (x);
}

static inline float fasterlog10 (float x)
{
    return 0.301029823f * fasterlog2 (x);
}

static inline float fastpow2 (float p)
{
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = (int)clipp;
    float z = clipp - w + offset;
    union { uint32_t i;
        float f;
    } v = { (uint32_t)((1 << 23) * (clipp + 121.2740838f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z)) };

    return v.f;
}

static inline float fastexp (float p)
{
    return fastpow2 (1.442695040f * p);
}

static inline float fastpow (float x, float p)
{
    return fastpow2 (p * fastlog2 (x));
}

static inline float fasterpow2 (float p)
{
    union { uint32_t i; float f; } v = { (1 << 23) * (p + 126.94269504f) };
    return v.f;
}
 
static inline float fasterexp (float p)
{
   return fasterpow2 (1.442695040f * p);
}

//////////////////////////////////////////////////////////////
/// y=pow(x,-(1/p)) where p>=1
///Fast inverse square root 
static inline float fasterinvproot (float x, float p)
{
    union { float f; uint32_t i; } v = { x };
    unsigned int R = 0x5F375A84U;   // R = (3/2) (B - sigma) L
    unsigned int pR = (unsigned int)(((2 * (p + 1)) / (3 * p)) * R);
    unsigned int sub = (unsigned int)(v.i / p);
    v.i = pR - sub;
    return v.f;
}

static inline float fastinvproot (float x, float p)
{
    float y = fasterinvproot (x, p);
    float log2x = fastlog2 (x);
    float log2y = fastlog2 (y);
    float err = 1.0 - 0.34657359f * (log2x / p + log2y);
    return y * err * err;
}

//////////////////////////////////////////////////////////////
///See http://en.wikipedia.org/wiki/Gamma_function
static inline float fastlgamma (float x)
{
    float logterm = fastlog (x * (1.0f + x) * (2.0f + x));
    float xp3 = 3.0f + x;
 
    return -2.081061466f - x + 0.0833333f / xp3 - logterm + (2.5f + x) * fastlog (xp3);
}
 
 //////////////////////////////////////////////////////////////
 ///See http://en.wikipedia.org/wiki/Digamma_function
static inline float fastdigamma (float x)
{
    float twopx = 2.0f + x;
    float logterm = fastlog (twopx);
 
    return - (1.0f + 2.0f * x) / (x * (1.0f + x))
           - (13.0f + 6.0f * x) / (12.0f * twopx * twopx)
           + logterm;
}

NS_IZENELIB_UTIL_END

#endif

