/*
  ==============================================================================

    MagicKnob.cpp
    Created: 16 Dec 2021 11:37:55am
    Author:  nstil

  ==============================================================================
*/

#include "MagicKnob.h"
#include "PluginEditor.h"

MagicKnob::MagicKnob(juce::String tooltip, juce::String paramID_)
	:paramID(paramID_)
{
	setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
	setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
						juce::MathConstants<float>::pi * 2.75f,
						true);
	setVelocityBasedMode(true);
	setVelocityModeParameters(0.89, 1, 0.1, false);
	setWantsKeyboardFocus(false);
	onValueChange = [&]()
	{
		if (getValue() < 10)
			setNumDecimalPlacesToDisplay(2);
		else if (10 <= getValue() && getValue() < 100)
			setNumDecimalPlacesToDisplay(1);
		else
			setNumDecimalPlacesToDisplay(0);
	};
	setLookAndFeel(&knob);
	setVelocityBasedMode(true);
	setVelocityModeParameters(0.5, 1, 0.09, false);
	setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
						juce::MathConstants<float>::pi * 2.75f,
						true);
	setTooltip(tooltip);
	setWantsKeyboardFocus(true);
}

MagicKnob::~MagicKnob()
{
	setLookAndFeel(nullptr);
}

void MagicKnob::mouseDrag(const juce::MouseEvent& event)
{
	juce::Slider::mouseDrag(event);

	if (event.mods.isShiftDown())
		setVelocityModeParameters(0.1, 1, 0.1, false);
	else
		setVelocityModeParameters(0.89, 1, 0.1, false);
}
//
bool MagicKnob::keyPressed(const juce::KeyPress& k)
{
	char numChars[] = "-.0123456789";

	for (auto numChar : numChars)
	{
		if (k.isKeyCode(numChar))
		{
			CenterKnob::CustomLabel::initialPressedKey = juce::String::charToString(numChar);
			showTextBox();

			return true;
		}
	}

	return false;
}
//

void MagicKnob::mouseDown(const juce::MouseEvent& event)
{
	juce::Slider::mouseDown(event);
	if (juce::ModifierKeys::currentModifiers.isRightButtonDown())
	{
		if (MofoFilterAudioProcessorEditor* editor =
			findParentComponentOfClass<MofoFilterAudioProcessorEditor>())
		{
			editor->showHostMenuForParam(event, paramID);
		}
	}
	setMouseCursor(juce::MouseCursor::NoCursor);
}

void MagicKnob::mouseUp(const juce::MouseEvent& event)
{
	juce::Slider::mouseUp(event);

	juce::Desktop::getInstance().getMainMouseSource().setScreenPosition(event.source.getLastMouseDownPosition());
	setMouseCursor(juce::MouseCursor::NormalCursor);
}