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
 The Plugin Processor is the central class of the plugin. It creates the control pararameter array, the EmpyModel object (where the sound processing happens). Both of these are created at initialization. It also creates the Plugin Editor, to which it passes a referene
 to the control parameters, and to itself.
 
 Before processing each block of audio, the Plugin Processor calls the update_parameters function, which passes the current parameter values to the EmpyModel, so that it can update its algorithmic parameters accordingly.
 */


#pragma once

#include <JuceHeader.h>
#include "EmpyModel.h"
#include "ControlParameter.h"
#include <array>
#include <memory>
#include <algorithm>

#include "utils.h"


class EmpyAudioProcessor  : public juce::AudioProcessor
{
public:
    EmpyAudioProcessor();
    ~EmpyAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS> control_parameters;
    
    EmpyModel empyModel;

private:
    void update_parameters();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EmpyAudioProcessor)
};
