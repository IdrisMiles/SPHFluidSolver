#include <cuda_runtime.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <device_functions.h>
#include <math_constants.h>

#include <math.h>
#include <float.h>


__device__ float3 operator+(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}

__device__ float3 operator-(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
}

__device__ float3 operator*(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x*rhs.x, lhs.y*rhs.y, lhs.z*rhs.z);
}

__device__ float3 operator*(const float3 lhs, const float rhs)
{
    return make_float3(lhs.x*rhs, lhs.y*rhs, lhs.z*rhs);
}

__device__ float3 operator*(const float lhs, const float3 rhs)
{
    return make_float3(lhs*rhs.x, lhs*rhs.y, lhs*rhs.z);
}

__device__ float3 operator/(const float3 lhs, const float3 rhs)
{
    return make_float3(lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z);
}

__device__ float3 operator/(const float3 lhs, const float rhs)
{
    return make_float3(lhs.x/rhs, lhs.y/rhs, lhs.z/rhs);
}

__device__ float dot(const float3 lhs, const float3 rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

__device__ float length(const float3 vec)
{
    return sqrtf(dot(vec,vec));
}

__device__ float length2(const float3 vec)
{
    return dot(vec,vec);
}

__device__ float magnitude(const float3 vec)
{
    return sqrtf(dot(vec,vec));
}


__device__ float3 normalize(const float3 vec)
{
    float mag = magnitude(vec);
    if(fabs(mag) < FLT_EPSILON)
    {
        return vec;
    }
    else
    {
        return vec / mag;
    }
}
