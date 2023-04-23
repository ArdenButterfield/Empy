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

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
EmpyAudioProcessorEditor::EmpyAudioProcessorEditor (EmpyAudioProcessor& p, std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    control_parameters = c;
    
    
    for (auto &c : *control_parameters) {
        c.controller_label.setFont (juce::Font (16.0f, juce::Font::bold));
        c.controller_label.setText(c.name, juce::dontSendNotification);
        c.controller_label.setColour (juce::Label::textColourId, juce::Colours::white);
        c.controller_label.setJustificationType (juce::Justification::centredTop);
        addAndMakeVisible(c.controller_label);
        
        if (c.controller_type == slider) {
            
            c.controller = std::make_unique<juce::Slider>();
            juce::Slider* slider = static_cast<juce::Slider *>(c.controller.get());
            
            slider->setRange(c.min_val, c.max_val);
            // slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
            // slider->setTextBoxStyle(juce::Slider::TextBoxBelow , false, 60, 20);
            slider->addListener(this);
            // addAndMakeVisible(c.controller.get());
        } else {
            c.controller = std::make_unique<juce::ComboBox>();
            juce::ComboBox* cbox = static_cast<juce::ComboBox *>(c.controller.get());
            cbox->addItemList(juce::StringArray{"4","8","16","32","64","128","256","512","1024","2048","4096"}, 1);
            cbox->addListener(this);
            // addAndMakeVisible(c.controller.get());
        }
    }
    
    
    
    frequencyGraph.set_lines(&(audioProcessor.empyModel.graphScaledLines));
    addAndMakeVisible(frequencyGraph);
    addAndMakeVisible(leftPanel);
    addAndMakeVisible(middlePanel);
    addAndMakeVisible(rightPanel);
    addAndMakeVisible(titlePanel);
    addAndMakeVisible(infoPanel);
    addAndMakeVisible(frequencyResolutionPanel);
    
    juce::Slider* mask_distance = static_cast<juce::Slider *>((*control_parameters)[2].controller.get());
    juce::Slider* speed = static_cast<juce::Slider *>((*control_parameters)[4].controller.get());
    juce::Slider* gate_ratio = static_cast<juce::Slider *>((*control_parameters)[11].controller.get());
    juce::Slider* dynamic_amount = static_cast<juce::Slider *>((*control_parameters)[0].controller.get());
    juce::Slider* static_amount = static_cast<juce::Slider *>((*control_parameters)[1].controller.get());
    juce::Slider* curve = static_cast<juce::Slider *>((*control_parameters)[9].controller.get());
    
    gate_ratio->setSkewFactorFromMidPoint(10.0);
    
    leftPanel.set_sliders(mask_distance,
                          speed,
                          gate_ratio,
                          dynamic_amount,
                          static_amount,
                          curve);
    
    juce::Slider* quantization = static_cast<juce::Slider *>((*control_parameters)[3].controller.get());
    juce::Slider* stick_prob = static_cast<juce::Slider *>((*control_parameters)[6].controller.get());
    juce::Slider* stick_length = static_cast<juce::Slider *>((*control_parameters)[7].controller.get());
    juce::Slider* mix = static_cast<juce::Slider *>((*control_parameters)[10].controller.get());
    
    stick_length->setSkewFactorFromMidPoint(0.3);
    
    rightPanel.set_sliders(quantization,
                           stick_prob,
                           stick_length,
                           mix);
    
    juce::Slider* bias_slider = static_cast<juce::Slider *>((*control_parameters)[8].controller.get());
    middlePanel.set_sliders(bias_slider);
    
    juce::ComboBox* resolution_combobox = static_cast<juce::ComboBox *>((*control_parameters)[5].controller.get());
    frequencyResolutionPanel.set_combobox(resolution_combobox);
    
    controllerListener = std::make_unique<ControllerListener>(control_parameters, &infoPanel, &titlePanel);
    
    startTimer(100);
    setSize (760, 500);
    
    setLookAndFeel(&empyLookAndFeel);
    empyLookAndFeel.set_control_parameters(control_parameters);
    
    frequencyGraph.set_control_parameters(control_parameters);
    
    rightPanel.stickBlinker.setEmpyModel(&(audioProcessor.empyModel));
    
    timerCallback();
}

EmpyAudioProcessorEditor::~EmpyAudioProcessorEditor()
{
    // Required to avoid an error
    setLookAndFeel(nullptr);
    
    
    
    // Since our sliders are owned by the AP not the APE, they stick around longer,
    // so we need to remove the old listeners or we will run into errors trying to
    // communicate to freed memory.
    
    for (auto &c : *control_parameters) {
        if (c.controller_type == slider) {
            juce::Slider* slider = static_cast<juce::Slider *>(c.controller.get());
            slider->removeListener(this);
        } else {
            juce::ComboBox* cbox = static_cast<juce::ComboBox *>(c.controller.get());
            cbox->removeListener(this);
        }
    }
}

//==============================================================================
void EmpyAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void EmpyAudioProcessorEditor::resized()
{

    const int gutter = 10;
    const int top = gutter;
    const int bottom = getHeight() - gutter;
    const int left = gutter;
    const int right = getWidth() - gutter;
    
    const int top_part_height = 80;
    const int left_part_width = 250;
    const int right_part_width = 160;
    const int middle_panel_height = 90 - gutter; // To line up with the heading and subheading of the left panel.
    
    const int middle_part_start = left + left_part_width + gutter;
    const int bottom_part_start = top_part_height + gutter + gutter;
    const int middle_part_width = right - right_part_width - gutter - middle_part_start;
    
    titlePanel.setBounds(left, top, left_part_width, top_part_height);
    infoPanel.setBounds(middle_part_start, top, right - middle_part_start, top_part_height);
    leftPanel.setBounds(gutter, bottom_part_start, left_part_width, bottom - bottom_part_start);
    middlePanel.setBounds(middle_part_start, bottom_part_start, middle_part_width, middle_panel_height);
    rightPanel.setBounds(right - right_part_width, bottom_part_start, right_part_width, bottom - bottom_part_start);
    frequencyGraph.setBounds(middle_part_start, bottom_part_start + middlePanel.getHeight() + gutter, middle_part_width, 240);
    frequencyResolutionPanel.setBounds(middle_part_start, frequencyGraph.getBottom() + gutter, middle_part_width, bottom - (frequencyGraph.getBottom() + gutter));
}

void EmpyAudioProcessorEditor::sliderValueChanged(juce::Slider* changed_slider)
{
    juce::AudioParameterFloat* ap_float;
    for (auto &c : *control_parameters) {
        if (c.controller_type == slider) {
            juce::Slider* slider = static_cast<juce::Slider *>(c.controller.get());
            if (changed_slider == slider) {
                c.audio_parameter->beginChangeGesture();
                ap_float = static_cast<juce::AudioParameterFloat*>(c.audio_parameter);
                (*ap_float) = (float)slider->getValue();
                c.audio_parameter->endChangeGesture();
            }
        }
        
    }
}

void EmpyAudioProcessorEditor::comboBoxChanged(juce::ComboBox* changed_combobox)
{
    juce::AudioParameterChoice* ap_choice;
    for (auto &c : *control_parameters) {
        if (c.controller_type == combobox) {
            juce::ComboBox* combobox = static_cast<juce::ComboBox *>(c.controller.get());
            if (changed_combobox == combobox) {
                c.audio_parameter->beginChangeGesture();
                ap_choice = static_cast<juce::AudioParameterChoice*>(c.audio_parameter);
                (*ap_choice) = combobox->getSelectedId();
                c.audio_parameter->endChangeGesture();
            }
        }
    }
}

void EmpyAudioProcessorEditor::timerCallback()
{
    check_active();
    
    juce::AudioParameterFloat* ap_float;
    juce::AudioParameterChoice* ap_choice;
    for (auto &c : *control_parameters) {
        if (c.controller_type == combobox) {
            ap_choice = static_cast<juce::AudioParameterChoice*>(c.audio_parameter);
            juce::ComboBox* combobox = static_cast<juce::ComboBox *>(c.controller.get());
            combobox->setSelectedId(ap_choice->getIndex(), juce::dontSendNotification);
        } else {
            ap_float = static_cast<juce::AudioParameterFloat*>(c.audio_parameter);
            juce::Slider* slider = static_cast<juce::Slider *>(c.controller.get());
            slider->setValue(*ap_float, juce::dontSendNotification);
        }
    }
}

void EmpyAudioProcessorEditor::check_active()
{
    /*
     If knobs won't do anything because of the position of other knobs, we want
     to gray them out, so the user isn't confused why the knob isn't doing
     anything. When drawing a slider, the EmpyLookAndFeel class checks the
     control parameters to see if the slider is active. If not, it grays out
     the slider.
     */
    bool dynamic_happening = (static_cast<juce::AudioParameterFloat*>((*control_parameters)[0].audio_parameter)->get() != 0.0);
    bool static_happening = (static_cast<juce::AudioParameterFloat*>((*control_parameters)[1].audio_parameter)->get() != 0.0);
    bool stick_happening = (static_cast<juce::AudioParameterFloat*>((*control_parameters)[6].audio_parameter)->get() != 0.0);
    
    set_active(&(*control_parameters)[4], dynamic_happening); // speed
    set_active(&(*control_parameters)[2], dynamic_happening); // smoothness
    set_active(&(*control_parameters)[9], static_happening); // curve
    set_active(&(*control_parameters)[11], static_happening || dynamic_happening); // ratio
    set_active(&(*control_parameters)[8], static_happening || dynamic_happening); // bias
    set_active(&(*control_parameters)[7], stick_happening); // stick length

}

void EmpyAudioProcessorEditor::set_active(ControlParameter* c, bool active)
{
    if (c->active != active) {
        c->active = active;
        c->controller->repaint();
    }
}
