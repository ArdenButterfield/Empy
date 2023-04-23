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
 This class draws the pretty graph in the center of the display.  
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include <math.h>
#include <array>

#include "utils.h"
#include "EmpyModel.h"
#include "LookFeel.h"

struct ShadingPath {
    juce::Path top_line;
    juce::Path fill;
};

//==============================================================================
/*
*/
class FrequencyGraph  : public juce::AnimatedAppComponent
{
public:
    FrequencyGraph();
    ~FrequencyGraph() override;

    
    void set_lines(GraphScaledLines* gsl);
    void set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c);
    
    void update() override;
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent &event) override;

private:
    bool enabled;
    
    juce::Path build_path(std::vector<floattype>& amplitudes,
                          const floattype amp_min,
                          const floattype amp_max,
                          const bool close_path);
    
    ShadingPath build_shading_path(std::vector<floattype>& amplitudes,
                                                 const floattype amp_min,
                                                 const floattype amp_max);
    
    void resize(unsigned long new_size);
    
    juce::Path input_line, threshold_line, bias_line, static_line, dynamic_line, spread_line;
    ShadingPath output;
    
    GraphScaledLines* graphScaledLines;
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* control_parameters;
    
    
    std::vector<floattype> x_positions;
    int leftmost_x_position;
    
    const juce::Colour BACKGROUND_COLOR = juce::Colour(0xffebeae8);
    const juce::Colour INPUT_COLOR = juce::Colour(0xffa9b3a1);
    const juce::Colour OUTPUT_LINE_COLOR = juce::Colour(0xff2e2edb);
    const juce::Colour OUTPUT_SHADING_COLOR = juce::Colour(0xff2e2edb);
    const juce::Colour OUTLINE_COLOR = juce::Colour(0xff0a0a0a);
    const juce::Colour BIAS_COLOR = juce::Colour(0xffff6e3d);
    const juce::Colour THRESHOLD_COLOR = juce::Colour(0xff9e7408);
    
    const floattype db_max = 6.0;
    const floattype db_min = -100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyGraph)
};
