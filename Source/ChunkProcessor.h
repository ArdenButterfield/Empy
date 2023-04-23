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

/*
 This class is managed by EmpyModel, and handles the processing of individual MDCT blocks.
 There will be a ChunkProcessor object for each channel.
*/

#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>

#include "AbsoluteThreshold.h"
#include "RootMeanSquare.h"
#include "utils.h"

floattype amplitude_to_db(const floattype amplitude);
floattype power_to_db(const floattype power);
floattype db_to_amplitude(const floattype db);
floattype db_to_power(const floattype db);

class ChunkProcessor {
public:
    ChunkProcessor();
    ChunkProcessor(int lines, floattype fs);
    ~ChunkProcessor();
    
    void resize(int new_num_lines);
    void build_threshold(const std::vector<floattype> &kernel,
                         const int kernel_center,
                         const floattype threshold_level,
                         const floattype abs_threshold_level,
                         const floattype speed,
                         const floattype perceptual_curve);
    
    void apply_threshold(const floattype bit_reduction_above_threshold,
                         const floattype gate_ratio);
    void calc_graph_lines();
    void recover_packet();
    
    std::vector<floattype> threshold;
    
    std::vector<floattype> raw_freq_lines;
    std::vector<floattype> processed_freq_lines;
    std::vector<floattype> prev_processed_lines;

    
    std::vector<floattype> raw_samples;
    std::vector<floattype> processed_samples;
    
    std::vector<floattype> new_static_thresh;
    std::vector<floattype> new_dynamic_thresh;
    
    std::vector<floattype> spread_demo;
    
    int num_lines;
    
    std::vector<floattype> bias_curve;
    
    void build_bias(floattype new_bias);
    
private:
    std::array<floattype, 26> CRITICAL_BAND_CUTOFFS = {
        0,
        100,
        200,
        300,
        400,
        510,
        630,
        770,
        920,
        1080,
        1270,
        1480,
        1720,
        2000,
        2320,
        2700,
        3150,
        3700,
        4400,
        5300,
        6400,
        7700,
        9500,
        12000,
        15000,
        25000
    };

    std::vector<floattype> absolute_threshold;
    
    
    std::vector<floattype> energies;
    std::vector<floattype> spread_energies;
    std::vector<floattype> demo_spread_energies;
    
    std::vector<int> band_assignments;
    std::vector<int> lines_per_band;
    
    void assign_bands();
    void spread(const std::vector<floattype> &kernel,
                const int kernel_center);
    void calc_static_thresh(const floattype abs_threshold_level,
                            const floattype perceptual_curve);
    void calc_dynamic_thresh(const floattype masking_threshold_scalar);
    void apply_bias_curve();
    
    void fill_absolute_threshold();
    
    void make_spread_demo(const std::vector<floattype> &kernel,
                          const int kernel_center);
    
    floattype sample_rate;
    
    floattype freq_to_line(floattype freq);
    floattype line_to_freq(floattype line);
    
    floattype prev_abs_threshold_level;
    floattype prev_abs_threshold_curve;
    
    AbsoluteThreshold absoluteThreshold;
    
    RootMeanSquare rms;
};
