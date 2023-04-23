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

/**
 The Plugin Editor is a core component of a JUCE plugin, along with the Plugin Processor. Broadly speaking, a plugin editor object is created when the plugin window is opened, and is destroyed when the plugin window is closed. As such, sometimes the plugin will be running with an EmpyAudioProcessorEditor, and sometimes without.
 
 This class controls the GUI of the plugin. The specific layout is mostly handled by the panel children of the Plugin Editor.
 
 The Plugin Editor updates the parameters when a controller is moved, and updates the controller when a parameter is changed from elsewhere (for instance, when the parameter is automated.) In future versions of this pluign, it might be wise to convert over to using the AudioProcessorValueTreeState class to manage the interaction between sliders and audio parameters.
 */
#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "FrequencyGraph.h"
#include "utils.h"
#include "guielements/BigPanel.h"
#include "ControllerListener.h"
#include "LookFeel.h"
#include "guielements/StickBlinker.h"

class EmpyAudioProcessorEditor:
        public juce::AudioProcessorEditor,
        public juce::Slider::Listener,
        public juce::ComboBox::Listener,
        public juce::Timer
{
public:
    EmpyAudioProcessorEditor (EmpyAudioProcessor& p, std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c);
    ~EmpyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider*) override;
    void comboBoxChanged(juce::ComboBox*) override;
    void timerCallback() override;
     
private:
    
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* control_parameters;
    FrequencyGraph frequencyGraph;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EmpyAudioProcessor& audioProcessor;
    
    LeftPanel leftPanel;
    MiddlePanel middlePanel;
    RightPanel rightPanel;
    TitlePanel titlePanel;
    InfoPanel infoPanel;
    FrequencyResolutionPanel frequencyResolutionPanel;
    
    std::unique_ptr<ControllerListener> controllerListener;
    
    EmpyLookAndFeel empyLookAndFeel;
    
    void check_active();
    void set_active(ControlParameter* c, bool active);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmpyAudioProcessorEditor)
};
