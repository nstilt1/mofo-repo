/*
  ==============================================================================

    maths.h
    Created: 20 Apr 2024 7:12:24pm
    Author:  somed

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

using Register = juce::dsp::SIMDRegister<float>;

// defining division for 128-bit registers
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
inline Register operator / (const Register& l, const Register& r)
{
    return _mm_div_ps(l.value, r.value);
}

#elif defined(_M_ARM64) || defined (__arm64__) || defined (__aarch64__)
inline Register operator / (const Register& l, const Register& r)
{
    return vdivq_f32(l.value, r.value);
}

#else
inline Register operator / (const Register& l, const Register& r)
{
    float elementsL[Register::size()];
    float elementsR[Register::size()];
    l.store(elementsL);
    r.store(elementsR);

    for (int i = 0; i < Register::size(); ++i) {
        elementsL[i] /= elementsR[i];
    }

    result.load(elementsL);
    return Register::fromRawArray(elementsL);
}
#endif

/** Provides a fast approximation of the function tanh(x) using a Pade approximant
    continued fraction, based on `juce_FastMathApproximations.h`

    The original `juce_LadderFilter.h` file used a Lookup Table of `std::tanh`, but 
    I guess we'll see how approximation sounds.
*/
static Register SIMDtanh(Register x) noexcept
{
    auto x2 = x * x;
    auto n1 = Register(39251520);
    auto n2 = Register(18471600);
    auto n3 = Register(1075032);
    auto n4 = Register(14615);

    auto d2 = Register(1154160);
    auto d3 = Register(-16632);
    auto d4 = Register(127);

    auto negative1 = Register(-1.0);
    auto numerator = negative1 * (Register::multiplyAdd(
        n1, // added to product of:
        x2,
        Register::multiplyAdd(
            n2, // added to product of:
            x2,
            Register::multiplyAdd(
                n3, // added to product of:
                n4,
                x2
            )
        )
    ));

    auto denominator = Register::multiplyAdd(
        negative1 * n1, // added to the product of:
        x2,
        Register::multiplyAdd(
            d2, // added to the product of
            x2,
            Register::multiplyAdd(
                d3, // added to the product of:
                x2,
                d4
            )
        )
    );
    
    return numerator / denominator;
}