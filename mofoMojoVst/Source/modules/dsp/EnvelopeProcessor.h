/*
  ==============================================================================

    EnvelopeProcessor.h
    Created: 13 Apr 2024 10:46:32pm
    Author:  Somedooby

  ==============================================================================
*/

#pragma once

/**
 * @brief Performs a unit bias calculation.
 * 
 * Based on Signalsmith's unit bias formula:
 * https://www.desmos.com/calculator/97t87kp8cq
 * 
 * @param volumeRatio The ratio of current volume over max volume.
 * @param tension The tension knob's value for biasing the envelope.
 * @return float The bias
 */
float bias(float volumeRatio, float tension) {
    if (tension == 0.5f)
        return volumeRatio;
    if (volumeRatio == 0.f || volumeRatio == 1.f) return volumeRatio;
    return (tension * volumeRatio) / (1.f - tension - volumeRatio + (2.f * volumeRatio * tension));
}

/**
 * @brief Calculates an envelope for a parameter.
 *
 * @param volumeRatio The ratio of current volume over max volume.
 * @param tension The tension knob's value for biasing the envelope.
 * @param baseValue The lower base value for the parameter we are going to make an envelope for.
 * @param envelopeAmount The user's chosen upper bound for the envelope. This value will be clamped.
 * @param upperBound The maximum value for the envelope's range that we will clamp the envelopeAmount to.
 * @param sweepDirection Whether the envelope should sweep up or down. 1 = sweep up, 0 = sweep down.
 * - note that the direction is only valid for instruments/samples with a decaying volume
 * @return float The envelope for a parameter.
 */
float envelope(
    float volumeRatio, 
    float tension, 
    float baseValue, 
    float envelopeAmount,
    float upperBound, 
    int sweepDirection
) {
    if (envelopeAmount == baseValue) return baseValue;

    float envelopeBias = bias(volumeRatio, tension);

    // clamp the envelopeAmount if it is above the maximum after adding the baseValue
    if (envelopeAmount > upperBound) envelopeAmount = upperBound;

    float biasedDifference = (envelopeAmount - baseValue) * envelopeBias;

    if (sweepDirection)
        return envelopeAmount - biasedDifference;
    return baseValue + biasedDifference;
}