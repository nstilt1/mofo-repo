//use std::arch::x86_64::{__m128, __m256};

use criterion::{black_box, criterion_group, criterion_main, Criterion};

#[cfg(target_arch = "aarch64")]
use simd_tests::{bias::bias_values_neon, envelope::compute_envelopes_neon};
#[cfg(any(target_arch = "x86", target_arch = "x86_64"))]
use simd_tests::{bias::{bias_values_simd_sse2, bias_values_simd_avx2}, envelope::{compute_envelopes_simd_sse2, compute_envelopes_simd_avx2}};

use simd_tests::{bias::bias_values, envelope::{compute_envelopes, compute_envelopes_simd}};

pub fn no_simd(c: &mut Criterion) {
    let input = 0.7547;
    let tensions = [0.342, 0.234, 0.122, 0.94373];
    c.bench_function("bias_soft_x4", |b| b.iter(|| black_box(bias_values(input, tensions))));

    let volume_ratio = 0.456f32;
    let tensions = [0.3333, 0.6666, 0.384473, 0.35668];
    let mins = [0.0, 1.0, 4.5, 8.0];
    let maxes = [0.0, 4.0, 4.5, 9.0];
    let bounds = [1.2, 4.5, 7.0, 20.0];
    let directions = [1.0, -1.0, 1.0, -1.0];
    c.bench_function("compute_envelope_soft_x4", |b| b.iter(|| black_box(compute_envelopes(volume_ratio, tensions, mins, maxes, bounds, directions))));

    #[cfg(target_arch = "x86_64")]
    {
        let tensions = [0.342, 0.234, 0.122, 0.94373, 0.33455, 0.33, 0.874, 0.2345];
        c.bench_function("bias_soft_x8", |b| b.iter(|| black_box(bias_values(input, tensions))));

        let volume_ratio = 0.456f32;
        let tensions = [0.3333, 0.6666, 0.384473, 0.35668, 0.334343, 0.6455, 0.33535, 0.1111];
        let mins = [0.0, 1.0, 4.5, 8.0, 0.5, 12.0, 11.0, 1.0];
        let maxes = [0.0, 4.0, 4.5, 9.0, 5.0, 12.0, 11.0, 15.0];
        let bounds = [1.2, 4.5, 7.0, 20.0, 0.5, 15.0, 11.0, 10.0];
        let directions = [1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0];
        c.bench_function("compute_envelope_soft_x8", |b| b.iter(|| black_box(compute_envelopes(volume_ratio, tensions, mins, maxes, bounds, directions))));
    }
}

pub fn with_simd_trait(c: &mut Criterion) {
    let input = 0.7547;
    let tensions = [0.342, 0.234, 0.122, 0.94373];

    let volume_ratio = 0.456f32;
    let mins = [0.0, 1.0, 4.5, 8.0];
    let maxes = [0.0, 4.0, 4.5, 9.0];
    let bounds = [1.2, 4.5, 7.0, 20.0];
    let directions = [1.0, -1.0, 1.0, -1.0];

    #[cfg(target_arch = "aarch64")]
    {
        c.bench_function("bias_neon_x4", |b| b.iter(|| unsafe { black_box(bias_values_neon(input, tensions))}));

        c.bench_function("compute_envelope_neon_x4", |b| b.iter(|| unsafe {black_box(compute_envelopes_neon(volume_ratio, tensions, mins, maxes, bounds, directions))}));
    }
}

pub fn old_simd(c: &mut Criterion) {
    use simd_tests::{
        bias::*,
        envelope::*
    };
    #[cfg(target_arch = "x86_64")]
    {
        use core::arch::x86_64::*;
        let input = 0.7547;
        let tensions = [0.342, 0.234, 0.122, 0.94373];
        c.bench_function("bias_traitless_sse2_x4", |b| b.iter(|| unsafe {black_box(bias_values_sse2(input, tensions)) }));
        c.bench_function("bias_trait_sse2_x4", |b| b.iter(|| unsafe {black_box(bias_values_simd_sse2(input, tensions))}));

        let tensions = [0.342, 0.234, 0.122, 0.94373, 0.33455, 0.33, 0.874, 0.2345];
        c.bench_function("bias_traitless_avx2", |b| b.iter(|| unsafe {black_box(bias_values_avx2(input, tensions)) }));
        c.bench_function("bias_trait_avx2_x4", |b| b.iter(|| unsafe {black_box(bias_values_simd_avx2(input, tensions))}));


        let volume_ratio = 0.456f32;
        let tensions = [0.3333, 0.6666, 0.384473, 0.35668];
        let mins = [0.0, 1.0, 4.5, 8.0];
        let maxes = [0.0, 4.0, 4.5, 9.0];
        let bounds = [1.2, 4.5, 7.0, 20.0];
        let directions = [1.0, -1.0, 1.0, -1.0];
        c.bench_function("compute_envelope_traitless_sse2_x4", |b| b.iter(|| black_box(unsafe {compute_envelopes_sse2(volume_ratio, tensions, mins, maxes, bounds, directions)})));
        c.bench_function("compute_envelope_trait_sse2_x4", |b| b.iter(|| { unsafe { black_box(compute_envelopes_simd_sse2(volume_ratio, tensions, mins, maxes, bounds, directions))}}));

        let volume_ratio = 0.456f32;
        let tensions = [0.3333, 0.6666, 0.384473, 0.35668, 0.334343, 0.6455, 0.33535, 0.1111];
        let mins = [0.0, 1.0, 4.5, 8.0, 0.5, 12.0, 11.0, 1.0];
        let maxes = [0.0, 4.0, 4.5, 9.0, 5.0, 12.0, 11.0, 15.0];
        let bounds = [1.2, 4.5, 7.0, 20.0, 0.5, 15.0, 11.0, 10.0];
        let directions = [1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0];
        c.bench_function("compute_envelope_traitless_avx2_x8", |b| b.iter(|| black_box(unsafe {compute_envelopes_avx2(volume_ratio, tensions, mins, maxes, bounds, directions)})));
        c.bench_function("compute_envelope_trait_avx2_x8", |b| b.iter(|| { unsafe { black_box(compute_envelopes_simd_avx2(volume_ratio, tensions, mins, maxes, bounds, directions))}}));
    }
}

criterion_group!(bench_v1, no_simd);
criterion_group!(bench_v3, with_simd_trait);
criterion_group!(bench_v2, old_simd);

criterion_main!(bench_v1, bench_v3, bench_v2);