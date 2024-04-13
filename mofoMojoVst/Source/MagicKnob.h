/*
  ==============================================================================

    MagicKnob.h
    Created: 16 Dec 2021 11:37:55am
    Author:  nstil

  ==============================================================================
*/

#pragma once

#include "Knob.h"

class MagicKnob : public juce::Slider
{
public:
	MagicKnob(juce::String tooltip, juce::String paramID);
	~MagicKnob() override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseUp(const juce::MouseEvent& event) override;
	//void squareLayout();
	void mouseDrag(const juce::MouseEvent& event) override;
	bool keyPressed(const juce::KeyPress& k) override;

private:
	juce::String paramID;
	Knob knob;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagicKnob)
};