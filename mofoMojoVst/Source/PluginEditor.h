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

    bool toolTipsEnabled;

    bool isEnabled = true;
    
    double lastCutValue;
    double lastMinCutValue;

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

    juce::TextButton freqIsUpUp;
    juce::TextButton freqIsUpDown;
    juce::Slider freqIsUp;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqIsUpAttachment;

    juce::TextButton resIsUpUp;
    juce::TextButton resIsUpDown;
    juce::Slider resIsUp;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resIsUpAttachment;

    juce::TextButton driveIsUpUp;
    juce::TextButton driveIsUpDown;
    juce::Slider driveIsUp;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveIsUpAttachment;

    juce::TextButton speedIsUpUp;
    juce::TextButton speedIsUpDown;
    juce::Slider speedIsUp;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedIsUpAttachment;

    MagicKnob minFrequency;
    MagicKnob amount;
    MagicKnob cutoffFrequency;

    MagicKnob resonance;
    MagicKnob resonanceAmt;


    MagicKnob drive;
    MagicKnob volume;
    MagicKnob driveAmt;
    MagicKnob speedAmt;
    MagicKnob shape;
    MagicKnob resShape;
    MagicKnob driveShape;
    MagicKnob speedShape;

    MagicKnob mixer;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> harmonicThreshAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxResonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> shapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resShapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveShapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedShapeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixerAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MofoFilterAudioProcessorEditor)
};
