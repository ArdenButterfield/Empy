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

#include "EmpyModel.h"


floattype decibel(floattype sample)
{
    return std::log10(sample * sample) * 10.0f;
}

EmpyModel::EmpyModel()
{
    // initialize random seed
    srand ((unsigned int)time(NULL));
}

void EmpyModel::set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c)
{
    control_parameters = c;
}

void EmpyModel::prepare(int mdct_step, floattype sample_rate, int n_channels)
{
    num_channels = n_channels;
    
    // Forces set bias
    bias = 2;
    MDCT_WIDTH = mdct_step * 2;
    MDCT_LINES = mdct_step;
    SAMPLE_RATE = sample_rate;
    
    if (not isPowerOfTwo(mdct_step)) {
        throw std::invalid_argument("MDCT step size must be a power of 2");
    }
    // We do this in 2 lines (resize and then fill) because we want to call the constructor
    // with the new number of lines and sample rate for all the items.
    chunk_processors.resize(num_channels);
    std::fill(chunk_processors.begin(), chunk_processors.end(), ChunkProcessor(MDCT_LINES, SAMPLE_RATE));
    
    graphScaledLines.resize(MDCT_LINES);

    mdct = std::make_unique<ModifiedDiscreteCosineTransform>(MDCT_WIDTH);
    
    // The block index tracks the position of the start of the input/output
    // mdct buffers. It's kinda arbitrary where we initialize it.
    block_index = 0;
    
    in_loss_state = false;
}

void EmpyModel::process(int start_pos)
{
    // Does all the processing we need to do, and adds the result to the output.
    
    for (int c = 0; c < num_channels; ++c) {
        mdct->transform(chunk_processors[c].raw_samples, chunk_processors[c].raw_freq_lines, start_pos);
    }
    
    for (int c = 0; c < num_channels; ++c) {
        chunk_processors[c].build_threshold(kernel,
                                            kernel_center,
                                            masking_amount,
                                            absolute_threshold_level,
                                            speed,
                                            perceptual_curve);
        chunk_processors[c].apply_threshold(bit_reduction_above_threshold,
                                            gate_ratio);
    }
    
    in_loss_state = lossModel.tick();
    for (int c = 0; c < num_channels; ++c) {
        if (in_loss_state) {
            chunk_processors[c].recover_packet();
        } else {
            chunk_processors[c].prev_processed_lines = chunk_processors[c].processed_freq_lines;
        }
    }

    for (int c = 0; c < num_channels; ++c) {
        mdct->inverseTransform(chunk_processors[c].processed_samples, chunk_processors[c].processed_freq_lines, start_pos);
    }
    
    prepare_graph_lines();
}

void EmpyModel::processBlock(juce::AudioBuffer<float>& buffer)
{
    // raw_block will contain the the most recent input samples (enough to
    // for the MDCT). processed_block will contain the most recent output
    // samples. For raw_block, we can simply overwrite old samples with new
    // samples. However, since the MDCT involves overlapping the transform
    // results, we need to be a bit cleverer, summing two neighboring windows together.
    int num_samples;
    
    std::vector<float *> channel_samples;
    channel_samples.resize(num_channels);
    
    for (int c = 0; c < num_channels; ++c) {
        channel_samples[c] = buffer.getWritePointer(c);
    }
    num_samples = buffer.getNumSamples();
    
    int input_index = 0;
    
    while (input_index < num_samples) {
        if (block_index == 0) {
            for (int c = 0; c < num_channels; ++c) {
                std::fill(chunk_processors[c].processed_samples.begin() + MDCT_WIDTH / 2,
                          chunk_processors[c].processed_samples.end(),
                          0);
            }
            process(block_index);
        }
        if (block_index == MDCT_WIDTH / 2) {
                for (int c = 0; c < num_channels; ++c) {
                    std::fill(chunk_processors[c].processed_samples.begin(),
                              chunk_processors[c].processed_samples.begin() + MDCT_WIDTH / 2,
                              0);
                }
            
            process(block_index);
        }

        int steps_til_process;
        if (block_index < MDCT_WIDTH / 2) {
            steps_til_process = MDCT_WIDTH / 2 - block_index;
        } else {
            steps_til_process = MDCT_WIDTH - block_index;
        }
        steps_til_process = std::min(steps_til_process, num_samples - input_index);
        floattype dry, wet;
        for (int c = 0; c < num_channels; ++c) {
            for (int i = 0; i < steps_til_process; ++i) {
                dry = chunk_processors[c].raw_samples[block_index + i];
                wet = chunk_processors[c].processed_samples[block_index + i];
                chunk_processors[c].raw_samples[block_index + i] = channel_samples[c][input_index + i];
                channel_samples[c][input_index + i] = wet * mix + dry * (1 - mix);
            }
        }
        block_index += steps_til_process;
        input_index += steps_til_process;
        block_index %= MDCT_WIDTH;
    }
}

floattype linpower(const floattype input, const floattype transition_point)
{
    // A mostly-power scale that touches the origin. The magic numbers are from
    // messing around until I liked the way that the slider acted.
    floattype log = std::pow(10.0, input * 4 - 1);
    if (input > transition_point) {
        return log;
    }
    if (input <= 0) {
        return 0;
    }
    return log * (input / transition_point);
        
}

void EmpyModel::set_mask_threshold(const floattype new_threshold)
{
    
    masking_amount = linpower(new_threshold * 1.4, 1.0);
}

void EmpyModel::set_spread_distance(const floattype new_distance)
{
    // The spread distance is how many bands the kernel will spread on either side.
    int new_kernel_size = std::floor(new_distance) * 2 + 1;
    
    if (new_kernel_size == kernel_size) {
        return;
    }
    
    std::cout << new_distance << "\n";
    kernel.resize(new_kernel_size);
    kernel_size = new_kernel_size;
    kernel_center = std::floor(new_distance);
    
    // I'm just setting the window to be triangular for now, but
    // https://en.wikipedia.org/wiki/List_of_window_functions has some other
    // tantilizing options. :] This is also where I'm adapting this algorithm from.
    floattype kernelsum = 0.0;
    for (int i = 0; i < kernel_size; ++i) {
        floattype v = 1 - std::abs(((floattype)i - std::floor(new_distance)) / new_distance);
        kernel[i] = v;
        kernelsum += v;
    }
    // We want the overall threshold level to stay the same when we change the kernel size,
    // so we ensure that kernels do not anything to the threshold, merely spread it around.
    for (int i = 0; i < kernel_size; ++i) {
        kernel[i] /= kernelsum;
    }
}

void EmpyModel::set_bit_reduction_above_threshold(const floattype new_redux)
{
    bit_reduction_above_threshold = new_redux;
}

void EmpyModel::set_packet_loss(const floattype probability,
                                const floattype length,
                                const floattype max_length) {
    
    // If there is a probabilty q of us leaving the loss each sample, we will stay
    // in the loss state, on average, for (1 - q) / q (= sum(n:0->inf)n*q*(1-q)^n)
    // samples. Solving for q, we get q = 1 / (samle_length + 1)
    std::cout << "length: " << length << "\n";
    if (length >= max_length) {
        lossModel.q = 0.0;
    } else {
        floattype sample_length = length * SAMPLE_RATE / MDCT_WIDTH;
        lossModel.q = 1.0 / (sample_length + 1.0);
    }
    
    // The eigenvector of this markov chain is [q, p], which means that we spend p/(q+p) of the time
    // in the loss state and q/(q+p) of the time in the non-loss state. Since q is set by the length of
    // loss, we need to set p to achieve the correct balance. Let r be the probability that we are in a
    // loss state, if r = p/(q+p), then p = qr/(1-r).
    if (probability == 1.0) {
        lossModel.p = 1.0;
    } else {
        lossModel.p = std::min((floattype)1.0, (lossModel.q * probability) / (1 - probability));
    }
}

void EmpyModel::set_speed(const floattype new_speed)
{
    speed = new_speed;
}

void EmpyModel::set_mdct_size(const floattype new_size)
{
    if (new_size != MDCT_LINES) {
        std:: cout << new_size << "\n";
        prepare(new_size, SAMPLE_RATE, num_channels);
    }
}


void EmpyModel::set_absolute_threshold(const floattype new_abs_threshold)
{
    absolute_threshold_level = std::pow(10.f, new_abs_threshold * 25 - 22);
}

void EmpyModel::set_bias(const floattype new_bias)
{
    if (new_bias != bias) {
        for (int c = 0; c < num_channels; ++c) {
            chunk_processors[c].build_bias(new_bias);
        }
        bias = new_bias;
        for (int f = 0; f < MDCT_LINES; ++f) {
            graphScaledLines.bias[f] = std::log10(chunk_processors[0].bias_curve[f]) * 10;
        }
    }
}

void EmpyModel::set_perceptual_curve(const floattype new_perceptual_curve)
{
    perceptual_curve = new_perceptual_curve;
}

void EmpyModel::set_mix(const floattype new_mix)
{
    mix = new_mix / 100.0;
}

void EmpyModel::set_gate_ratio(const floattype new_ratio)
{
    gate_ratio = new_ratio;
}

floattype safe_pow_to_db(const floattype pow) {
    if (pow <= 0) {
        return -10000;
    } else {
        return std::log10(pow) * 10;
    }
}

void EmpyModel::prepare_graph_lines()
{
    // The bias line is prepared in set_bias(), because it doesn't move around as often, so it
    // would be wasteful to call it every single block.
    floattype raw, proc, thresh, static_thresh, dynamic_thresh, spread;
    for (int f = 0; f < MDCT_LINES; ++f) {
        // This seems to be how ableton does it: that is, if L & R are perfectly out of phase, spectrum view
        // shows no signal, if L & R are identical then both playing at once is +6dB (twice as loud) compared
        // to just one of the channels.
        raw = 0;
        proc = 0;
        thresh = 0;
        static_thresh = 0;
        dynamic_thresh = 0;
        spread = 0;
        
        for (auto &c : chunk_processors) {
            raw += c.raw_freq_lines[f];
            proc += c.processed_freq_lines[f];
            thresh += c.threshold[f];
            static_thresh += c.new_static_thresh[f];
            dynamic_thresh += c.new_dynamic_thresh[f];
            spread += c.spread_demo[f];
            
        }
        raw /= num_channels;
        proc /= num_channels;
        thresh /= num_channels;
        static_thresh /= num_channels;
        dynamic_thresh /= num_channels;
        spread /= num_channels;
        
        graphScaledLines.input[f] = safe_pow_to_db(raw * raw);
        graphScaledLines.output[f] = safe_pow_to_db(proc * proc);
        graphScaledLines.threshold[f] = safe_pow_to_db(thresh);
        graphScaledLines.static_threshold[f] = safe_pow_to_db(static_thresh);
        graphScaledLines.dynamic_threshold[f] = safe_pow_to_db(dynamic_thresh);
        graphScaledLines.spread[f] = safe_pow_to_db(spread);
    }
}

bool EmpyModel::is_stuck()
{
    return in_loss_state;
}
