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

#include "BigPanel.h"

juce::Rectangle<int> EmpyGUIComponent::setUsableBounds()
{
    usable_bounds = getLocalBounds()
        .withTrimmedBottom(LINE_WIDTH * 2)
        .withTrimmedRight(LINE_WIDTH * 2)
        .withTrimmedTop(LINE_WIDTH * 2)
        .withTrimmedLeft(LINE_WIDTH * 2);
}

juce::Rectangle<int> EmpyGUIComponent::draw_beveled_rectangle(juce::Graphics& g, juce::Rectangle<int> rect, bool raised)
{
    // Returns the rectangle inside of the bevel.
    
    juce::Colour top_outer, top_inner, bottom_outer, bottom_inner;
    if (raised) {
        top_outer = BEVEL_WHITE;
        top_inner = BEVEL_LIGHT;
        bottom_inner = BEVEL_DARK;
        bottom_outer = BEVEL_BLACK;
    } else {
        top_outer = BEVEL_BLACK;
        top_inner = BEVEL_DARK;
        bottom_inner = BEVEL_LIGHT;
        bottom_outer = BEVEL_WHITE;
    }
    
    g.setColour(bottom_outer);
    g.fillRect(rect);
    
    rect = rect.withTrimmedBottom(LINE_WIDTH).withTrimmedRight(LINE_WIDTH);
    g.setColour(top_outer);
    g.fillRect(rect);
    
    rect = rect.withTrimmedTop(LINE_WIDTH).withTrimmedLeft(LINE_WIDTH);
    g.setColour(bottom_inner);
    g.fillRect(rect);
    
    rect = rect.withTrimmedBottom(LINE_WIDTH).withTrimmedRight(LINE_WIDTH);
    g.setColour(top_inner);
    g.fillRect(rect);
    
    rect = rect.withTrimmedTop(LINE_WIDTH).withTrimmedLeft(LINE_WIDTH);
    g.setColour(PANEL_BACKGROUND_COLOR);
    g.fillRect(rect);
    
    return rect;
}

void EmpyGUIComponent::prepare_background(juce::Graphics& g)
{
    draw_beveled_rectangle(g, getLocalBounds(), false);
}


// LEFT PANEL

void LeftPanel::paint (juce::Graphics& g)
{
    prepare_background(g);

    g.setColour (TEXT_COLOR);
    g.setFont(H1_font);
    g.drawText ("Threshold", header, juce::Justification::centred, true);
    g.drawHorizontalLine(header.getBottom(),header.getX(),header.getRight());
    g.drawVerticalLine(dynamic_section.getRight(),dynamic_section.getY(),dynamic_section.getBottom());
    g.drawHorizontalLine(dynamic_header.getBottom(),dynamic_header.getX(),dynamic_header.getRight());
    g.drawHorizontalLine(static_header.getBottom(),static_header.getX(),static_header.getRight());
    
    g.setFont(H2_font);
    g.drawText("Dynamic", dynamic_header, juce::Justification::centred, true);
    g.drawText("Static",static_header, juce::Justification::centred, true);
    
    g.drawVerticalLine(dynamic_left_column.getRight(),dynamic_left_column.getY(),dynamic_left_column.getBottom());
    
    g.drawHorizontalLine(mask_distance_box.getBottom(), mask_distance_box.getX(), mask_distance_box.getRight());
    g.drawHorizontalLine(speed_box.getBottom(), speed_box.getX(), speed_box.getRight());
    
    g.setFont(H3_font);
    g.drawText("Smoothness",
               mask_distance_box.withHeight(H3_HEIGHT),
               juce::Justification::centredBottom, true);
    g.drawText("Speed",
               speed_box.withHeight(H3_HEIGHT),
               juce::Justification::centredBottom, true);
    g.drawText("Ratio",
               gate_strength_box.withHeight(H3_HEIGHT),
               juce::Justification::centredBottom, true);
}

void LeftPanel::resized()
{
    setUsableBounds();
    header = usable_bounds.withHeight(TITLE_SECTION_HEIGHT);
    
    dynamic_section = usable_bounds.withTrimmedTop(TITLE_SECTION_HEIGHT).withTrimmedRight(SLIDERBOX_WIDTH);
    static_section = usable_bounds.withTrimmedTop(TITLE_SECTION_HEIGHT).withTrimmedLeft(dynamic_section.getWidth());
    dynamic_header = dynamic_section.withHeight(SUBHEADING_HEIGHT);
    static_header = static_section.withHeight(SUBHEADING_HEIGHT);
    dynamic_left_column = dynamic_section.withTrimmedTop(SUBHEADING_HEIGHT).withTrimmedRight(SLIDERBOX_WIDTH);
    dynamic_right_column = dynamic_section.withTrimmedTop(SUBHEADING_HEIGHT).withTrimmedLeft(dynamic_left_column.getWidth());
    int littlebox_height = dynamic_left_column.getHeight() / 3;
    mask_distance_box = dynamic_left_column.withHeight(littlebox_height);
    speed_box = mask_distance_box.translated(0, littlebox_height);
    gate_strength_box = speed_box.translated(0, littlebox_height);
    
    int curve_box_height = static_header.getWidth();
    static_amount_box = static_section.withTrimmedTop(SUBHEADING_HEIGHT).withTrimmedBottom(curve_box_height);
    static_curve_box = static_section.withTrimmedTop(SUBHEADING_HEIGHT).withTrimmedTop(static_amount_box.getHeight());
    dynamic_amount_box = dynamic_right_column.withTrimmedBottom(curve_box_height);
    
    masking_distance_slider->setBounds(mask_distance_box.withTrimmedTop(H3_HEIGHT));
    speed_slider->setBounds(speed_box.withTrimmedTop(H3_HEIGHT));
    gate_strength_slider->setBounds(gate_strength_box.withTrimmedTop(H3_HEIGHT));
    
    dynamic_amount_slider->setBounds(dynamic_amount_box.withTrimmedTop(10).withTrimmedBottom(10));
    static_amount_slider->setBounds(static_amount_box.withTrimmedTop(10).withTrimmedBottom(10));
    curve_slider->setBounds(static_curve_box);
    
}

void LeftPanel::set_sliders(juce::Slider* mask_distance,
                 juce::Slider* speed,
                 juce::Slider* gate_strength,
                 juce::Slider* dynamic_amount,
                 juce::Slider* static_amount,
                 juce::Slider* curve)
{
    masking_distance_slider = mask_distance;
    speed_slider = speed;
    gate_strength_slider = gate_strength;
    dynamic_amount_slider = dynamic_amount;
    static_amount_slider = static_amount;
    curve_slider = curve;
    
    masking_distance_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    speed_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gate_strength_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    dynamic_amount_slider->setSliderStyle(juce::Slider::LinearVertical);
    static_amount_slider->setSliderStyle(juce::Slider::LinearVertical);
    
    curve_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    masking_distance_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    speed_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    gate_strength_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    dynamic_amount_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    static_amount_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    curve_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    
    masking_distance_slider->setNumDecimalPlacesToDisplay(2);
    speed_slider->setNumDecimalPlacesToDisplay(3);
    gate_strength_slider->setNumDecimalPlacesToDisplay(2);
    dynamic_amount_slider->setNumDecimalPlacesToDisplay(3);
    static_amount_slider->setNumDecimalPlacesToDisplay(3);
    curve_slider->setNumDecimalPlacesToDisplay(3);
    
    addAndMakeVisible(masking_distance_slider);
    addAndMakeVisible(speed_slider);
    addAndMakeVisible(gate_strength_slider);
    addAndMakeVisible(dynamic_amount_slider);
    addAndMakeVisible(static_amount_slider);
    addAndMakeVisible(curve_slider);
}

// MIDDLE PANEL

void MiddlePanel::paint (juce::Graphics& g)
{
    prepare_background(g);
    g.setColour (TEXT_COLOR);
    g.setFont(H1_font);
    g.drawText ("Bias", header, juce::Justification::centred, true);
}

void MiddlePanel::resized()
{
    setUsableBounds();
    header = usable_bounds.withHeight(40);
    slider_area = usable_bounds.withTrimmedTop(30);
    bias_slider->setBounds(slider_area.withTrimmedLeft(10).withTrimmedRight(10));
}

void MiddlePanel::set_sliders(juce::Slider* bias)
{
    bias_slider = bias;
    bias_slider->setSliderStyle(juce::Slider::LinearHorizontal);
    bias_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    bias_slider->setNumDecimalPlacesToDisplay(3);
    addAndMakeVisible(bias_slider);
}

// RIGHT PANEL

RightPanel::RightPanel()
{
    addAndMakeVisible(stickBlinker);
}

void RightPanel::paint (juce::Graphics& g)
{
    prepare_background(g);
    g.setColour (TEXT_COLOR);
    g.setFont(H2_font);
    g.drawHorizontalLine(quantization_window.getBottom(), quantization_window.getX(), quantization_window.getRight());
    g.drawHorizontalLine(stick_window.getBottom(), stick_window.getX(), stick_window.getRight());
    
    g.drawText("Quantization", quantization_window.withTrimmedTop(5).withHeight(20), juce::Justification::centredBottom, true);
    g.drawText("Mix", mix_header, juce::Justification::centredBottom, true);
    g.drawText("Stick", stick_header, juce::Justification::centredBottom, true);
    
    g.setFont(H3_font);
    g.drawText("Prob",
               stick_prob_window.withHeight(15),
               juce::Justification::centredBottom,
               true);
    
    g.drawText("Length",
               stick_length_window.withHeight(15),
               juce::Justification::centredBottom,
               true);
}

void RightPanel::resized()
{
    setUsableBounds();
    quantization_window = usable_bounds.withHeight(QUANTIZATION_WINDOW_HEIGHT);
    stick_window = usable_bounds
        .withTrimmedTop(QUANTIZATION_WINDOW_HEIGHT)
        .withTrimmedBottom(MIX_WINDOW_HEIGHT);
    mix_window = usable_bounds.withTop(usable_bounds.getBottom() - MIX_WINDOW_HEIGHT);
    mix_header = mix_window.withHeight(25);
    stick_header = stick_window.withHeight(25);
    
    auto stick_slider_area = stick_window.withTrimmedTop(stick_header.getHeight());
    stick_prob_window = stick_slider_area.withTrimmedRight(stick_slider_area.getWidth()/2);
    stick_length_window = stick_slider_area.withTrimmedLeft(stick_slider_area.getWidth()/2);
    quantization_slider->setBounds(quantization_window.withTrimmedTop(25));
    stick_prob_slider->setBounds(stick_prob_window.withTrimmedTop(15));
    stick_length_slider->setBounds(stick_length_window.withTrimmedTop(15));
    
    mix_slider->setBounds(mix_window.withTrimmedTop(mix_header.getHeight()));
    
    stickBlinker.setBounds(stick_slider_area.withSizeKeepingCentre(10, 10).withY(stick_slider_area.getBottom() - 20));
}

void RightPanel::set_sliders(juce::Slider* quantization,
                             juce::Slider* stick_prob,
                             juce::Slider* stick_length,
                             juce::Slider* mix)
{
    quantization_slider = quantization;
    stick_prob_slider = stick_prob;
    stick_length_slider = stick_length;
    mix_slider = mix;
    
    quantization_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    stick_prob_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    stick_length_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mix_slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    quantization_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    stick_prob_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    stick_length_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    mix_slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
    
    quantization_slider->setNumDecimalPlacesToDisplay(2);
    stick_prob_slider->setNumDecimalPlacesToDisplay(3);
    stick_length_slider->setNumDecimalPlacesToDisplay(3);
    mix_slider->setNumDecimalPlacesToDisplay(2);
    
    addAndMakeVisible(quantization_slider);
    addAndMakeVisible(stick_prob_slider);
    addAndMakeVisible(stick_length_slider);
    addAndMakeVisible(mix_slider);
}

// TITLE PANEL

void TitlePanel::paint (juce::Graphics& g)
{
    auto inner_rect = draw_beveled_rectangle(g, getLocalBounds(), true);
    auto gradient = juce::ColourGradient::horizontal(juce::Colours::darkblue,
                                                     juce::Colours::lightblue,
                                                     inner_rect);
    g.setGradientFill(gradient);
    g.fillRect(inner_rect);


    g.setColour (juce::Colours::white);
    g.setFont(title_font);
    g.drawText ("Empy", inner_rect,
                juce::Justification::centredLeft, true);
}

void TitlePanel::resized()
{
    
}

// INFO PANEL

InfoPanel::InfoPanel()
{
    std::string new_name = std::string("");
    std::string new_description = std::string("");
    
    change_text(&new_name, &new_description);
    
    name.setColour(juce::Label::textColourId, TEXT_COLOR);
    name.setFont(name_font);
    name.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (name);
    
    description.setColour(juce::Label::textColourId, TEXT_COLOR);
    description.setFont (description_font);
    description.setJustificationType (juce::Justification::centred);
    description.setMinimumHorizontalScale(1.0); // Don't squash text
    addAndMakeVisible (description);
}

void InfoPanel::paint (juce::Graphics& g)
{
    prepare_background(g);
    
}

void InfoPanel::resized()
{
    setUsableBounds();
    name.setBounds(usable_bounds.withTrimmedTop(5).withHeight(20));
    description.setBounds(usable_bounds
                          .withTrimmedTop(5)
                          .withTrimmedTop(20));

}

void InfoPanel::change_text(std::string* new_name, std::string* new_description)
{
    name.setText(*new_name, juce::dontSendNotification);
    description.setText(*new_description, juce::dontSendNotification);
}

void InfoPanel::setNoText()
{
    name.setText("", juce::dontSendNotification);
    description.setText("", juce::dontSendNotification);
}

// FREQUENCY RESOLUTION PANEL

void FrequencyResolutionPanel::paint (juce::Graphics& g)
{
    prepare_background(g);

    g.setColour (TEXT_COLOR);
    g.setFont(H2_font);
    g.drawText ("Frequency Resolution", title_section,
                juce::Justification::centred, true);
}

void FrequencyResolutionPanel::set_combobox(juce::ComboBox* resolution)
{
    resolution_combobox = resolution;
    addAndMakeVisible(resolution_combobox);
}

void FrequencyResolutionPanel::resized()
{
    setUsableBounds();
    title_section = usable_bounds.withTrimmedRight(110);
    combobox_bounds = usable_bounds.withTrimmedLeft(title_section.getWidth());
    resolution_combobox->setBounds(combobox_bounds.withSizeKeepingCentre(combobox_bounds.getWidth() - 10,
                                                                         combobox_bounds.getHeight() - 10));
}
