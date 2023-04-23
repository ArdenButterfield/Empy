/*
Copyright (C) 2023  Arden Butterfield

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <JuceHeader.h>

#include "ControlParameter.h"

#define PI 3.14159265359

// BEDSTEAD FONT
// https://www.1001fonts.com/bedstead-font.html
// Bedtead by Ben Harris.
// Released into the public domain.
// Thanks to: https://forum.juce.com/t/modern-custom-font-guide/20841/3

class EmpyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    EmpyLookAndFeel();
    ~EmpyLookAndFeel();
    
    
    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           const float rotaryStartAngle,
                           const float rotaryEndAngle,
                           juce::Slider&) override;
    
    void drawLinearSlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& s
                          ) override;
#if 0
    void drawComboBox (juce::Graphics& g,
                       int width,
                       int height,
                       const bool isButtonDown,
                       int buttonX,
                       int buttonY,
                       int buttonW,
                       int buttonH,
                       juce::ComboBox& box) override;
#endif
    
    void set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c);
                                
    const juce::Colour PANEL_BACKGROUND_COLOR = juce::Colour(0xffd4d0c8);
    
    const juce::Colour BEVEL_WHITE = juce::Colour(0xffececec);
    const juce::Colour BEVEL_LIGHT = juce::Colour(0xffdfdfdf);
    const juce::Colour BEVEL_DARK = juce::Colour(0xff808080);
    const juce::Colour BEVEL_BLACK = juce::Colour(0xff0a0a0a);
    
    const juce::Typeface::Ptr main_font;
    const juce::Typeface::Ptr bold_font;
    const juce::Typeface::Ptr tooltip_font;
    
private:
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* controlParameters;
};
