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
    if (audioProcessor.treeState.getRawParameterValue("freqIsUp")->load() < 0.5f)
    {
        freqIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("freqIsUp")->load() > 0.5f)
    {
        freqIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("resIsUp")->load() < 0.5f)
    {
        resIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("resIsUp")->load() > 0.5f)
    {
        resIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("driveIsUp")->load() < 0.5f)
    {
        driveIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("driveIsUp")->load() > 0.5f)
    {
        driveIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("speedIsUp")->load() < 0.5f)
    {
        speedIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    if (audioProcessor.treeState.getRawParameterValue("speedIsUp")->load() > 0.5f)
    {
        speedIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
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
    : AudioProcessorEditor(&p), audioProcessor(p), drive("", "drive"),
    minFrequency("Cutoff frequency for Auto Mode", "minCutoff"), amount("", "amount"), cutoffFrequency("Cutoff frequency for Classic Mode", "cutoff"),
    resonance("", "resonance"), resonanceAmt("", "maxResonance"), volume("", "volume"), driveAmt("", "maxDrive"), speedAmt("Envelope follower for the cutoff amount", "maxSpeed"),
    shape("Adjust the tension for the cutoff envelope.", "shape"), resShape("Adjust the tension for the resonance envelope.", "resShape"), 
    driveShape("Adjust the tension for the drive envelope", "driveShape"), speedShape("Adjust the tension for the sepeed envelope", "speedShape"), 
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

    // FREQISUP
    freqIsUpUp.setButtonText("Up");
    freqIsUpUp.setRadioGroupId(freq);
    freqIsUpUp.setTooltip("The cutoff envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    freqIsUpUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("freqIsUp")->load() < 0.5f)
    {
        freqIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    freqIsUpUp.addListener(this);
    addAndMakeVisible(freqIsUpUp);

    freqIsUpDown.setButtonText("Down");
    freqIsUpDown.setRadioGroupId(freq);
    freqIsUpDown.setTooltip("The cutoff envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    freqIsUpDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("freqIsUp")->load() > 0.5f)
    {
        freqIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        freqIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    freqIsUpDown.addListener(this);
    addAndMakeVisible(freqIsUpDown);

    
    addAndMakeVisible(freqIsUp);

    freqIsUpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "freqIsUp", freqIsUp);


    //RESISUP
    resIsUpUp.setButtonText("Up");
    resIsUpUp.setRadioGroupId(res);
    resIsUpUp.setTooltip("The resonance envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    resIsUpUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("resIsUp")->load() < 0.5f)
    {
        resIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    resIsUpUp.addListener(this);
    addAndMakeVisible(resIsUpUp);

    resIsUpDown.setButtonText("Down");
    resIsUpDown.setRadioGroupId(res);
    resIsUpDown.setTooltip("The resonance envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    resIsUpDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("resIsUp")->load() > 0.5f)
    {
        resIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        resIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    resIsUpDown.addListener(this);
    addAndMakeVisible(resIsUpDown);

    addAndMakeVisible(resIsUp);

    resIsUpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resIsUp", resIsUp);

    //DRIVEISUP
    driveIsUpUp.setButtonText("Up");
    driveIsUpUp.setRadioGroupId(drivee);
    driveIsUpUp.setTooltip("The drive envelope follower sweeps up when the instrument has a decaying tone" + tooltipSuffix);
    driveIsUpUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("driveIsUp")->load() < 0.5f)
    {
        driveIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    driveIsUpUp.addListener(this);
    addAndMakeVisible(driveIsUpUp);

    driveIsUpDown.setButtonText("Down");
    driveIsUpDown.setRadioGroupId(drivee);
    driveIsUpDown.setTooltip("The drive envelope follower sweeps down when the instrument has a decaying tone" + tooltipSuffix);
    driveIsUpDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("driveIsUp")->load() > 0.5f)
    {
        driveIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        driveIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    driveIsUpDown.addListener(this);
    addAndMakeVisible(driveIsUpDown);

    addAndMakeVisible(driveIsUp);

    driveIsUpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "driveIsUp", driveIsUp);

    //SPEEDISUP
    speedIsUpUp.setButtonText("Up");
    speedIsUpUp.setRadioGroupId(speeed);
    speedIsUpUp.setTooltip("This one's tricky... The cutoff envelope follower's upper bound sweeps downwards when the instrument has a decaying tone" + tooltipSuffix);
    speedIsUpUp.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnBottom);
    if (audioProcessor.treeState.getRawParameterValue("speedIsUp")->load() < 0.5f)
    {
        speedIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedIsUpUp.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    speedIsUpUp.addListener(this);
    addAndMakeVisible(speedIsUpUp);

    speedIsUpDown.setButtonText("Down");
    speedIsUpDown.setRadioGroupId(speeed);
    speedIsUpUp.setTooltip("This one's tricky... The cutoff envelope follower's upper bound sweeps upwards when the instrument has a decaying tone" + tooltipSuffix);
    speedIsUpDown.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnTop);
    if (audioProcessor.treeState.getRawParameterValue("speedIsUp")->load() > 0.5f)
    {
        speedIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
    }
    else
    {
        speedIsUpDown.setToggleState(true, juce::NotificationType::dontSendNotification);
    }
    speedIsUpDown.addListener(this);
    addAndMakeVisible(speedIsUpDown);

    addAndMakeVisible(speedIsUp);

    speedIsUpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "speedIsUp", speedIsUp);

    // ISAUTO
    isAutoOn.setButtonText("Auto");
    isAutoOn.setRadioGroupId(Auto);
    isAutoOn.setTooltip("Sets the cutoff mode to auto, setting the cutoff frequency in harmonics of the current lowest note played. 1 = fundamental frequency.");
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

    amount.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    amount.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    amount.setLookAndFeel(&knob);
    amount.setTooltip("Sets the upper bound of the envelope follower, units are +(half octaves).");
    addAndMakeVisible(amount);

    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "amount", amount);
   
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

    maxResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "maxResonance", resonanceAmt);

    driveAmt.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveAmt.setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 16);
    driveAmt.setLookAndFeel(&knob);
    driveAmt.setTooltip("Sets the upper bound of the drive sweeping.");
    addAndMakeVisible(driveAmt);

    maxDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "maxDrive", driveAmt);

    speedAmt.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    speedAmt.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    speedAmt.setLookAndFeel(&knob);
    speedAmt.setTooltip("Sets the upper bound of the cutoffs follower's upper bound.");
    addAndMakeVisible(speedAmt);

    maxSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "maxSpeed", speedAmt);

    volume.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    volume.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    volume.setLookAndFeel(&center);
    volume.setTooltip("Sets the output volume of the plugin in dB.");
    addAndMakeVisible(volume);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "volume", volume);

    shape.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    shape.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    shape.setLookAndFeel(&center);
    shape.setTooltip("Changes the shape that the cutoff follower responds with. 0 = linear");
    addAndMakeVisible(shape);

    shapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "shape", shape);

    resShape.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    resShape.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    resShape.setLookAndFeel(&center);
    resShape.setTooltip("Changes the shape that the resonance follower responds with. 0 = linear");
    addAndMakeVisible(resShape);

    resShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "resShape", resShape);

    driveShape.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    driveShape.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    driveShape.setLookAndFeel(&center);
    driveShape.setTooltip("Changes the shape that the drive follower responds with. 0 = linear");
    addAndMakeVisible(driveShape);

    driveShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "driveShape", driveShape);

    speedShape.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    speedShape.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 16);
    speedShape.setLookAndFeel(&center);
    speedShape.setTooltip("Changes the shape that the speed follower responds with. 0 = linear");
    addAndMakeVisible(speedShape);

    speedShapeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "speedShape", speedShape);

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
    else if (button == &freqIsUpUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        freqIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        freqIsUp.setValue(1);
    }
    else if (button == &freqIsUpDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        freqIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        freqIsUp.setValue(0);
    }
    else if (button == &resIsUpUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        resIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        resIsUp.setValue(1);
    }
    else if (button == &resIsUpDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        resIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        resIsUp.setValue(0);
    }
    else if (button == &driveIsUpUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        driveIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        driveIsUp.setValue(1);
    }
    else if (button == &driveIsUpDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        driveIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        driveIsUp.setValue(0);
    }
    else if (button == &speedIsUpUp)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        speedIsUpDown.setToggleState(false, juce::NotificationType::dontSendNotification);
        speedIsUp.setValue(1);
    }
    else if (button == &speedIsUpDown)
    {
        button->setToggleState(true, juce::NotificationType::dontSendNotification);
        speedIsUpUp.setToggleState(false, juce::NotificationType::dontSendNotification);
        speedIsUp.setValue(0);
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
    amount.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 2.0f, size * 0.8f, size * 0.8f);
    resonanceAmt.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 6.13f, size * 0.8f, size * 0.8f);

    //driveAmt 11.75, 5.25
    driveAmt.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 8.32f, size * 0.8f, size * 0.8f);

    //speedAmt 11.75, 7.25
    speedAmt.setBounds(getWidth() * 0.05f * 11.8f, getWidth() * 0.05f * 3.95f, size * 0.8f, size * 0.8f);

    shape.setBounds(getWidth() * 0.05f * 17.05f, getWidth() * 0.05f * 2.0f, size * 0.8f, size * 0.8f);

    speedShape.setBounds(getWidth() * 0.05f * 17.05f, getWidth() * 0.05f * 3.95f, size * 0.8f, size * 0.8f);

    resShape.setBounds(speedShape.getX(), resonanceAmt.getY(), size * 0.8f, size * 0.8f);

    driveShape.setBounds(speedShape.getX(), driveAmt.getY(), size * 0.8f, size * 0.8f);

    freqIsUpUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 2.08f, size * 0.84f, size * 0.32f);
    freqIsUpDown.setBounds(freqIsUpUp.getX(), freqIsUpUp.getY() + freqIsUpUp.getHeight(), freqIsUpUp.getWidth(), size * 0.32f);

    resIsUpUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 6.25f, size * 0.84f, size * 0.32f);
    resIsUpDown.setBounds(resIsUpUp.getX(), resIsUpUp.getY() + resIsUpUp.getHeight() - 2, resIsUpUp.getWidth(), size * 0.32f);

    driveIsUpUp.setBounds(resIsUpUp.getX(), getWidth() * 0.05f * 8.5f, size * 0.84f, size * 0.32f);
    driveIsUpDown.setBounds(driveIsUpUp.getX(), driveIsUpUp.getY() + driveIsUpUp.getHeight(), driveIsUpUp.getWidth(), size * 0.32f);

    speedIsUpUp.setBounds(getWidth() * 0.05f * 14.45f, getWidth() * 0.05f * 4.1f, size * 0.84f, size * 0.32f);
    speedIsUpDown.setBounds(speedIsUpUp.getX(), speedIsUpUp.getY() + speedIsUpUp.getHeight() - 2, speedIsUpUp.getWidth(), size * 0.32f);
}