#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Knob.h"
#include "MagicKnob.h"
#include "../Source/modules/licensing/SoftwareLicensorUnlockForm.h";

// ─── Version ─────────────────────────────────────────────────────────────────
// SOURCE OF TRUTH: the version string is injected by CMake from the .jucer file
// via the VERSION compile definition.  The constant below provides a safe
// fallback when building outside of the CMake pipeline (e.g. directly from an
// IDE that doesn't run the CMake configure step).
#ifndef VERSION
  #define VERSION "0.0.0"
#endif
const juce::String clientVersion = VERSION;

// ─── Version-check helpers ───────────────────────────────────────────────────
namespace VersionCheck
{
    /** Key used to persist the last-check epoch in the JUCE application properties. */
    static const juce::String kLastCheckKey   = "lastVersionCheckEpoch";
    /** Minimum gap (in seconds) between remote version checks: 3 days. */
    static constexpr int64    kCheckIntervalS = 3LL * 24LL * 3600LL;
    /** CloudFront URL that serves the current version string, e.g. "1.0.9". */
    static const juce::String kVersionUrl     =
        "https://hephaestus.alteredbrainchemistry.com/downloads/mofo-mojo-version.txt";
    /** User-Agent header required by CloudFront to allow the download. */
    static const juce::String kUserAgent      = "MofoMojoPlugin/" VERSION;

    /**
     * Returns the PropertiesFile used to persist the timestamp.
     * Uses the same application properties as the rest of the plugin.
     */
    inline juce::PropertiesFile* getPropsFile()
    {
        juce::PropertiesFile::Options opts;
        opts.applicationName     = "MofoMojoFilter";
        opts.filenameSuffix      = ".settings";
        opts.folderName          = "Altered Brain Chemistry";
        opts.osxLibrarySubFolder = "Application Support";
        static juce::ApplicationProperties appProps;
        appProps.setStorageParameters(opts);
        return appProps.getUserSettings();
    }

    /**
     * Returns the Unix epoch (seconds) of the last successful version check,
     * or 0 if no check has ever been performed.
     */
    inline int64 getLastCheckTime()
    {
        auto* props = getPropsFile();
        if (props == nullptr) return 0;
        return (int64) props->getDoubleValue(kLastCheckKey, 0.0);
    }

    /** Saves the current wall-clock time as the last-check timestamp. */
    inline void saveCheckTime()
    {
        auto* props = getPropsFile();
        if (props == nullptr) return;
        auto now = (int64) juce::Time::currentTimeMillis() / 1000LL;
        props->setValue(kLastCheckKey, (double) now);
        props->saveIfNeeded();
    }

    /**
     * Returns true when more than kCheckIntervalS seconds have elapsed since
     * the last recorded version check.
     */
    inline bool isDue()
    {
        auto lastCheck = getLastCheckTime();
        if (lastCheck == 0) return true;                   // never checked
        auto now = (int64) juce::Time::currentTimeMillis() / 1000LL;
        return (now - lastCheck) >= kCheckIntervalS;
    }

    /**
     * Fetches the remote version string synchronously.
     * The CloudFront distribution requires a recognisable User-Agent; we send
     * kUserAgent so the request is allowed through.
     *
     * Returns an empty string on any network / HTTP error.
     */
    inline juce::String fetchRemoteVersion()
    {
        juce::URL url(kVersionUrl);
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders("User-Agent: " + kUserAgent)
                           .withConnectionTimeoutMs(8000);

        auto stream = url.createInputStream(options);
        if (stream == nullptr) return {};

        // The file should contain a single line like "1.0.9"
        auto text = stream->readEntireStreamAsString().trim();
        // Basic sanity check: must look like a version number
        if (text.isEmpty() || !text[0].isDigit()) return {};
        return text;
    }
} // namespace VersionCheck

/**
 * @brief A license status notification that only shows when the software is locked.
 */
class LicenseStatusNotification : public juce::Component
{
public:
    LicenseStatusNotification()
    {
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
        g.fillAll(juce::Colours::darkslategrey);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        messageLabel.setBoundsInset(juce::BorderSize<int>(10));
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
     * Checks whether a new version is available.
     *
     * The check proceeds in two phases:
     *   1. If a cloud version is already stored on the LicensingStatus object
     *      (i.e. the licensor server already told us), use that.
     *   2. Otherwise, if 3 days have elapsed since we last fetched from
     *      CloudFront, fetch the version text now, save the timestamp, and
     *      compare against the build-time clientVersion.
     *
     * Either way the comparison logic is the same semver check as before.
     */
    bool isNewVersionAvailable()
    {
        juce::String cloudVersion;

        // Phase 1 – prefer the version the licensor already gave us.
        if (status.isUnlocked())
        {
            cloudVersion = status.getCloudVersion();
        }

        // Phase 2 – if we don't have a cloud version yet, do a direct check
        // (rate-limited to once per 3 days).
        if (cloudVersion.isEmpty() && VersionCheck::isDue())
        {
            auto fetched = VersionCheck::fetchRemoteVersion();
            if (fetched.isNotEmpty())
            {
                cloudVersion = fetched;
                // Record the time of this successful check so we don't hammer
                // the endpoint again for another 3 days.
                VersionCheck::saveCheckTime();
            }
        }

        if (cloudVersion.isEmpty()) return false;

        return compareVersions(cloudVersion, clientVersion) > 0;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkslategrey);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
    }

    void resized() override
    {
        messageLabel.setBoundsInset(juce::BorderSize<int>(10));
    }

private:
    juce::Label     messageLabel;
    LicensingStatus& status;

    /**
     * Returns  1 if a > b,  0 if equal, -1 if a < b.
     * Compares up to three numeric segments separated by '.'.
     */
    static int compareVersions(const juce::String& a, const juce::String& b)
    {
        auto parsePart = [](const juce::String& v, int seg) -> int {
            juce::StringArray parts;
            parts.addTokens(v, ".", "");
            if (seg >= parts.size()) return 0;
            return parts[seg].getIntValue();
        };

        for (int i = 0; i < 3; ++i)
        {
            int av = parsePart(a, i);
            int bv = parsePart(b, i);
            if (av > bv) return  1;
            if (av < bv) return -1;
        }
        return 0;
    }
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

    juce::TextButton&          unlockButton;
    LicensingStatus&           status;
    LicenseStatusNotification& notification;
};

//==============================================================================
class MofoFilterAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        public juce::Button::Listener,
                                        public juce::Timer
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
    bool toolTipsEnabled       = true;
    bool isEnabled             = true;

    void timerCallback () override;
    void buttonClicked(juce::Button* button) override;
    void showHostMenuForParam(const juce::MouseEvent& e, juce::String paramID);
    juce::PopupMenu modifyHostMenu(juce::PopupMenu menu);

    UnlockForm                 unlockForm;
    LicenseStatusNotification  notification;
    VersionChangeNotification  versionNotification;

    void showInfo(juce::String pId);

private:
    MofoFilterAudioProcessor& audioProcessor;
    juce::Image UI;

    enum
    {
        isHP    = 111,
        is2P    = 222,
        freq    = 223,
        Auto    = 224,
        res     = 225,
        speeed  = 226,
        drivee  = 227
    };

    Knob       knob;
    CenterKnob center;

    juce::TextButton unlockButton;

    juce::TextButton isHighPassOff;
    juce::TextButton isHighPassOn;
    juce::Slider     isHighPass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> isHighPassAttachment;

    juce::TextButton isAutoOff;
    juce::TextButton isAutoOn;
    juce::Slider     isAuto;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> isAutoAttachment;

    juce::TextButton is2Pole2;
    juce::TextButton is2Pole4;
    juce::Slider     is2Pole;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> is2PoleAttachment;

    juce::TextButton freqDirectionUp;
    juce::TextButton freqDirectionDown;
    juce::Slider     freqDirection;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqDirectionAttachment;

    juce::TextButton resDirectionUp;
    juce::TextButton resDirectionDown;
    juce::Slider     resDirection;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resDirectionAttachment;

    juce::TextButton driveDirectionUp;
    juce::TextButton driveDirectionDown;
    juce::Slider     driveDirection;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveDirectionAttachment;

    juce::TextButton speedDirectionUp;
    juce::TextButton speedDirectionDown;
    juce::Slider     speedDirection;
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
