macro_rules! use_simd {
    () => {
        #[cfg(target_arch = "x86_64")]
        use core::arch::x86_64::*;
        #[cfg(target_arch = "x86")]
        use core::arch::x86::*;
        #[cfg(target_arch = "aarch64")]
        use core::arch::aarch64::*;
    };
}

use_simd!();

use crate::bias::bias_values_simd;

pub trait Simd {
    const WORDS: usize;

    unsafe fn loadu_ps(ptr: *const f32) -> Self;
    unsafe fn set(vals: [f32; Self::WORDS]) -> Self;
    unsafe fn bias_values(volume_ratio: f32, tensions: [f32; Self::WORDS]) -> Self;
    unsafe fn add(a: Self, b: Self) -> Self;
    unsafe fn min(a: Self, b: Self) -> Self;
    unsafe fn sub(a: Self, b: Self) -> Self;
    unsafe fn mul(a: Self, b: Self) -> Self;
    unsafe fn div(a: Self, b: Self) -> Self;
    /// Sets all elements of the SIMD register to `v`
    unsafe fn set_all(v: f32) -> Self;
    /// Blends `b` and `c` based on the indices where `mask_value` is present in `a`.
    unsafe fn blend<const USE_SSE41: bool>(a: Self, mask_value: f32, b: Self, c: Self) -> Self;
    unsafe fn mult_add(a: Self, b: Self, c: Self) -> Self;
    unsafe fn to_array(&self) -> [f32; Self::WORDS];
}
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
impl Simd for __m128 {
    const WORDS: usize = 4;

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn loadu_ps(ptr: *const f32) -> Self {
        _mm_loadu_ps(ptr)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn set(vals: [f32; Self::WORDS]) -> Self {
        _mm_setr_ps(vals[0], vals[1], vals[2], vals[3])
    }

    #[inline]
    #[target_feature(enable = "sse,fma")]
    unsafe fn bias_values(volume_ratio: f32, tensions: [f32; Self::WORDS]) -> Self {
        bias_values_simd::<Self>(volume_ratio, tensions)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn add(a: Self, b: Self) -> Self {
        _mm_add_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "sse,sse4.1")]
    unsafe fn blend<const USE_SSE41: bool>(a: Self, mask_value: f32, b: Self, c: Self) -> Self {
        let mask = _mm_cmp_ps::<_CMP_EQ_OS>(a, Self::set_all(mask_value));
        
        if USE_SSE41 {
            _mm_blendv_ps(c, b, mask)
        } else {
            _mm_add_ps(
                _mm_and_ps(mask, b),
                _mm_andnot_ps(mask, c)
            )
        }
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn min(a: Self, b: Self) -> Self {
        _mm_min_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn sub(a: Self, b: Self) -> Self {
        _mm_sub_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn mul(a: Self, b: Self) -> Self {
        _mm_mul_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn div(a: Self, b: Self) -> Self {
        _mm_div_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "sse")]
    unsafe fn set_all(v: f32) -> Self {
        _mm_set1_ps(v)
    }

    #[inline]
    #[target_feature(enable = "sse,fma")]
    unsafe fn mult_add(a: Self, b: Self, c: Self) -> Self {
        _mm_fmadd_ps(a, b, c)
    }

    #[inline(always)]
    unsafe fn to_array(&self) -> [f32; Self::WORDS] {
        let result: [f32; 4] = core::mem::transmute_copy(self);
        result
    }
}
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
impl Simd for core::arch::x86_64::__m256 {
    const WORDS: usize = 8;

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn loadu_ps(ptr: *const f32) -> Self {
        _mm256_loadu_ps(ptr)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn set(vals: [f32; Self::WORDS]) -> Self {
        _mm256_setr_ps(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7])
    }

    #[inline]
    #[target_feature(enable = "avx,fma")]
    unsafe fn bias_values(volume_ratio: f32, tensions: [f32; Self::WORDS]) -> Self {
        bias_values_simd::<Self>(volume_ratio, tensions)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn add(a: Self, b: Self) -> Self {
        _mm256_add_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn blend<const USE_SSE41: bool>(a: Self, mask_value: f32, b: Self, c: Self) -> Self {
        let mask = _mm256_cmp_ps::<_CMP_EQ_OS>(a, Self::set_all(mask_value));
        _mm256_blendv_ps(c, b, mask)
        // _mm256_add_ps(
        //     _mm256_and_ps(mask, b),
        //     _mm256_andnot_ps(mask, c)
        // )
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn min(a: Self, b: Self) -> Self {
        _mm256_min_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn sub(a: Self, b: Self) -> Self {
        _mm256_sub_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn mul(a: Self, b: Self) -> Self {
        _mm256_mul_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn div(a: Self, b: Self) -> Self {
        _mm256_div_ps(a, b)
    }

    #[inline]
    #[target_feature(enable = "avx")]
    unsafe fn set_all(v: f32) -> Self {
        _mm256_set1_ps(v)
    }

    #[inline]
    #[target_feature(enable = "fma")]
    unsafe fn mult_add(a: Self, b: Self, c: Self) -> Self {
        //_mm256_add_ps(c, _mm256_mul_ps(a, b))
        _mm256_fmadd_ps(a, b, c)
    }

    #[inline(always)]
    unsafe fn to_array(&self) -> [f32; Self::WORDS] {
        let result: [f32; 8] = core::mem::transmute_copy(self);
        result
    }
}


#[cfg(target_arch = "aarch64")]
use core::arch::aarch64::*;
use std::is_x86_feature_detected;

#[cfg(target_arch = "aarch64")]
impl Simd for float32x4_t {
    const WORDS: usize = 4;

    #[inline(always)]
    unsafe fn add(a: Self, b: Self) -> Self {
        vaddq_f32(a, b)
    }

    #[inline(always)]
    unsafe fn blend<const _USE_SSE41: bool>(a: Self, mask_value: f32, b: Self, c: Self) -> Self {
        let mask = vceqq_f32(a, Self::set_all(mask_value));
        vbslq_f32(mask, b, c)
    }

    #[inline(always)]
    unsafe fn bias_values(volume_ratio: f32, tensions: [f32; Self::WORDS]) -> Self {
        bias_values_simd::<Self>(volume_ratio, tensions)
    }

    #[inline(always)]
    unsafe fn div(a: Self, b: Self) -> Self {
        vdivq_f32(a, b)
    }

    #[inline(always)]
    unsafe fn loadu_ps(ptr: *const f32) -> Self {
        vld1q_f32(ptr)
    }

    #[inline]
    unsafe fn set(vals: [f32; Self::WORDS]) -> Self {
        vld1q_f32(vals.as_ptr())
    }

    #[inline(always)]
    unsafe fn min(a: Self, b: Self) -> Self {
        vminnmq_f32(a, b)
    }

    #[inline(always)]
    unsafe fn mul(a: Self, b: Self) -> Self {
        vmulq_f32(a, b)
    }

    #[inline(always)]
    unsafe fn mult_add(a: Self, b: Self, c: Self) -> Self {
        vfmaq_f32(c, a, b)
    }

    #[inline(always)]
    unsafe fn set_all(v: f32) -> Self {
        vdupq_n_f32(v)
    }

    #[inline(always)]
    unsafe fn sub(a: Self, b: Self) -> Self {
        vsubq_f32(a, b)
    }

    #[inline(always)]
    unsafe fn to_array(&self) -> [f32; 4] {
        let result: [f32; 4] = core::mem::transmute_copy(self);
        result
    }
}