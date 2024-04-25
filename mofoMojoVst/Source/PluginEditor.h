#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Knob.h" 
#include "MagicKnob.h"

//==============================================================================
/**
*/
class MofoFilterAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, public juce::Timer
{
public:
    MofoFilterAudioProcessorEditor (MofoFilterAudioProcessor&);
    ~MofoFilterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::TooltipWindow tip;
    
    bool keyPressed(const juce::KeyPress& k) override;

    void redrawButtons();

    bool shouldWindowBeVisible = false;

    bool toolTipsEnabled = true;

    bool isEnabled = true;

    void timerCallback () override;

    void buttonClicked(juce::Button* button) override;

    void showHostMenuForParam(const juce::MouseEvent& e, juce::String paramID);

    void showInfo(juce::String pId);
private:
    MofoFilterAudioProcessor& audioProcessor;
    juce::Image UI;
    enum
    {
        isHP = 111,
        is2P = 222,
        freq = 223,
        Auto = 224,
        res = 225,
        speeed = 226,
        drivee = 227
    };

    Knob knob;
    CenterKnob center;

    juce::TextButton isHighPassOff;
    juce::TextButton isHighPassOn;
    juce::Slider isHighPass;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> isHighPassAttachment;

    juce::TextButton isAutoOff;
    juce::TextButton isAutoOn;
    juce::Slider isAuto;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> isAutoAttachment;

    juce::TextButton is2Pole2;
    juce::TextButton is2Pole4;
    juce::Slider is2Pole;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> is2PoleAttachment;

    juce::TextButton freqDirectionUp;
    juce::TextButton freqDirectionDown;
    juce::Slider freqDirection;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqDirectionAttachment;

    juce::TextButton resDirectionUp;
    juce::TextButton resDirectionDown;
    juce::Slider resDirection;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resDirectionAttachment;

    juce::TextButton driveDirectionUp;
    juce::TextButton driveDirectionDown;
    juce::Slider driveDirection;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveDirectionAttachment;

    juce::TextButton speedDirectionUp;
    juce::TextButton speedDirectionDown;
    juce::Slider speedDirection;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedDirectionAttachment;

    MagicKnob minFrequency;
    MagicKnob cutoffAmount;
    MagicKnob cutoffFrequency;
    MagicKnob cutoffTension;
    MagicKnob speedAmount;
    MagicKnob speedTension;

    MagicKnob resonance;
    MagicKnob resonanceAmt;
    MagicKnob resonanceTension;

    MagicKnob drive;
    MagicKnob driveAmount;
    MagicKnob driveTension;
    MagicKnob volume;

    MagicKnob mixer;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> harmonicThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAmtAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffTensionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceTensionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveTensionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedTensionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixerAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MofoFilterAudioProcessorEditor)
};
