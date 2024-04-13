/*
  ==============================================================================

    Knob.h
    Created: 11 Dec 2021 5:57:05pm
    Author:  nstil

  ==============================================================================
*/

#pragma once
#include "PluginProcessor.h"

class Knob : public juce::LookAndFeel_V4{
public: 



    juce::CaretComponent* createCaretComponent(juce::Component* keyFocusOwner) override
    {
        auto caret = new juce::CaretComponent(keyFocusOwner);

        caret->setColour(juce::CaretComponent::caretColourId, juce::Colours::red);

        return caret;
    }

    struct CustomLabel : public juce::Label
    {
        static juce::String initialPressedKey;

        juce::TextEditor* createEditorComponent() override
        {
            auto* ed = juce::Label::createEditorComponent();

            ed->setJustification(juce::Justification::centred);
            ed->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
            ed->setInputRestrictions(5, "0123456789.");
            ed->setIndents(4, 0);
            ed->setCentreRelative(0.5f, 0.5f);

            return ed;
        }

        void editorShown(juce::TextEditor* editor) override
        {
            getParentComponent()->setMouseCursor(juce::MouseCursor::NoCursor);
            editor->clear();
            editor->setText(initialPressedKey);
        }

        void editorAboutToBeHidden(juce::TextEditor* ed) override
        {
            getParentComponent()->setMouseCursor(juce::MouseCursor::NormalCursor);
        }

        void resized() override
        {
            auto height = (float)getHeight();
            setFont(juce::Font(height / 4.5f));
            Label::resized();
        }
    };

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        auto localBounds = slider.getLocalBounds();

        juce::Slider::SliderLayout layout;

        layout.textBoxBounds = localBounds.withY(-1);
        layout.sliderBounds = localBounds;

        return layout;
    }

    CustomLabel* createSliderTextBox(juce::Slider& slider) override
    {
        auto* l = new CustomLabel();

        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::Label::textWhenEditingColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
        l->setInterceptsMouseClicks(false, false);
        auto w = slider.getWidth();
        
        juce::Font f;
        l->setFont(14.f);
        return l;
    }

    void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider) override{
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);

        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

        float diameter = fmin(width, height);
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = radius * 0.085f;
        auto arcRadius = radius - lineW * 1.6f;

        float centerX = x + width * 0.5;
        float centerY = y + height * 0.5;
        float rx = centerX - radius;
        float ry = centerY - radius;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    rotaryEndAngle,
                                    true);

        g.setColour(juce::Colour::fromFloatRGBA(0.4f, 0.4f, 0.4f, 1.0f));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
                               bounds.getCentreY(),
                               arcRadius,
                               arcRadius,
                               0.0f,
                               rotaryStartAngle,
                               toAngle,
                               true);
        auto gradient = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);

        g.setColour(gradient.getColourAtPosition(sliderPos));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path thumb;
        auto thumbWidth = lineW * 2.0f;
        
        thumb.addRectangle(-thumbWidth / 2, -thumbWidth / 2, thumbWidth, radius*0.95f + lineW);
        
        g.fillPath(thumb, juce::AffineTransform::rotation(toAngle + 3.12f).translated(bounds.getCentre()));
        
        g.setGradientFill(gradient);
        
        g.fillEllipse(bounds.reduced(radius*0.25f));
    }
};

class CenterKnob : public juce::LookAndFeel_V4
{
public:



    juce::CaretComponent* createCaretComponent(juce::Component* keyFocusOwner) override
    {
        auto caret = new juce::CaretComponent(keyFocusOwner);

        caret->setColour(juce::CaretComponent::caretColourId, juce::Colours::blue);

        return caret;
    }

    struct CustomLabel : public juce::Label
    {
        static juce::String initialPressedKey;

        juce::TextEditor* createEditorComponent() override
        {
            auto* ed = juce::Label::createEditorComponent();

            ed->setJustification(juce::Justification::centred);
            ed->setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
            ed->setInputRestrictions(5, "0123456789.-");
            ed->setIndents(4, 0);

            return ed;
        }

        void editorShown(juce::TextEditor* editor) override
        {
            getParentComponent()->setMouseCursor(juce::MouseCursor::NoCursor);
            editor->clear();
            editor->setText(initialPressedKey);
        }

        void editorAboutToBeHidden(juce::TextEditor* ed) override
        {
            getParentComponent()->setMouseCursor(juce::MouseCursor::NormalCursor);
        }
        //
        void resized() override
        {
            //repaint();

            /*/
            int w;
            int h;


            float initial = 30.0f;

            int targetWidth = getLocalBounds().getWidth() ;
            int targetHeight = getLocalBounds().getHeight();

            juce::String t = this->getText();

            if (t.length() < 2)
            {
                this->setText("5555", juce::NotificationType::dontSendNotification);
            }
            /*/

            auto height = (float)getHeight();
            setFont(juce::Font(height / 3.f));
            Label::resized();

            /*/
            do
            {
                initial -= 1.f;
                this->setFont(juce::Font (initial));
                w = this->getWidth();
                h = this->getHeight();

            } while (w > targetWidth || h > targetHeight );

            this->setText(t, juce::NotificationType::dontSendNotification);

            /*/
        }
        //

        /*/
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds()
        }
        /*/
    };

    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        auto localBounds = slider.getLocalBounds();

        juce::Slider::SliderLayout layout;

        layout.textBoxBounds = localBounds.withY(-1); 
        layout.textBoxBounds = layout.textBoxBounds.withSizeKeepingCentre(localBounds.getWidth() * 4 / 5, localBounds.getHeight() * 4 / 5);
        layout.sliderBounds = localBounds;

        return layout;
    }

    CustomLabel* createSliderTextBox(juce::Slider& slider) override
    {
        //auto* l = new juce::Label();
        auto* l = new CustomLabel();

        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::Label::textWhenEditingColourId, slider.findColour(juce::Slider::textBoxTextColourId));
        l->setColour(juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
        l->setInterceptsMouseClicks(false, false);
        //l->setFont(18.0f);
        
        int w;
        int h;

        float initial = 30.0f;

        auto t = l->getText();

        l->setText("66666", juce::NotificationType::dontSendNotification);

        do
        {
            initial = initial - 0.1f;
            l->setFont(initial);
            w = l->getWidth();
            h = l->getHeight();

        } while (w > slider.getWidth() / 3 || h > slider.getHeight() / 3);

        l->setSize(slider.getWidth() * 2 / 5, slider.getHeight() * 2 / 5);
        l->setText(t, juce::NotificationType::dontSendNotification);

        return l;
    }


    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        /*/
       float diameter = fmin(width, height);
       diameter *= 0.8;
       float radius = diameter * 0.5;
       float centerX = x + width * 0.5;
       float centerY = y + height * 0.5;
       float rx = centerX - radius;
       float ry = centerY - radius;
       float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

       juce::Rectangle<float> dialArea (rx, ry, diameter, diameter);
       g.setColour(juce::Colour::fromFloatRGBA(0.796, 0.705, 0.894, 1)); //center
       //g.setGradientFill(juce::ColourGradient::horizontal(juce::Colour::fromFloatRGBA(0.1, 0.1, 0.1, .5), centerY * .25, juce::Colour::fromFloatRGBA(0.15, 0.15, 0.15, .5), centerY * 2.5));
       g.fillEllipse(dialArea);
       g.setColour(juce::Colour::fromFloatRGBA(0, 0, 0, 0.5f)); //tick color

       juce::Path dialTick;
       dialTick.addRectangle(0, -radius + 3, 3.0f, radius * 0.452211);
       g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
       g.setColour(juce::Colour::fromFloatRGBA(0, 0, 0, 0)); //outline
       g.drawEllipse(rx, ry, diameter, diameter, 1.0f);
       */
        auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);

        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        //auto radius = juce::jmin(width, height) / 2.0f;


        float diameter = fmin(width, height);
        //diameter *= 0.8;
        //float radius = diameter * 0.5;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = radius * 0.085f;
        auto arcRadius = radius - lineW * 1.6f;

        float centerX = x + width * 0.5;
        float centerY = y + height * 0.5;
        float rx = centerX - radius;
        float ry = centerY - radius;


        juce::ColourGradient gradient;
        juce::Colour c;

        juce::Path backgroundArc;
        //
        backgroundArc.addCentredArc(bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    rotaryEndAngle,
                                    true);

        //backgroundArc.addCentredArc(centerX, centerY, radius*1.05f, radius*1.05f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

        g.setColour(juce::Colour::fromFloatRGBA(0.4f, 0.4f, 0.4f, 1.0f));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        //

        if (slider.getMinimum() < 0)
        {
            auto radian = juce::MathConstants<float>::twoPi - rotaryStartAngle;

            if (slider.getValue() < 0)
            {
                juce::Path valueArcMinus;
                valueArcMinus.addCentredArc(bounds.getCentreX(),
                                            bounds.getCentreY(),
                                            arcRadius,
                                            arcRadius,
                                            0.0f,
                                            0.0f,
                                            juce::jmap(sliderPos, 0.5f, 0.0f, 0.0f, -radian),
                                            true);

                gradient = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);
                c = gradient.getColourAtPosition(std::abs(sliderPos - 0.5f) / 0.5f);
                g.setColour(c);
                g.strokePath(valueArcMinus, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else
            {
                juce::Path valueArcPlus;
                valueArcPlus.addCentredArc(bounds.getCentreX(),
                                           bounds.getCentreY(),
                                           arcRadius,
                                           arcRadius,
                                           0.0f,
                                           0.0f,
                                           juce::jmap(sliderPos, 0.5f, 1.0f, 0.0f, radian),
                                           true);

                gradient = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);
                c = gradient.getColourAtPosition(std::abs(sliderPos - 0.5f) / 0.5f);
                g.setColour(c);
                g.strokePath(valueArcPlus, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }
        else
        {
            if (slider.getValue() > 0.5f)
            {

            
                auto radian = juce::MathConstants<float>::twoPi - rotaryStartAngle;
                juce::Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                                       bounds.getCentreY(),
                                       arcRadius,
                                       arcRadius,
                                       0.0f,
                                       0.0f,
                                       juce::jmap(sliderPos, 0.5f, 1.0f, 0.0f, radian),
                                       true);


                gradient = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);
                c = gradient.getColourAtPosition(std::abs(sliderPos - 0.5f) / 0.5f);
                g.setColour(c);
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
            else
            {
                auto radian = juce::MathConstants<float>::twoPi - rotaryStartAngle;
                juce::Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                                            bounds.getCentreY(),
                                            arcRadius,
                                            arcRadius,
                                            0.0f,
                                            0.0f,
                                            juce::jmap(sliderPos, 0.5f, 0.0f, 0.0f, -radian),
                                            true);

                gradient = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);

                g.setColour(gradient.getColourAtPosition(sliderPos));
                g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            }
        }

        //valueArc.addCentredArc(centerX, centerY, radius*1.05f, radius*1.05f, 0.0f, rotaryStartAngle, toAngle, true);

        //g.setColour(fill);
        //g.setColour(juce::Colours::hotpink);


        //g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path thumb;
        auto thumbWidth = lineW * 2.0f;
        g.setColour(c);
        thumb.addRectangle(-thumbWidth / 2, -thumbWidth / 2, thumbWidth, radius*0.95f + lineW);
        //thumb.addRectangle<juce::Rectangle<float>(3,4)
        //thumb.addTriangle(juce::Point<float>(x - thumbWidth / 2, y - thumbWidth / 2), juce::Point<float>(x + thumbWidth / 2, y + thumbWidth / 2), juce::Point<float>(x + radius + lineW, y + radius + lineW));
        //thumb.addTriangle(0.f, -radius - thumbWidth * 0.5f, radius, 0.0f, radius, 0.f);

        //g.setColour(juce::Colours::purple);
        //auto gradient = juce::ColourGradient();
        //juce::Colour pink = juce::Colour(2);
        //juce::ColourGradient gradient2(pink, (x, y), juce::Colours::purple, (x + width, y + height), true);
        //gradient.isRadial = true;
        //gradient.addColour(0, juce::Colours::purple);
        //gradient.addColour(1, juce::Colours::pink);
        //g.setColour(gradient);
        //g.setGradientFill(gradient);
        g.fillPath(thumb, juce::AffineTransform::rotation(toAngle + 3.12f).translated(bounds.getCentre()));

        //diameter *= 0.8;

        //g.setColour(juce::Colours::purple);
        auto gradient2 = juce::ColourGradient(juce::Colours::purple, centerX, centerY, juce::Colours::hotpink, centerX + radius * 0.9f, centerY + radius * 0.9f, true);
        g.setGradientFill(gradient2);

        g.fillEllipse(bounds.reduced(radius * 0.25f));
        //g.fillEllipse(rx, ry, diameter, diameter);
        //g.drawEllipse(rx, ry, diameter, diameter, 1.0f);
    }

    //void mouseDown
    //void drawDrawableButton
};