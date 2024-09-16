#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Knob.h"
#include <JuceHeader.h>
#include <public.sdk/source/vst/vstcomponentbase.h>

void MofoFilterAudioProcessorEditor::showHostMenuForParam(const juce::MouseEvent& event, juce::String paramID)
{ 
    auto c = getHostContext(); 
    if (c != nullptr) 
    {
        auto d = c->getContextMenuForParameterIndex(audioProcessor.treeState.getParameter(paramID));
        auto o = juce::PopupMenu::Options();

        d->getEquivalentPopupMenu().showMenuAsync(o);
    }
}
void MofoFilterAudioProcessorEditor::showInfo(juce::String pId)
{

}

bool MofoFilterAudioProcessorEditor::keyPressed(const juce::KeyPress& k)
{
    if (k.getTextCharacter() == 't')
    {
        toolTipsEnabled = !toolTipsEnabled;
    }
    return true;
}

 
void MofoFilterAudioProcessorEditor::redrawButtons()
{
    if (audioProcessor.treeState.getRawParameterValue("isHighPass")->load() > 0.5f)
    {
        isHighPassOff.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isHighPassOff.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("isHighPass")->load() < 0.5f)
    {
        isHighPassOn.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isHighPassOn.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("is2Pole")->load() < 0.5f)
    {
        is2Pole2.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        is2Pole2.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("is2Pole")->load() > 0.5f)
    {
        is2Pole4.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        is2Pole4.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("freqDirection")->load() < 0.5f)
    {
        freqDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("freqDirection")->load() > 0.5f)
    {
        freqDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("resDirection")->load() < 0.5f)
    {
        resDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("resDirection")->load() > 0.5f)
    {
        resDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("driveDirection")->load() < 0.5f)
    {
        driveDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("driveDirection")->load() > 0.5f)
    {
        driveDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("speedDirection")->load() < 0.5f)
    {
        speedDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("speedDirection")->load() > 0.5f)
    {
        speedDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("isAuto")->load() < 0.5f)
    {
        isAutoOn.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isAutoOn.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("isAuto")->load() > 0.5f)
    {
        isAutoOff.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isAutoOff.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
}

void MofoFilterAudioProcessorEditor::timerCallback ()
{
    redrawButtons ();
}

//==============================================================================
MofoFilterAudioProcessorEditor::MofoFilterAudioProcessorEditor(MofoFilterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), 
    drive("", "drive"),
    driveAmount("", "driveAmount"),
    driveTension("Adjust the tension for the drive envelope", "driveTension"),
    minFrequency("Cutoff frequency for Auto Mode", "minCutoff"),
    cutoffFrequency("Cutoff frequency for Classic Mode", "cutoff"),
    cutoffAmount("Envelope follower for the cutoff frequency.", "cutoffAmount"),
    cutoffTension("Adjust the tension for the cutoff envelope.", "cutoffTension"),
    speedAmount("Envelope follower for the cutoff envelope", "speedAmount"),
    speedTension("Adjust the tension for the speed envelope", "speedTension"),
    resonance("", "resonance"), 
    resonanceAmt("", "resonanceAmount"),
    resonanceTension("Adjust the tension for the resonance envelope.", "resonanceTension"),
    volume("", "volume"),     
    mixer("Controls the dry/wet ratio", "mix")
{
    juce::Rectangle<int> r = juce::Desktop::getInstance().getDisplays().getTotalBounds(true);
    int x = r.getWidth();
    auto width = x / 2.0f;
    auto height = width / 1.77777f;

    AudioProcessorEditor::setResizable(true, true);
    AudioProcessorEditor::getConstrainer()->setFixedAspectRatio(1.7777777f);
    setWantsKeyboardFocus(false);
    setSize (width, height);

    tip.setEnabled(true);
    tip.setTopRightPosition(juce::Component::getMouseXYRelative().getX(), juce::Component::getMouseXYRelative().getY());
    tip.setMillisecondsBeforeTipAppears(1420);
    addAndMakeVisible(tip);

    isHighPassOff.setButtonText("LP");
    isHighPassOff.setRadioGroupId(isHP);
    
    isHighPassOff.setTooltip("Set filter to lowpass.");
    isHighPassOff.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("isHighPass")->load() > 0.5f)
    {
        isHighPassOff.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isHighPassOff.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    isHighPassOff.addListener(this);
    addAndMakeVisible(isHighPassOff);
    
    isHighPassOn.setButtonText("HP");
    isHighPassOn.setRadioGroupId(isHP);
    isHighPassOn.setTooltip("Set filter to high pass.");
    isHighPassOn.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("isHighPass")->load() < 0.5f)
    {
        isHighPassOn.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isHighPassOn.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    isHighPassOn.addListener(this);
    addAndMakeVisible(isHighPassOn);
    
    addAndMakeVisible(isHighPass);

    isHighPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "isHighPass", isHighPass);

    // IS2POLE

    is2Pole2.setButtonText("-12 dB");
    is2Pole2.setRadioGroupId(is2P);
    is2Pole2.setTooltip("Set slope to -12 dB per octave.");
    is2Pole2.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("is2Pole")->load() < 0.5f)
    {
        is2Pole2.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        is2Pole2.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    is2Pole2.addListener(this);
    addAndMakeVisible(is2Pole2);

    is2Pole4.setButtonText("-24 dB");
    is2Pole4.setRadioGroupId(is2P);
    is2Pole4.setTooltip("Set slope to -24 dB per octave.");
    is2Pole4.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("is2Pole")->load() > 0.5f)
    {
        is2Pole4.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        is2Pole4.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    is2Pole4.addListener(this);
    addAndMakeVisible(is2Pole4);

    addAndMakeVisible(is2Pole);

    is2PoleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "is2Pole", is2Pole);

    juce::String tooltipSuffix = " (is diminuendo).";

    // freqDirection
    freqDirectionUp.setButtonText("Up");
    freqDirectionUp.setRadioGroupId(freq);
    freqDirectionUp.setTooltip("The cutoff envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    freqDirectionUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("freqDirection")->load() < 0.5f)
    {
        freqDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    freqDirectionUp.addListener(this);
    addAndMakeVisible(freqDirectionUp);

    freqDirectionDown.setButtonText("Down");
    freqDirectionDown.setRadioGroupId(freq);
    freqDirectionDown.setTooltip("The cutoff envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    freqDirectionDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("freqDirection")->load() > 0.5f)
    {
        freqDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    freqDirectionDown.addListener(this);
    addAndMakeVisible(freqDirectionDown);

    
    addAndMakeVisible(freqDirection);

    freqDirectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "freqDirection", freqDirection);


    //resDirection
    resDirectionUp.setButtonText("Up");
    resDirectionUp.setRadioGroupId(res);
    resDirectionUp.setTooltip("The resonance envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    resDirectionUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("resDirection")->load() < 0.5f)
    {
        resDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    resDirectionUp.addListener(this);
    addAndMakeVisible(resDirectionUp);

    resDirectionDown.setButtonText("Down");
    resDirectionDown.setRadioGroupId(res);
    resDirectionDown.setTooltip("The resonance envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    resDirectionDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("resDirection")->load() > 0.5f)
    {
        resDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    resDirectionDown.addListener(this);
    addAndMakeVisible(resDirectionDown);

    addAndMakeVisible(resDirection);

    resDirectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resDirection", resDirection);

    //driveDirection
    driveDirectionUp.setButtonText("Up");
    driveDirectionUp.setRadioGroupId(drivee);
    driveDirectionUp.setTooltip("The drive envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    driveDirectionUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("driveDirection")->load() < 0.5f)
    {
        driveDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    driveDirectionUp.addListener(this);
    addAndMakeVisible(driveDirectionUp);

    driveDirectionDown.setButtonText("Down");
    driveDirectionDown.setRadioGroupId(drivee);
    driveDirectionDown.setTooltip("The drive envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    driveDirectionDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("driveDirection")->load() > 0.5f)
    {
        driveDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    driveDirectionDown.addListener(this);
    addAndMakeVisible(driveDirectionDown);

    addAndMakeVisible(driveDirection);

    driveDirectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "driveDirection", driveDirection);

    //speedDirection
    speedDirectionUp.setButtonText("Up");
    speedDirectionUp.setRadioGroupId(speeed);
    speedDirectionUp.setTooltip("This one's tricky... The cutoff envelope follower's upper bound sweeps downwards when the instrument has a decaying tone" + tooltipSuffix);
    speedDirectionUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("speedDirection")->load() < 0.5f)
    {
        speedDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedDirectionUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    speedDirectionUp.addListener(this);
    addAndMakeVisible(speedDirectionUp);

    speedDirectionDown.setButtonText("Down");
    speedDirectionDown.setRadioGroupId(speeed);
    speedDirectionUp.setTooltip("This one's tricky... The cutoff envelope follower's upper bound sweeps upwards when the instrument has a decaying tone" + tooltipSuffix);
    speedDirectionDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("speedDirection")->load() > 0.5f)
    {
        speedDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedDirectionDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    speedDirectionDown.addListener(this);
    addAndMakeVisible(speedDirectionDown);

    addAndMakeVisible(speedDirection);

    speedDirectionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "speedDirection", speedDirection);

    // ISAUTO
    isAutoOn.setButtonText("Auto");
    isAutoOn.setRadioGroupId(Auto);
    isAutoOn.setTooltip("Sets the cutoff mode to auto, setting the cutoff frequency in harmonics of the current lowest note played. 1 is the estimated fundamental frequency.");
    isAutoOn.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("isAuto")->load() < 0.5f)
    {
        isAutoOn.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isAutoOn.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    isAutoOn.addListener(this);
    addAndMakeVisible(isAutoOn);

    isAutoOff.setButtonText("Classic");
    isAutoOff.setRadioGroupId(Auto);
    isAutoOff.setTooltip("Sets the cutoff mode to classic. This is the classic cutoff mode that most pedals and plugins primarily use. Set in Hz.");
    isAutoOff.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("isAuto")->load() > 0.5f)
    {
        isAutoOff.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        isAutoOff.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    isAutoOff.addListener(this);
    addAndMakeVisible(isAutoOff);

    
    addAndMakeVisible(isAuto);

    isAutoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "isAuto", isAuto);

    drive.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    drive.setTooltip("Sets the amount of saturation/distortion.");

    addAndMakeVisible(drive);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "drive", drive);

    minFrequency.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    minFrequency.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    minFrequency.setTooltip("Sets the frequency in harmonics of the lowest note played, where 1 is the fundamental frequency. Only works when the Auto cutoff mode is selected.");
    minFrequency.setLookAndFeel(&knob);
    addAndMakeVisible(minFrequency);

    minFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "minCutoff", minFrequency);


    cutoffFrequency.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    cutoffFrequency.setTextBoxStyle(juce::Slider::TextBoxBelow,false, 58, 16);

    cutoffFrequency.setTooltip("Set the base cutoff frequency in Hz. Only works when the Classic cutoff mode is selected.");

    cutoffFrequency.setLookAndFeel(&knob);
    addAndMakeVisible(cutoffFrequency);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "cutoff", cutoffFrequency);

    cutoffAmount.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    cutoffAmount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    cutoffAmount.setLookAndFeel(&knob);
    cutoffAmount.setTooltip("Sets the upper bound of the cutoff envelope follower, units are +(half octaves).");
    addAndMakeVisible(cutoffAmount);

    cutoffAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "cutoffAmount", cutoffAmount);
   
    resonance.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    resonance.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    resonance.setTooltip("Sets the resonance of the filter. At higher values, the filter will self-oscillate.");
    resonance.setLookAndFeel(&knob);
    addAndMakeVisible(resonance);

    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resonance", resonance);

    resonanceAmt.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    resonanceAmt.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    resonanceAmt.setLookAndFeel(&knob);
    resonanceAmt.setTooltip("Sets the upper bound of the resonance sweeping.");
    addAndMakeVisible(resonanceAmt);

    resonanceAmtAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resonanceAmount", resonanceAmt);

    driveAmount.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveAmount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    driveAmount.setLookAndFeel(&knob);
    driveAmount.setTooltip("Sets the upper bound of the drive sweeping.");
    addAndMakeVisible(driveAmount);

    driveAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "driveAmount", driveAmount);

    speedAmount.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    speedAmount.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    speedAmount.setLookAndFeel(&knob);
    speedAmount.setTooltip("Sets the upper bound of the cutoff envelope follower's upper bound.");
    addAndMakeVisible(speedAmount);

    speedAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "speedAmount", speedAmount);

    volume.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    volume.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    volume.setLookAndFeel(&center);
    volume.setTooltip("Sets the output volume of the plugin in dB.");
    addAndMakeVisible(volume);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "volume", volume);

    cutoffTension.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    cutoffTension.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    cutoffTension.setLookAndFeel(&center);
    cutoffTension.setTooltip("Changes the tension that the cutoff envelope follower responds with. 0 = linear");
    addAndMakeVisible(cutoffTension);

    cutoffTensionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "cutoffTension", cutoffTension);

    resonanceTension.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    resonanceTension.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    resonanceTension.setLookAndFeel(&center);
    resonanceTension.setTooltip("Changes the tension that the resonance envelope follower responds with. 0 = linear");
    addAndMakeVisible(resonanceTension);

    resonanceTensionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resonanceTension", resonanceTension);

    driveTension.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveTension.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    driveTension.setLookAndFeel(&center);
    driveTension.setTooltip("Changes the tension that the drive envelope follower responds with. 0 = linear");
    addAndMakeVisible(driveTension);

    driveTensionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "driveTension", driveTension);

    speedTension.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    speedTension.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    speedTension.setLookAndFeel(&center);
    speedTension.setTooltip("Changes the tension that the speed envelope follower responds with. 0 = linear");
    addAndMakeVisible(speedTension);

    speedTensionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "speedTension", speedTension);

    addAndMakeVisible (mixer);
    mixerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.treeState, "mix", mixer);
}

void MofoFilterAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &isHighPassOff)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        isHighPassOn.setToggleState(false, juce::NotificationType::dontSendNotification);
        isHighPass.setValue(0);
    }
    else if (button == &isHighPassOn)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        isHighPassOff.setToggleState(false, juce::NotificationType::dontSendNotification);
        isHighPass.setValue(1);
    }
    else if (button == &is2Pole2)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        is2Pole4.setToggleState(false, juce::NotificationType::dontSendNotification);
        is2Pole.setValue(1);
    }
    else if (button == &is2Pole4)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        is2Pole2.setToggleState(false, juce::NotificationType::dontSendNotification);
        is2Pole.setValue(0);
    }
    else if (button == &isAutoOn)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        isAutoOff.setToggleState(false, juce::NotificationType::dontSendNotification);
        isAuto.setValue(1);
    }
    else if (button == &isAutoOff)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        isAutoOn.setToggleState(false, juce::NotificationType::dontSendNotification);
        isAuto.setValue(0);
    }
    else if (button == &freqDirectionUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        freqDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        freqDirection.setValue(1);
    }
    else if (button == &freqDirectionDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        freqDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        freqDirection.setValue(0);
    }
    else if (button == &resDirectionUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        resDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        resDirection.setValue(1);
    }
    else if (button == &resDirectionDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        resDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        resDirection.setValue(0);
    }
    else if (button == &driveDirectionUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        driveDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        driveDirection.setValue(1);
    }
    else if (button == &driveDirectionDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        driveDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        driveDirection.setValue(0);
    }
    else if (button == &speedDirectionUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        speedDirectionDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        speedDirection.setValue(1);
    }
    else if (button == &speedDirectionDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        speedDirectionUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        speedDirection.setValue(0);
    }
}

MofoFilterAudioProcessorEditor::~MofoFilterAudioProcessorEditor()
{
}

//==============================================================================
void MofoFilterAudioProcessorEditor::paint(juce::Graphics& g)
{   
    UI = juce::ImageCache::getFromMemory(BinaryData::unlockedUI_png, BinaryData::unlockedUI_pngSize);

    g.drawImageWithin(UI, 0, 0, AudioProcessorEditor::getWidth(), AudioProcessorEditor::getHeight(), juce::RectanglePlacement::stretchToFit);
}

void MofoFilterAudioProcessorEditor::resized()
{
    auto size = AudioProcessorEditor::getWidth() * 0.1f;

    // 2, 7
    isHighPassOff.setBounds(getWidth() * 0.05f * 1.9f, getWidth() * 0.05f * 6.2f, size * 0.6f, size * 0.25f);
    // 2, 7.5
    isHighPassOn.setBounds(getWidth() * 0.05f * 1.9f, getWidth() * 0.05f * 6.7f, size * 0.6f, size * 0.25f);

    is2Pole2.setBounds(getWidth() * 0.05f * 6.3f, getWidth() * 0.05f * 6.2f, size * 0.7f, size * 0.25f);
    is2Pole4.setBounds(getWidth() * 0.05f * 6.3f, getWidth() * 0.05f * 6.7f, size * 0.7f, size * 0.25f);

    isAutoOn.setBounds(getWidth() * 0.05f * 4.f, getWidth() * 0.05f * 3.1f, size * 0.8f, size * 0.25f);
    isAutoOff.setBounds(getWidth() * 0.05f * 4.f, getWidth() * 0.05f * 3.6f, size * 0.8f, size * 0.25f);

    // cutoff (2,1)
    cutoffFrequency.setBounds(getWidth() * 0.05 * 5.74f, getWidth() * 0.05f * 2.2f, size*1.3f, size*1.3f);

    // mincutoff (2,,3)
    minFrequency.setBounds(getWidth() * 0.05f * 1.18f, getWidth() * 0.05f * 2.2f, size*1.3f, size * 1.3f);

    // resonance (4,,4)
    resonance.setBounds(getWidth() * 0.05f * 3.4f, getWidth() * 0.05f * 6.3f, size * 1.3f, size * 1.3f);

    //drive 9.5, 3
    drive.setBounds(getWidth() * 0.05f * 9.f, getWidth() * 0.05f * 3.85f, size, size);

    volume.setBounds(getWidth() * 0.05f * 9.f, getWidth() * 0.05f * 6.25f, size, size);

    mixer.setBounds (getWidth () * 0.05f * 9.f, getWidth () * 0.05f * 1.3f, size, size);

    //cutoff amount 12,2
    cutoffAmount.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 2.0f, size * 0.8f, size * 0.8f);
    resonanceAmt.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 6.13f, size * 0.8f, size * 0.8f);

    //driveAmt 11.75, 5.25
    driveAmount.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 8.32f, size * 0.8f, size * 0.8f);

    //speedAmt 11.75, 7.25
    speedAmount.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 3.95f, size * 0.8f, size * 0.8f);

    cutoffTension.setBounds(getWidth() * 0.05f * 17.05f, getWidth() * 0.05f * 2.0f, size * 0.8f, size * 0.8f);

    speedTension.setBounds(getWidth() * 0.05f * 17.05f, getWidth() * 0.05f * 3.95f, size * 0.8f, size * 0.8f);

    resonanceTension.setBounds(speedTension.getX(), resonanceAmt.getY(), size * 0.8f, size * 0.8f);

    driveTension.setBounds(speedTension.getX(), driveAmount.getY(), size * 0.8f, size * 0.8f);

    freqDirectionUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 2.08f, size * 0.84f, size * 0.32f);
    freqDirectionDown.setBounds(freqDirectionUp.getX(), freqDirectionUp.getY() + freqDirectionUp.getHeight(), freqDirectionUp.getWidth(), size * 0.32f);

    resDirectionUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 6.25f, size * 0.84f, size * 0.32f);
    resDirectionDown.setBounds(resDirectionUp.getX(), resDirectionUp.getY() + resDirectionUp.getHeight() - 2, resDirectionUp.getWidth(), size * 0.32f);

    driveDirectionUp.setBounds(resDirectionUp.getX(), getWidth() * 0.05f * 8.5f, size * 0.84f, size * 0.32f);
    driveDirectionDown.setBounds(driveDirectionUp.getX(), driveDirectionUp.getY() + driveDirectionUp.getHeight(), driveDirectionUp.getWidth(), size * 0.32f);

    speedDirectionUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 4.1f, size * 0.84f, size * 0.32f);
    speedDirectionDown.setBounds(speedDirectionUp.getX(), speedDirectionUp.getY() + speedDirectionUp.getHeight() - 2, speedDirectionUp.getWidth(), size * 0.32f);
}