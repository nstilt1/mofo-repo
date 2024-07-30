/*
  ==============================================================================

    SIMDLadderFilter.h
    Created: 18 Apr 2024 11:04:07am
    Author:  Noah Stiltner

  ==============================================================================
*/

#pragma once
class SIMDLadderFiler  : public juce::LadderFilter
{
public:
    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();
        
        jassert (inputBlock.getNumChannels() <= getNumChannels());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);
        
        if (! enabled || context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }
        
        for (size_t n = 0; n < numSamples; ++n)
        {
            updateSmoothers();
            
            for (size_t ch = 0; ch < numChannels; ++ch)
                outputBlock.getChannelPointer (ch)[n] = processSample (inputBlock.getChannelPointer (ch)[n], ch);
        }
    }
};
