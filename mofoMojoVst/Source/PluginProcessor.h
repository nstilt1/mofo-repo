/*
  ==============================================================================

    PluginLicensorStatus.h
    Created: 27 Dec 2022 12:17:07pm
    Author:  Somedooby

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <math.h>
#include "SIMDLadderFilter.h"

struct ParamTree
{
    juce::ValueTree node;
};

/**
 * @struct ChainSettings
 * @brief Settings for audio processing chain.
 *
 * @var ChainSettings::drive
 * Initial drive level.
 * @var ChainSettings::cutoff
 * Initial cutoff frequency setting.
 * @var ChainSettings::minCutoff
 * Minimum allowed cutoff frequency.
 * ...
 * @var ChainSettings::is2Pole
 * Toggle for 2-pole filter mode (0 for off, 1 for on).
 * ...
 */
struct ChainSettings 
{
    // The drive level
    float drive{ 0 };
    // The classic cutoff frequency
    float cutoff{ 0 };
    // the auto cutoff knob value n, where n needs to be multiplied with the detected
    // pitch to obtain a frequency
    float minCutoff{ 0 };
    // the resonance knob's value between 0.0 and 10.0; the Ladder Filter will require
    // this value to be normalized, between 0 and 1.
    float resonance{ 0 };
    // the output volume knob
    float volume{ 0 };
    // the mix ratio
    float mix{ 0 };
    // the cutoff amount knob, measured in +0.5 octaves
    float cutoffAmount{ 0 };
    // the resonance amount knob
    float resonanceAmount{ 0 };
    // the cutoff envelope tension
    float cutoffTension{ 0 };
    // the resonance envelope tension
    float resonanceTension{ 0 };
    // the drive envelope tension
    float driveTension{ 0 };
    // the tension for the envelope that affects the cutoff amount
    float speedTension{ 0 };
    // the envelope amount for the envelope that affects the cutoff envelope amount
    float speedAmount{ 0 };
    // the envelope amount for the drive's envelope
    float driveAmount{ 0 };
    int is2Pole{ 0 };
    int isHighPass{ 0 };
    // whether the "Auto" mode is selected
    int isAuto{ 0 };
    // direction of the cutoff envelope
    int freqIsUp{ 0 };
    // direction of the resonance envelope
    int resIsUp{ 0 };
    int driveIsUp{ 0 };
    int speedIsUp{ 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& treeState);

class MofoFilterAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    MofoFilterAudioProcessor();
    ~MofoFilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;


    //==============================================================================
    /*
     For converting from a ratio to a curve for use in an envelope or LFO.
     */
    float curveConvert(float input, float shape);
    void pushNextSampleIntoFifo(float sample) noexcept;
    float getFFTData();
    enum
    {
        fftOrder = 10,
        fftSize = 1 << fftOrder
    };

    float MAX;

    unsigned int iteration2;

    unsigned int num3;
    unsigned int last;
    float lastFreq;
    float lastResonance;
    unsigned int last2;
    unsigned int SAMPLERATE;

    float max;


    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState treeState{ *this, nullptr, "Parameters", createParameterLayout() };


    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================

    juce::dsp::FFT forwardFFT;
    
    float fifo[fftSize];
    float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    juce::dsp::Gain<float> gain;
    juce::dsp::DryWetMixer<float> mix;

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    float currentVolume;

    //juce::dsp::LadderFilter<float> ladderFilter;
    SIMDLadderFilter<float> ladderFilter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MofoFilterAudioProcessor)
};
