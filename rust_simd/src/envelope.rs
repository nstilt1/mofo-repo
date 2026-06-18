//! SIMD code to perform the following C++ code on at least 4 f32s:
//! ```ignore
//! {
//!     [paramXRatio, paramYRatio, paramZRatio, paramXXRatio] = bias_values(ratio, [paramXTension, paramYTension, paramZTension, paramXXTension]);
//!     float minParamX = chainSettings.minParamX;
//!     float maxParamX = chainSettings.maxParamX;
//!     float upperBoundParamX = 32;
//!     if (maxParamX != 0)
//!     {
//!         if (minParamX + maxParamX > upperBoundParamX) maxParamX = upperBoundParamX;
//!         else maxParamX += minParamX;
//!         if (treeState.getParameterValue ("paramXIsUp")) minParamX = maxParamX - ((maxParamX - minParamX) * paramXRatio);
//!         else minParamX += (maxParamX - minParamX) * paramXRatio;
//!     }
//!     return minParamX;
//!}
//! ```

use crate::{bias::bias_values, simd::Simd};

#[cfg(target_arch = "x86_64")]
use crate::bias::{bias_values_sse2, m128_to_f32x4};

#[cfg(target_arch = "x86_64")]
use core::arch::x86_64::*;
#[cfg(target_arch = "x86")]
use core::arch::x86::*;

#[inline(always)]
pub fn compute_envelopes<const N: usize>(volume_ratio: f32, tensions: [f32; N], mins: [f32; N], maxes: [f32; N], bounds: [f32; N], directions: [f32; N]) -> [f32; N] {
    let ratios = bias_values(volume_ratio, tensions);

    let mut result = [0f32; N];
    for i in 0..N {
        let (min, mut max, bound, direction, ratio) = (mins[i], maxes[i], bounds[i], directions[i], ratios[i]);
        if max != 0.0 {
            if min + max > bound {
                max = bound;
            } else {
                max += min;
            }
            if direction == 1.0 {
                result[i] = max - ((max - min) * ratio)
            } else {
                result[i] = min + ((max - min) * ratio);
            }
        } else {
            result[i] = min;
        }
    }

    result
}

#[inline(always)]
pub unsafe fn compute_envelopes_simd<S: Simd + Copy>(volume_ratio: f32, tensions: [f32; S::WORDS], mins: [f32; S::WORDS], maxes: [f32; S::WORDS], bounds: [f32; S::WORDS], directions: [f32; S::WORDS]) -> [f32; S::WORDS] {
    let mut max_vals = S::set(maxes);
    let mins_reg = S::set(mins);
    //let mut max_vals = S::loadu_ps(maxes.as_ptr());
    //let mins_reg = S::loadu_ps(mins.as_ptr());

    max_vals = S::min(
        S::add(mins_reg, max_vals), 
        S::set(bounds)
        //S::loadu_ps(bounds.as_ptr())
    );

    let right_side = S::mul(
        S::sub(max_vals, mins_reg), 
        S::bias_values(volume_ratio, tensions)
    );

    let directions_reg = S::set(directions);
    //let directions_reg = S::loadu_ps(directions.as_ptr());
    
    let left_side = S::blend::<true>(directions_reg, 1.0, max_vals, mins_reg);
    
    //let right_sign = S::blend(S::loadu_ps(directions.as_ptr()), 1.0, S::set_all(-1.0), S::set_all(1.0));
    let right_sign = S::mul(directions_reg, S::set_all(-1.0));

    let result = S::mult_add(right_side, right_sign, left_side);
    result.to_array()
}

#[inline]
#[target_feature(enable = "neon")]
#[cfg(target_arch = "aarch64")]
pub unsafe fn compute_envelopes_neon(volume_ratio: f32, tensions: [f32; 4], mins: [f32; 4], maxes: [f32; 4], bounds: [f32; 4], directions: [f32; 4]) -> [f32; 4] {
    compute_envelopes_simd::<core::arch::aarch64::float32x4_t>(volume_ratio, tensions, mins, maxes, bounds, directions)
}

#[inline]
#[target_feature(enable = "avx,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
pub unsafe fn compute_envelopes_simd_avx2(volume_ratio: f32, tensions: [f32; 8], mins: [f32; 8], maxes: [f32; 8], bounds: [f32; 8], directions: [f32; 8]) -> [f32; 8] {
    compute_envelopes_simd::<__m256>(volume_ratio, tensions, mins, maxes, bounds, directions)
}

#[inline]
#[target_feature(enable = "sse,sse4.1,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
pub unsafe fn compute_envelopes_simd_sse2(volume_ratio: f32, tensions: [f32; 4], mins: [f32; 4], maxes: [f32; 4], bounds: [f32; 4], directions: [f32; 4]) -> [f32; 4] {
    compute_envelopes_simd::<__m128>(volume_ratio, tensions, mins, maxes, bounds, directions)
}


/// Computes envelopes given a volume ratio and some settings.
/// 
/// # Arguments
/// 
/// - `is_ups` - a f32 array that contains a list of `-1.0` or `1.0` values used for determining whether the envelope should travel up.
#[target_feature(enable = "sse,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn compute_envelopes_sse2(volume_ratio: f32, tensions: [f32; 4], mins: [f32; 4], maxes: [f32; 4], bounds: [f32; 4], directions: [f32; 4]) -> [f32; 4] {
    let ratios = bias_values_sse2(volume_ratio, tensions);
    let mut maxes_reg = _mm_loadu_ps(maxes.as_ptr());
    let mins_reg = _mm_loadu_ps(mins.as_ptr());
    let bounds_reg = _mm_loadu_ps(bounds.as_ptr());

    // restrain max
    let a_add_b = _mm_add_ps(mins_reg, maxes_reg);
    maxes_reg = _mm_min_ps(a_add_b, bounds_reg);

    // get a vector that contains either a positive or negative `(max - min) * ratio` depending on is_ups
    let mut right_side = _mm_sub_ps(maxes_reg, mins_reg);
    right_side = _mm_mul_ps(right_side, ratios);
    let right_sign = _mm_mul_ps(_mm_loadu_ps(directions.as_ptr()), _mm_set1_ps(-1.0));

    // set register to a sequence of `min` and `max`, depending on whether `directions` is 1.0 or not
    let masked_direction = _mm_cmp_ps::<_CMP_EQ_OS>(_mm_loadu_ps(directions.as_ptr()), _mm_set1_ps(1.0));
    
    let left_side = _mm_add_ps(
        _mm_and_ps(maxes_reg, masked_direction), 
        _mm_andnot_ps(masked_direction, mins_reg));

    let result = _mm_fmadd_ps(right_side, right_sign, left_side);

    m128_to_f32x4(&result)
}

/// Computes envelopes given a volume ratio and some settings.
/// 
/// # Arguments
/// 
/// - `is_ups` - a f32 array that contains a list of `-1.0` or `1.0` values used for determining whether the envelope should travel up.
#[target_feature(enable = "avx,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn compute_envelopes_avx2(volume_ratio: f32, tensions: [f32; 8], mins: [f32; 8], maxes: [f32; 8], bounds: [f32; 8], directions: [f32; 8]) -> [f32; 8] {
    use crate::bias::{bias_values_avx2, m256_to_f32x8};

    let ratios = bias_values_avx2(volume_ratio, tensions);
    let mut maxes_reg = _mm256_loadu_ps(maxes.as_ptr());
    let mins_reg = _mm256_loadu_ps(mins.as_ptr());
    let bounds_reg = _mm256_loadu_ps(bounds.as_ptr());

    // restrain max
    let a_add_b = _mm256_add_ps(mins_reg, maxes_reg);
    maxes_reg = _mm256_min_ps(a_add_b, bounds_reg);

    // get a vector that contains either a positive or negative `(max - min) * ratio` depending on is_ups
    let mut right_side = _mm256_sub_ps(maxes_reg, mins_reg);
    right_side = _mm256_mul_ps(right_side, ratios);
    let right_sign = _mm256_mul_ps(_mm256_loadu_ps(directions.as_ptr()), _mm256_set1_ps(-1.0));

    // set register to a sequence of `min` and `max`, depending on whether `directions` is 1.0 or not
    let masked_direction = _mm256_cmp_ps::<_CMP_EQ_OS>(_mm256_loadu_ps(directions.as_ptr()), _mm256_set1_ps(1.0));
    
    let left_side = _mm256_add_ps(
        _mm256_and_ps(maxes_reg, masked_direction), 
        _mm256_andnot_ps(masked_direction, mins_reg));

    let result = _mm256_fmadd_ps(right_side, right_sign, left_side);

    m256_to_f32x8(&result)
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    #[cfg(target_arch = "x86_64")]
    fn test_blend() {
        unsafe {
            let a = [1.0, 2.0, 2.5, 3.0];
            let b = [6.0, 2.0, 4.0, 1.5];
            let a_add_b = _mm_add_ps(_mm_loadu_ps(a.as_ptr()), _mm_loadu_ps(b.as_ptr()));
            let bounds = [4.0, 5.0, 5.5, 6.0];
            let bounds_reg = _mm_loadu_ps(bounds.as_ptr());
            let m = _mm_min_ps(a_add_b, bounds_reg);
            //let mask = _mm_cmp_ps_mask::<_CMP_GT_OS>(a_add_b, bounds_reg);

            //let blended = _mm_mask_blend_ps(mask, bounds_reg, a_add_b);
            //let blended = _mm_loadu_ps(a.as_ptr());

            let result = m128_to_f32x4(&m);
            for i in 0..4 {
                assert_eq!(result[i], f32::min(a[i] + b[i], bounds[i]))
            }
        }
    }

    #[test]
    #[cfg(target_arch = "x86_64")]
    fn test_masking() {
        unsafe {
            let a = [1.0, 1.5, 1.0, -1.0];

            let m = _mm_cmp_ps::<_CMP_EQ_OS>(_mm_loadu_ps(a.as_ptr()), _mm_set1_ps(1.0));

            let test_vals = [123.33, 14.1, 999.9, 11.05];

            assert_eq!([test_vals[0], 0.0, test_vals[2], 0.0], m128_to_f32x4(&_mm_and_ps(m, _mm_loadu_ps(test_vals.as_ptr()))));
        }
    }

    #[test]
    #[cfg(target_arch = "x86_64")]
    fn test_envelope_sse2() {
        let volume_ratio = 0.456f32;
        let tensions = [0.3333, 0.6666, 0.384473, 0.35668];
        let mins = [0.0, 1.0, 4.5, 8.0];
        let maxes = [0.0, 4.0, 4.5, 9.0];
        let bounds = [1.2, 4.5, 7.0, 20.0];
        let directions = [1.0, -1.0, 1.0, -1.0];

        let soft = compute_envelopes(volume_ratio, tensions, mins, maxes, bounds, directions);

        let sse2 = unsafe { compute_envelopes_sse2(volume_ratio, tensions, mins, maxes, bounds, directions) };

        assert_eq!(soft, sse2);

        let simd = unsafe { compute_envelopes_simd::<__m128>(volume_ratio, tensions, mins, maxes, bounds, directions)};
        assert_eq!(soft, simd);
    }

    #[test]
    #[cfg(target_arch = "x86_64")]
    fn test_envelope_avx2() {
        let volume_ratio = 0.456f32;
        let tensions = [0.0, 1.0, 0.384473, 0.35668, 0.8575, 0.12345, 0.21345, 0.5];
        let mins = [0.0, 1.0, 4.5, 8.0, 4.567, 0.234, 0.764, 0.11];
        let maxes = [0.0, 4.0, 4.5, 9.0, 25.0, 2.3, 5.3, 3.1];
        let bounds = [1.2, 4.5, 7.0, 20.0, 4.2, 3.3, 1.0, 3333.0];
        let directions = [1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0];

        let soft = compute_envelopes(volume_ratio, tensions, mins, maxes, bounds, directions);

        let sse2 = unsafe { compute_envelopes_avx2(volume_ratio, tensions, mins, maxes, bounds, directions) };

        assert_eq!(soft, sse2);

        let simd = unsafe {
            compute_envelopes_simd::<__m256>(volume_ratio, tensions, mins, maxes, bounds, directions)
        };
        assert_eq!(simd, soft);
    }

    #[test]
    fn test_envelope_simd() {
        #[cfg(target_arch = "x86_64")]
        use core::arch::x86_64::{__m128, __m256};
        #[cfg(target_arch = "aarch64")]
        use core::arch::aarch64::float32x4_t;

        let volume_ratio = 0.456f32;
        
        let tensions_x4 = [0.3333, 0.6666, 0.384473, 0.35668];
        let mins_x4 = [0.0, 1.0, 4.5, 8.0];
        let maxes_x4 = [0.0, 4.0, 4.5, 9.0];
        let bounds_x4 = [1.2, 4.5, 7.0, 20.0];
        let directions_x4 = [1.0, -1.0, 1.0, -1.0];

        #[cfg(target_arch = "x86_64")]
        unsafe { 
            let tensions_x8 = [0.0, 1.0, 0.384473, 0.35668, 0.8575, 0.12345, 0.21345, 0.5];
            let mins_x8 = [0.0, 1.0, 4.5, 8.0, 4.567, 0.234, 0.764, 0.11];
            let maxes_x8 = [0.0, 4.0, 4.5, 9.0, 25.0, 2.3, 5.3, 3.1];
            let bounds_x8 = [1.2, 4.5, 7.0, 20.0, 4.2, 3.3, 1.0, 3333.0];
            let directions_x8 = [1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0];
            assert_eq!(compute_envelopes(volume_ratio, tensions_x4, mins_x4, maxes_x4, bounds_x4, directions_x4), compute_envelopes_simd::<__m128>(volume_ratio, tensions_x4, mins_x4, maxes_x4, bounds_x4, directions_x4));
            assert_eq!(compute_envelopes(volume_ratio, tensions_x8, mins_x8, maxes_x8, bounds_x8, directions_x8), compute_envelopes_simd::<__m256>(volume_ratio, tensions_x8, mins_x8, maxes_x8, bounds_x8, directions_x8)); 
        }

        #[cfg(target_arch = "aarch64")]
        unsafe {
            assert_eq!(compute_envelopes(volume_ratio, tensions_x4, mins_x4, maxes_x4, bounds_x4, directions_x4), compute_envelopes_simd::<float32x4_t>(volume_ratio, tensions_x4, mins_x4, maxes_x4, bounds_x4, directions_x4));
        }
    }
}