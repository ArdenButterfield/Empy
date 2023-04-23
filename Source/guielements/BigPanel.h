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

#include "StickBlinker.h"
#include "../LookFeel.h"
//==============================================================================
/*
*/
class EmpyGUIComponent : public juce::Component
{
public:
    EmpyGUIComponent()
    {}
    
    const juce::Font main_font = juce::Font(EmpyLookAndFeel().main_font);
    const juce::Font bold_font = juce::Font(EmpyLookAndFeel().bold_font);
    const juce::Font tooltip_font = juce::Font(EmpyLookAndFeel().tooltip_font);
    const juce::Font title_font = main_font.withHeight(60.f);
    const juce::Font H1_font = main_font.withHeight(21.f);
    const juce::Font H2_font = main_font.withHeight(17.f);
    const juce::Font H3_font = main_font.withHeight(14.f);
    
    const juce::Colour PANEL_BACKGROUND_COLOR = EmpyLookAndFeel().PANEL_BACKGROUND_COLOR;
    const juce::Colour BEVEL_WHITE = EmpyLookAndFeel().BEVEL_WHITE;
    const juce::Colour BEVEL_LIGHT = EmpyLookAndFeel().BEVEL_LIGHT;
    const juce::Colour BEVEL_DARK = EmpyLookAndFeel().BEVEL_DARK;
    const juce::Colour BEVEL_BLACK = EmpyLookAndFeel().BEVEL_BLACK;
    
    const juce::Colour TEXT_COLOR = juce::Colours::black;
    const juce::Colour BORDER_COLOR = juce::Colours::darkblue;
    
    void prepare_background(juce::Graphics& g);
    juce::Rectangle<int> draw_beveled_rectangle(juce::Graphics& g, juce::Rectangle<int> rect, bool raised);

protected:
    juce::Rectangle<int> setUsableBounds();
    juce::Rectangle<int> usable_bounds;
    const int LINE_WIDTH = 2; // For bevel
};

class LeftPanel  : public EmpyGUIComponent
{
public:
    LeftPanel() : SIDE_KNOB_HEIGHT ((getHeight() - TITLE_SECTION_HEIGHT)/3) {}
    
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void set_sliders(juce::Slider* mask_distance,
                     juce::Slider* speed,
                     juce::Slider* gate_strength,
                     juce::Slider* dynamic_amount,
                     juce::Slider* static_amount,
                     juce::Slider* curve);
    
private:
    juce::Rectangle<int> header;
    juce::Rectangle<int> dynamic_section;
    juce::Rectangle<int> static_section;
    juce::Rectangle<int> dynamic_header;
    juce::Rectangle<int> static_header;
    juce::Rectangle<int> dynamic_left_column;
    juce::Rectangle<int> dynamic_right_column;
    juce::Rectangle<int> mask_distance_box;
    juce::Rectangle<int> speed_box;
    juce::Rectangle<int> gate_strength_box;
    juce::Rectangle<int> dynamic_amount_box;
    juce::Rectangle<int> static_amount_box;
    juce::Rectangle<int> static_curve_box;
    
    const int TITLE_SECTION_HEIGHT = 40;
    const int SUBHEADING_HEIGHT = 50;
    const int H3_HEIGHT = 20;
    const int SLIDERBOX_WIDTH = 80;
    const int SIDE_KNOB_HEIGHT;
    
    juce::Slider* masking_distance_slider;
    juce::Slider* speed_slider;
    juce::Slider* gate_strength_slider;
    juce::Slider* dynamic_amount_slider;
    juce::Slider* static_amount_slider;
    juce::Slider* curve_slider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LeftPanel)
};


class MiddlePanel  : public EmpyGUIComponent
{
public:
    MiddlePanel() {}
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void set_sliders(juce::Slider* bias);
    
private:
    juce::Rectangle<int> header;
    juce::Rectangle<int> slider_area;
    
    juce::Slider* bias_slider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiddlePanel)
};


class RightPanel  : public EmpyGUIComponent
{
public:
    RightPanel();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void set_sliders(juce::Slider* quantization,
                     juce::Slider* stick_prob,
                     juce::Slider* stick_length,
                     juce::Slider* mix);
    
    StickBlinker stickBlinker;
    
private:
    juce::Rectangle<int> quantization_window;
    juce::Rectangle<int> stick_window;
    juce::Rectangle<int> mix_window;
    juce::Rectangle<int> stick_header;
    juce::Rectangle<int> mix_header;
    juce::Rectangle<int> stick_prob_window;
    juce::Rectangle<int> stick_length_window;
    
    const int QUANTIZATION_WINDOW_HEIGHT = 125;
    const int MIX_WINDOW_HEIGHT = 125;
    
    juce::Slider* quantization_slider;
    juce::Slider* stick_prob_slider;
    juce::Slider* stick_length_slider;
    juce::Slider* mix_slider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RightPanel)
};

class TitlePanel  : public EmpyGUIComponent
{
public:
    TitlePanel() {}
    
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitlePanel)
};

class InfoPanel  : public EmpyGUIComponent
{
public:
    InfoPanel();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void change_text(std::string* new_name, std::string* new_description);
    void setNoText();

private:
    const juce::Font name_font = bold_font.withHeight(20.f);
    const juce::Font description_font = tooltip_font.withHeight(15.f);
    
    juce::Label name;
    juce::Label description;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoPanel)
};

class FrequencyResolutionPanel  : public EmpyGUIComponent
{
public:
    FrequencyResolutionPanel() {}
    
    void paint (juce::Graphics& g);
    void set_combobox(juce::ComboBox* resolution);
    void resized();

private:
    juce::Rectangle<int> title_section;
    juce::Rectangle<int> combobox_bounds;
    
    juce::ComboBox* resolution_combobox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyResolutionPanel)
};
