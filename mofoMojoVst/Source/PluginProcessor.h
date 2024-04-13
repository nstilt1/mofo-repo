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

struct ParamTree
{
    juce::ValueTree node;
};

struct ChainSettings 
{
    float drive{ 0 }, cutoff{ 0 }, minCutoff{ 0 }, resonance{ 0 }, volume{ 0 }, mix{ 0 };
    float amount{ 0 }, maxResonance{ 0 }, shape{ 0 }, resShape{ 0 }, driveShape{ 0 }, speedShape{ 0 }, maxSpeed{ 0 }, maxDrive{ 0 };
    int is2Pole{ 0 }, isHighPass{ 0 }, isAuto{ 0 }, freqIsUp{ 0 }, resIsUp{ 0 }, amountIsUp{ 0 }, driveIsUp{ 0 }, speedIsUp{ 0 };
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

    juce::dsp::LadderFilter<float> ladderFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MofoFilterAudioProcessor)
};
