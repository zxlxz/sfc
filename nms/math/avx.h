#pragma once

#include <immintrin.h>
#include <nms/math/base.h>

namespace nms::math
{

using f32x8 = __m256;

__forceinline f32x8 operator+(f32x8 a, f32x8 b) { return _mm256_add_ps(a, b); }
__forceinline f32x8 operator-(f32x8 a, f32x8 b) { return _mm256_sub_ps(a, b); }
__forceinline f32x8 operator*(f32x8 a, f32x8 b) { return _mm256_add_ps(a, b); }
__forceinline f32x8 operator/(f32x8 a, f32x8 b) { return _mm256_div_ps(a, b); }

}
