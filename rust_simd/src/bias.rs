//! SIMD code to perform the following C++ code on at least 4 f32s:
//! ```ignore
//! float MofoFilterAudioProcessor::curveConvert(float input, float shape)
//! {
//!    if (shape == 0.5f)
//!        return input;
//!    if (input == 0.f) return 0;
//!    if (input == 1.f) return 1;
//!    return (shape * input) / (1 - shape - input + (2 * input * shape));
//!}
//! ```

#[cfg(target_arch = "x86_64")]
use core::arch::x86_64::*;
#[cfg(target_arch = "x86")]
use core::arch::x86::*;

use crate::simd::Simd;

#[inline(always)]
fn bias_value(input: &f32, tension: &f32) -> f32 {
    //assert!(input >= &0.0 && input <= &1.0);
    //assert!(tension >= &0.0 && tension <= &1.0);

    match tension == &0.5f32 {
        true => *input,
        false => match input == &0.0 || input == &1.0 {
            true => *input,
            false => (tension * input) / (1.0 - tension - input + (2.0 * input * tension))
        }
    }
}

#[inline(always)]
pub fn bias_values<const N: usize>(input: f32, tensions: [f32; N]) -> [f32; N] {
    //if is_x86_feature_detected!("sse2") {
        //return unsafe { bias_values_sse2(inputs, tensions)}
    //} else {
        let mut result = [0f32; N];
        for (i, tens) in tensions.iter().enumerate() {
            result[i] = bias_value(&input, tens)
        }
        result
    //}
}

#[inline(always)]
pub unsafe fn bias_values_simd<S: Simd + Copy>(input: f32, tensions: [f32; S::WORDS]) -> S {
    let inputs_reg = S::set_all(input);
    //let tension_reg = S::loadu_ps(tensions.as_ptr());
    let tension_reg = S::set(tensions);

    let sub = S::sub(S::sub(S::set_all(1.0), tension_reg), inputs_reg);
    let numerator = S::mul(inputs_reg, tension_reg);
    let divisor = S::mult_add(S::set_all(2.0), numerator, sub);
    S::div(numerator, divisor)
}

#[inline]
#[target_feature(enable = "neon")]
#[cfg(target_arch = "aarch64")]
pub unsafe fn bias_values_neon(input: f32, tensions: [f32; 4]) -> core::arch::aarch64::float32x4_t {
    bias_values_simd::<core::arch::aarch64::float32x4_t>(input, tensions)
}

#[target_feature(enable = "avx,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn bias_values_simd_avx2(input: f32, tensions: [f32; 8]) -> __m256 {
    bias_values_simd::<__m256>(input, tensions)
}

#[target_feature(enable = "sse,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn bias_values_simd_sse2(input: f32, tensions: [f32; 4]) -> __m128 {
    bias_values_simd::<__m128>(input, tensions)
}

#[target_feature(enable = "sse,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn bias_values_sse2(input:f32, tensions: [f32; 4]) -> __m128 {
    let inputs_reg = _mm_set1_ps(input);
    let tension_reg = _mm_loadu_ps(tensions.as_ptr());

    // compute `(1.0 - tension - input)`
    let sub = _mm_sub_ps(_mm_sub_ps(_mm_set1_ps(1.0), tension_reg), inputs_reg);
    // compute `input * tension`
    let numerator = _mm_mul_ps(inputs_reg, tension_reg);
    // compute `sub + (2 * input * tension)`
    let divisor = _mm_fmadd_ps(_mm_set1_ps(2.0), numerator, sub);
    // then divide
    _mm_div_ps(numerator, divisor)
}

#[inline]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
pub unsafe fn m128_to_f32x4(i: &__m128) -> [f32; 4] {
    core::mem::transmute_copy(i)
}

#[inline]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
pub unsafe fn m256_to_f32x8(i: &__m256) -> [f32; 8] {
    core::mem::transmute_copy(i)
}

#[target_feature(enable = "avx,fma")]
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
#[inline]
pub unsafe fn bias_values_avx2(input: f32, tensions: [f32; 8]) -> __m256 {
    #[cfg(target_arch = "x86_64")]
    use core::arch::x86_64::*;
    #[cfg(target_arch = "x86")]
    use core::arch::x86::*;

    let inputs_reg = _mm256_set1_ps(input);
    let tension_reg = _mm256_loadu_ps(tensions.as_ptr());

    // compute `(1.0 - tension - input)`
    let sub = _mm256_sub_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), tension_reg), inputs_reg);
    // compute `input * tension`
    let numerator = _mm256_mul_ps(inputs_reg, tension_reg);
    // compute `sub + (2 * input * tension)`
    let divisor = _mm256_fmadd_ps(_mm256_set1_ps(2.0), numerator, sub);
    // then divide
    core::mem::transmute_copy(&_mm256_div_ps(numerator, divisor))
}

#[cfg(test)]
mod test {
    use super::*;

    #[target_feature(enable = "sse2")]
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    unsafe fn test(ints: [f32; 4], mult: [f32; 4]) -> [f32; 4] {
        #[cfg(target_arch = "x86_64")]
        use core::arch::x86_64::*;
        #[cfg(target_arch = "x86")]
        use core::arch::x86::*;

        let inputs_reg = _mm_loadu_ps(ints.as_ptr());
        let tension_reg = _mm_loadu_ps(mult.as_ptr());

        let result = _mm_mul_ps(inputs_reg, tension_reg);
        let r: [f32; 4] = core::mem::transmute_copy(&result);
        r
    }

    #[test]
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    fn test_multiplication() {
        let inputs = [0.5, 4.3, 2.98, 3.3];
        let mults = [1.344, 44.4, 33.3, 2.2];

        let mut expected_results = [0.0; 4];
        for (i, (inp, mult)) in inputs.iter().zip(mults.iter()).enumerate() {
            expected_results[i] = inp * mult;
        }
        unsafe { assert_eq!(expected_results, test(inputs, mults)); }
    }

    #[test]
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    fn test_bias_values_sse2() {
        let input = 0.33323521;
        //let tensions = [0.0, 1.0, 0.5, 0.875];
        let tensions = [0.111, 0.222, 0.777, 0.984];
        unsafe { assert_eq!(bias_values(input, tensions), m128_to_f32x4(&bias_values_sse2(input, tensions))); }
    }

    #[test]
    #[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
    fn test_bias_values_avx2() {
        let input = 0.83457;
        //let tensions = [0.0, 1.0, 0.5, 0.875];
        let tensions = [0.111, 0.222, 0.777, 0.984, 0.333, 0.333, 0.222, 0.556];
        unsafe { assert_eq!(bias_values(input, tensions), m256_to_f32x8(&bias_values_avx2(input, tensions))); }
    }

    #[test]
    fn test_bias_values_simd() {
        #[cfg(target_arch = "x86_64")]
        use core::arch::x86_64::{__m128, __m256};
        #[cfg(target_arch = "aarch64")]
        use core::arch::aarch64::float32x4_t;
        let input = 0.83457;
        let tensions_x4 = [0.0, 1.0, 0.5, 0.875];
        #[cfg(target_arch = "x86_64")]
        unsafe { 
            let tensions_x8 = [0.111, 0.222, 0.777, 0.984, 0.333, 0.333, 0.222, 0.556];

            assert_eq!(bias_values(input, tensions_x4), bias_values_simd::<__m128>(input, tensions_x4).to_array());
            assert_eq!(bias_values(input, tensions_x8), bias_values_simd::<__m256>(input, tensions_x8).to_array()); 
        }

        #[cfg(target_arch = "aarch64")]
        unsafe {
            assert_eq!(bias_values(input, tensions_x4), bias_values_simd::<float32x4_t>(input, tensions_x4).to_array());
        }
    }

    #[test]
    #[cfg(target_arch = "aarch64")]
    fn debug_aarch64() {
        unsafe {
            use core::arch::aarch64::float32x4_t as S;

            // test loadu_ps and to_array
            let start_nums = [1.5, 2.0, 1.734, 3.5555];
            let reg = S::loadu_ps(start_nums.as_ptr());
            assert_eq!(start_nums, S::to_array(&reg));

            // test add
            let n2 = [2.5, 3.5, 3.0, 4.5];
            let reg2 = S::loadu_ps(n2.as_ptr());
            let mut expected_vals = [0f32; 4];
            for i in 0..4 {
                expected_vals[i] = start_nums[i] + n2[i];
            }
            let add_result = S::add(reg, reg2);
            assert_eq!(add_result.to_array(), expected_vals);

            // test div
            for i in 0..4 {
                expected_vals[i] = start_nums[i] / n2[i];
            }
            let div_result = S::div(reg, reg2);
            assert_eq!(div_result.to_array(), expected_vals);

            // test set_all
            let reg3 = S::set_all(2.5);
            assert_eq!(reg3.to_array(), [2.5, 2.5, 2.5, 2.5]);

            // test sub
            for i in 0..4 {
                expected_vals[i] = start_nums[i] - n2[i];
            }
            let sub_result = S::sub(reg, reg2);
            assert_eq!(sub_result.to_array(), expected_vals);

            // test fmadd
            let add = [15.0, 16.0, 16.0, 15.0];
            for i in 0..4 {
                expected_vals[i] = (start_nums[i] * n2[i]) + add[i];
            }
            let fma_result = S::mult_add(reg, reg2, S::loadu_ps(add.as_ptr()));
            assert_eq!(expected_vals, fma_result.to_array());

            // test mul
            for i in 0..4 {
                expected_vals[i] = start_nums[i] * n2[i];
            }
            let mul_result = S::mul(reg, reg2);
            assert_eq!(mul_result.to_array(), expected_vals);
        }
    }
}