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

#include <JuceHeader.h>
#include "FrequencyGraph.h"

FrequencyGraph::FrequencyGraph()
{
    setFramesPerSecond (30);
    x_positions.resize(0);
    enabled = true;
    
}

FrequencyGraph::~FrequencyGraph()
{
    
}

void FrequencyGraph::mouseDoubleClick (const juce::MouseEvent &event)
{
    enabled = !enabled;
}

void FrequencyGraph::resize(unsigned long new_size)
{
    x_positions.resize(new_size, 0.f);
    floattype window_w = (floattype)getWidth();
    
    if (new_size >= 64) {
        // Log scale for larger mdct sizes
        // TODO: it could be nice to include the low values that are being cut off here. Maybe some sort of linear transforming into log scale thing?
        int leftmost_line = (int)new_size / 256 + 1;
        int rightmost_line = (int)new_size;
        
        floattype m = (floattype)window_w / std::log((floattype)rightmost_line / (floattype)leftmost_line);
        
        for (int l = 0; l < new_size; ++l) {
            if (l < leftmost_line) {
                x_positions[l] = -1;
            } else {
                x_positions[l] = std::log((floattype)l / (floattype) leftmost_line) * m;
            }
        }
    } else {
        // Linear scale for smaller mdct sizes
        for (int l = 0; l < new_size; ++l) {
            x_positions[l] = window_w * (floattype)l / (new_size - 1);
        }
    }
    for (int l = 0; l < new_size; ++l) {
        if (x_positions[l] >= 0) {
            leftmost_x_position = l;
            break;
        }
    }
}

juce::Path FrequencyGraph::build_path(std::vector<floattype>& amplitudes,
                                      const floattype amp_min,
                                      const floattype amp_max,
                                      const bool close_path)
{
    floattype window_h = (floattype)getHeight();
    floattype x,y;
    
    if (amplitudes.size() != x_positions.size()) {
        resize(amplitudes.size());
    }
    
    juce::Path path;
    
    const floattype rescaler = 1 / (amp_max - amp_min);
    x = x_positions[leftmost_x_position];
    y = window_h - window_h * ((amplitudes[leftmost_x_position] - amp_min) * rescaler);
    
    // From the juce manual:
    // If you're about to add a large number of lines or curves to the path, it
    // can make the task much more efficient to call this first and avoid costly
    // reallocations as the structure grows.... each lineTo() or startNewSubPath()
    // will require 3 coords (x, y and a type marker).... Closing a sub-path
    // will require 1.
    int space_needed = 3 + (amplitudes.size() - leftmost_x_position * 1);
    
    if (close_path) {
        space_needed += 7;
    }
    
    path.preallocateSpace(space_needed);
    
    if (close_path) {
        path.startNewSubPath(x, window_h);
        path.lineTo(x, y);
    } else {
        path.startNewSubPath(x, y);
    }
    for (int f = leftmost_x_position; f < amplitudes.size(); ++f) {
        
        x = x_positions[f];
        y = window_h - window_h * ((amplitudes[f] - amp_min) * rescaler);
        y = std::max((floattype)0.0, std::min(y, window_h));
        path.lineTo(x, y);
    }
    if (close_path) {
        path.lineTo(x, window_h);
        path.closeSubPath();
    }
    return path;
}

ShadingPath FrequencyGraph::build_shading_path(std::vector<floattype>& amplitudes,
                                      const floattype amp_min,
                                      const floattype amp_max)
{
    ShadingPath shadingPath;
    
    floattype window_h = (floattype)getHeight();
    floattype x,y;
    
    if (amplitudes.size() != x_positions.size()) {
        resize(amplitudes.size());
    }
        
    const floattype rescaler = 1 / (amp_max - amp_min);
    bool path_started = false;
    
    // This may be too much or not enough space.
    int space_needed = 3 * amplitudes.size() - leftmost_x_position;
    shadingPath.fill.preallocateSpace(space_needed);
    shadingPath.top_line.preallocateSpace(space_needed);
    
    for (int f = leftmost_x_position; f < amplitudes.size(); ++f) {
        if (amplitudes[f] < amp_min) {
            if (path_started) {
                shadingPath.fill.lineTo(x, window_h);
                shadingPath.fill.closeSubPath();
            }
            path_started = false;
            continue;
        }
        x = x_positions[f];
        y = window_h - window_h * (amplitudes[f] - amp_min) * rescaler;
        y = std::max((floattype)0.0, std::min(y, window_h));
        
        if (!path_started) {
            shadingPath.fill.startNewSubPath(x,window_h);
            shadingPath.fill.lineTo(x, y);
            shadingPath.top_line.startNewSubPath(x, y);
            path_started = true;
        } else {
            shadingPath.fill.lineTo(x, y);
            shadingPath.top_line.lineTo(x, y);
        }
            
    }
    if (path_started) {
        shadingPath.fill.lineTo(x, window_h);
        shadingPath.fill.closeSubPath();
    }
    return shadingPath;
}


void FrequencyGraph::paint (juce::Graphics& g)
{
    
    
    if (enabled) {
        g.fillAll (BACKGROUND_COLOR);
        
        g.setColour(INPUT_COLOR);
        g.fillPath(input_line);
            
        g.setColour(OUTPUT_SHADING_COLOR);
        g.fillPath(output.fill);
        
        g.setColour(OUTPUT_LINE_COLOR);
        g.strokePath(output.top_line, juce::PathStrokeType(1.0));
        
        g.setColour(THRESHOLD_COLOR);
        g.strokePath(threshold_line, juce::PathStrokeType(2.0));
        
        g.setColour(BIAS_COLOR);

        if ((*control_parameters)[8].focused) {
            g.strokePath(bias_line, juce::PathStrokeType (2.0));
        }
        
        else if ((*control_parameters)[0].focused || (*control_parameters)[4].focused) {
            g.strokePath(dynamic_line, juce::PathStrokeType (2.0));
        }
        
        else if ((*control_parameters)[1].focused || (*control_parameters)[9].focused) {
            g.strokePath(static_line, juce::PathStrokeType (2.0));
        }
        
        else if ((*control_parameters)[2].focused) {
            g.strokePath(spread_line, juce::PathStrokeType (2.0));
        }
    } else {
        g.fillAll(EmpyLookAndFeel().BEVEL_LIGHT);
        g.setFont(juce::Font(EmpyLookAndFeel().main_font));
        g.drawText ("double-click to enable metering", getLocalBounds(), juce::Justification::centred, true);
    }
    
    g.setColour (OUTLINE_COLOR);
    g.drawRect (getLocalBounds(), 2);
    

}

void FrequencyGraph::resized()
{
    
}

void FrequencyGraph::update()
{
    input_line = build_path(graphScaledLines->input,
                            db_min,
                            db_max,
                            true);
    output = build_shading_path(graphScaledLines->output,
                                db_min,
                                db_max);
    threshold_line = build_path(graphScaledLines->threshold,
                                db_min,
                                db_max,
                                false);
    if ((*control_parameters)[8].focused) {
        bias_line = build_path(graphScaledLines->bias,
                               -60,
                               60,
                               false);
    }
    
    else if ((*control_parameters)[0].focused || (*control_parameters)[4].focused) {
        dynamic_line = build_path(graphScaledLines->dynamic_threshold,
                                  db_min,
                                  db_max,
                                  false);
    }
    
    else if ((*control_parameters)[1].focused || (*control_parameters)[9].focused) {
        static_line = build_path(graphScaledLines->static_threshold,
                                 db_min,
                                 db_max,
                                 false);
    }
    
    else if ((*control_parameters)[2].focused) {
        spread_line = build_path(graphScaledLines->spread,
                                 db_min,
                                 db_max,
                                 false);
    }

}

void FrequencyGraph::set_lines(GraphScaledLines *gsl)
{
    graphScaledLines = gsl;
    
}

void FrequencyGraph::set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS> *c)
{
    control_parameters = c;
}
