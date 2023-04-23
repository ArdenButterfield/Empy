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
 The empy model handles the actual processing of the sound: converting it from the time domain to the frequency domain, applying the psychoacoustic model, and reducing the bitrate accordingly. The EmpyModel object is owned by the Plugin Processor, which updates its parameters, and passes it blocks of audio to process.
 
 The changes to the sound (calculating the threshold, applying the threshold, applying stick and quantization) actually don't happen in the EmpyModel, but in the ChunkProcessor objects owned by the EmpyModel. Since the ChunkProcessor works in the frequency domain, the EmpyModel must store samples until there are enough to transform to transform to the frequency domain, using the Modified Discrete Cosine Transform. The number of samples needed for a transform varies based on the frequency resolution set by the user. This may be larger or smaller than the length of the buffer that the EmpyModel receives from the PluginProcessor. The transforms overlap by 50%.
 
 The EmpyModel also prepares arrays of scaled values that are used by the frequency graph in the GUI.
 */

#pragma once

#include <stdexcept>
#include <iostream>
#include <algorithm>    // std::min, std::max
#include <math.h> // log10
#include <stdlib.h> // rand, srand
#include <vector>
#include <array>
#include <JuceHeader.h>

#include "mdct.h"
#include "ControlParameter.h"
#include "ChunkProcessor.h"
#include "utils.h"

floattype decibel(floattype sample);

struct GilbertElliottModel {
    /**
     This class keeps track of the packet loss. This simple two state Markov Chain model is able to emulate the loss of packets being transmitted over the internet. [1] Packets are generally lost in bursts, which is represented here by two states, a state with packet loss and a state without.
     
     [1] G. Hasslinger and O. Hohlfeld, "The Gilbert-Elliott Model for Packet Loss in Real Time Services on the Internet," 14th GI/ITG Conference - Measurement, Modelling and Evalutation of Computer and Communication Systems, 2008, pp. 1-15.
     */
    floattype p;
    floattype q;
    bool in_loss_state;
    
    GilbertElliottModel() : in_loss_state(false) {}
    bool tick()
    {
        // from https://stackoverflow.com/questions/5289613/generate-random-floattype-between-two-floattypes
        floattype random = ((floattype) rand()) / (floattype) RAND_MAX;
        if (in_loss_state && (random < q)) {
            in_loss_state = false;
        } else if ((!in_loss_state) && (random < p)) {
            in_loss_state = true;
        }
        
        return in_loss_state;
    }
};

struct GraphScaledLines
{
    std::vector<floattype> input;
    std::vector<floattype> output;
    std::vector<floattype> threshold;
    std::vector<floattype> bias;
    std::vector<floattype> static_threshold;
    std::vector<floattype> dynamic_threshold;
    std::vector<floattype> spread;
    
    void resize(const int newsize)
    {
        input.resize(newsize, 0);
        output.resize(newsize, 0);
        threshold.resize(newsize, 0);
        bias.resize(newsize, 0);
        static_threshold.resize(newsize, 0);
        dynamic_threshold.resize(newsize, 0);
        spread.resize(newsize, 0);
    }
};

floattype linpower(floattype input, floattype transition_point);

class EmpyModel
{
public:
    EmpyModel();
    
    void set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c);
    void prepare(int mdct_step, floattype sample_rate, int num_channels);

    
    // Modifies the sample array, changing the input values into output values.
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    
    GraphScaledLines graphScaledLines;
    
    int MDCT_WIDTH;
    int MDCT_LINES;
    
    // These are all called by the update_parameters method of the Plugin
    // Processor, so that we can update the algorithmic parameters to match
    // any changes.
    void set_mask_threshold(const floattype new_threshold);
    void set_spread_distance(const floattype new_distance);
    void set_bit_reduction_above_threshold(const floattype new_redux);
    void set_packet_loss(const floattype probability, const floattype length, const floattype max_length);
    void set_speed(const floattype new_speed);
    void set_mdct_size(const floattype size);
    void set_absolute_threshold(const floattype new_abs_threshold);
    void set_bias(const floattype new_bias);
    void set_perceptual_curve(const floattype new_perceptual_curve);
    void set_mix(const floattype new_mix);
    void set_gate_ratio(const floattype new_strength);
    
    void prepare_graph_lines();
    
    bool is_stuck();
    
private:
    GilbertElliottModel lossModel;
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* control_parameters;
    std::vector<ChunkProcessor> chunk_processors;
    
    void process(int start_pos);
    
    int block_index;
    
    floattype SAMPLE_RATE;
    
    floattype freq_to_line(floattype freq);
    floattype line_to_freq(floattype line);
    
    std::unique_ptr<ModifiedDiscreteCosineTransform> mdct;
    
    std::vector<floattype>kernel;
    int kernel_size;
    int kernel_center;
    
    floattype masking_amount;
    
    floattype bit_reduction_above_threshold;
    
    floattype speed;
    
    floattype step_down;
    floattype step_back;
    
    floattype absolute_threshold_level;
    
    floattype bias;
    
    floattype perceptual_curve;
    
    floattype mix;
    
    floattype gate_ratio;
    
    int num_channels;
    
    bool in_loss_state;
    
};
