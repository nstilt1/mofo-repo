#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Knob.h" 
#include "MagicKnob.h"
#include "../Source/modules/licensing/SoftwareLicensorUnlockForm.h";

const juce::String clientVersion = "1.0.2";

/**
 * @brief A license status notification that only shows when the software is locked.
 */
class LicenseStatusNotification : public juce::Component
{
public:
    LicenseStatusNotification()
    {
        // Configure the label
        addAndMakeVisible(messageLabel);
        messageLabel.setJustificationType(juce::Justification::centredLeft);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::black);
        messageLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
        messageLabel.setColour(juce::Label::outlineColourId, juce::Colours::black);
    }

    virtual void setText(const juce::String& text)
    {
        messageLabel.setText("License Status: " + text, juce::dontSendNotification);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkslategrey); // Set your desired background color
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1); // Set your desired border thickness
    }

    void resized() override
    {
        messageLabel.setBoundsInset(juce::BorderSize<int>(10)); // Margin inside the component
    }

private:
    juce::Label messageLabel;
};

class VersionChangeNotification : public juce::Component
{
public:
    VersionChangeNotification(LicensingStatus& status) : status(status)
    {
        addAndMakeVisible(messageLabel);
        messageLabel.setJustificationType(juce::Justification::centredLeft);
        messageLabel.setColour(juce::Label::textColourId, juce::Colours::black);
        messageLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
        messageLabel.setColour(juce::Label::outlineColourId, juce::Colours::black);
        messageLabel.setText("A new version is available", juce::NotificationType::dontSendNotification);
    }

    /**
     * @brief Returns whether a new version is available based on the licensing response.
     */
    bool isNewVersionAvailable() {
        if (!status.isUnlocked())
            return false;

        auto cloudVersion = status.getCloudVersion();
        auto cloudMajorVersion = cloudVersion.getIntValue();
        auto clientMajorVersion = clientVersion.getIntValue();
        if (cloudMajorVersion > clientMajorVersion) 
            return true;
        
        // if the cloud and client versions are the same, the dot index will be the same
        auto dotIndex = cloudVersion.indexOf(".");
        auto cloudTrimmed = cloudVersion.substring(dotIndex + 1);
        auto clientTrimmed = clientVersion.substring(dotIndex + 1);
        auto cloudMinorVersion = cloudTrimmed.getIntValue();
        auto clientMinorVersion = clientTrimmed.getIntValue();
        if (cloudMinorVersion > clientMinorVersion)
            return true;

        auto cloudPatch = cloudVersion.getTrailingIntValue();
        auto clientPatch = clientVersion.getTrailingIntValue();
        if (cloudPatch > clientPatch)
            return true;

        return false;
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkslategrey); // Set your desired background color
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1); // Set your desired border thickness
    }

    void resized() override
    {
        messageLabel.setBoundsInset(juce::BorderSize<int>(10)); // Margin inside the component
    }

private:
    juce::Label messageLabel;
    LicensingStatus& status;
};

class UnlockForm : public SoftwareLicensorUnlockForm
{
public:
    UnlockForm(
        LicensingStatus& status, 
        juce::String userInstructions, 
        bool hasCancelButton, 
        juce::TextButton& unlockButton, 
        LicenseStatusNotification& notification) 
        : SoftwareLicensorUnlockForm(status, userInstructions, hasCancelButton), 
            unlockButton(unlockButton), 
            status(status),
            notification(notification)
    {}
    void dismiss() override
    {
        this->setVisible(false);
        this->exitModalState();
        if (this->status.isUnlocked())
        {
            unlockButton.setVisible(false);
            notification.setVisible(false);
        }
    }
    juce::TextButton& unlockButton;
    LicensingStatus& status;
    LicenseStatusNotification& notification;
};

//class LicensingStatusNotification : public juce::Component
//{
//    LicensingStatusNotification(SoftwareLicensorStatus& s)
//};

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

    juce::PopupMenu modifyHostMenu(juce::PopupMenu menu);
    UnlockForm unlockForm;
    LicenseStatusNotification notification;
    VersionChangeNotification versionNotification;
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

    juce::TextButton unlockButton;

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
