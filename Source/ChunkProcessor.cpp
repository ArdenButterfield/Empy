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

#include "ChunkProcessor.h"


template <typename T> int sgn(T val) {
    // Source: https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
    return (T(0) < val) - (val < T(0));
}

ChunkProcessor::ChunkProcessor()
{
}

ChunkProcessor::ChunkProcessor(int lines, floattype fs)
{
    sample_rate = fs;
    num_lines = lines;
    
    raw_freq_lines = std::vector<floattype>(num_lines, 0.f);
    processed_freq_lines = std::vector<floattype>(num_lines, 0.f);
    prev_processed_lines = std::vector<floattype>(num_lines, 0.f);
    
    raw_samples = std::vector<floattype>(num_lines * 2, 0.f);
    processed_samples = std::vector<floattype>(num_lines * 2, 0.f);
        
    energies = std::vector<floattype>(CRITICAL_BAND_CUTOFFS.size(), 0.f);
    spread_energies = std::vector<floattype>(CRITICAL_BAND_CUTOFFS.size(), 0.f);
    demo_spread_energies = std::vector<floattype>(CRITICAL_BAND_CUTOFFS.size(), 0.f);
    
    threshold = std::vector<floattype>(num_lines, 0.f);

    absolute_threshold = std::vector<floattype>(num_lines);
    band_assignments = std::vector<int>(num_lines);
    lines_per_band = std::vector<int>(CRITICAL_BAND_CUTOFFS.size(),0.f);
    
    new_static_thresh = std::vector<floattype>(num_lines);
    new_dynamic_thresh = std::vector<floattype>(num_lines);
    spread_demo = std::vector<floattype>(num_lines);
    
    assign_bands();
    absoluteThreshold.fill_threshold(absolute_threshold, sample_rate);
    
    prev_abs_threshold_curve = -1;
    prev_abs_threshold_level = -1;
    
}

ChunkProcessor::~ChunkProcessor()
{
    ;
}

void ChunkProcessor::resize(int new_num_lines)
{
    num_lines = new_num_lines;
    
    raw_freq_lines.resize(num_lines, 0);
    processed_freq_lines.resize(num_lines, 0);
    prev_processed_lines = std::vector<floattype>(num_lines, 0.f);
    
    band_assignments.resize(num_lines);
    threshold.resize(num_lines, 0);
    absolute_threshold.resize(num_lines);
    
    new_static_thresh.resize(num_lines);
    new_dynamic_thresh.resize(num_lines);
    
    raw_samples = std::vector<floattype>(num_lines * 2, 0.f);
    processed_samples = std::vector<floattype>(num_lines * 2, 0.f);
    
    assign_bands();
    absoluteThreshold.fill_threshold(absolute_threshold, sample_rate);
    
}

void ChunkProcessor::spread(const std::vector<floattype> &kernel,
                            const int kernel_center)
{
    // Convolve spread kernel with energy to get spread energy
    int out_index;
    floattype src_energy;
    for (int b = 0; b < CRITICAL_BAND_CUTOFFS.size(); ++b) {
        src_energy = energies[b];
        for (int k = 0; k < kernel.size(); ++k) {
            out_index = b + k - kernel_center;
            if ((0 <= out_index) && (out_index < CRITICAL_BAND_CUTOFFS.size())) {
                spread_energies[out_index] += src_energy * kernel[k];
            }
        }
    }
}

void ChunkProcessor::make_spread_demo(const std::vector<floattype> &kernel,
                                      const int kernel_center)
{
    std::fill(demo_spread_energies.begin(), demo_spread_energies.end(), 0);
    
    const int demo_center = (int)CRITICAL_BAND_CUTOFFS.size() / 2;
    const floattype src_energy = energies[demo_center];
    
    for (int k = 0; k < kernel.size(); ++k) {
        demo_spread_energies[k - kernel_center + demo_center] = kernel[k] * src_energy;
    }
}

void ChunkProcessor::calc_static_thresh(const floattype abs_threshold_level,
                                            const floattype perceptual_curve)
{
    for (int f = 0; f < num_lines; ++f) {
        // The choice of 60 doesn't have much mathematical backing, although it's probably not far off from the geometric mean of the
        // threshold.... Honestly I'm not even sure if the geometric mean is the right sort of mean to take here, especially
        // since both axes have log scales. But really, I chose a value that made changing the curve not mess with the
        // threshold too much, when faced with the sort of frequency spectrum expected from musical sounds.
        floattype a = std::pow(absolute_threshold[f], perceptual_curve) * std::pow((floattype)60, (1 - perceptual_curve));
        new_static_thresh[f] = a * abs_threshold_level;
    }
}

void ChunkProcessor::calc_dynamic_thresh(const floattype masking_threshold_scalar)
{
    int band;
    
    for (int f = 0; f < num_lines; ++f) {
        band = band_assignments[f];
        new_dynamic_thresh[f] = spread_energies[band] * masking_threshold_scalar;
        spread_demo[f] = demo_spread_energies[band] * masking_threshold_scalar;
    }
}

void ChunkProcessor::apply_bias_curve()
{
    for (int f = 0; f < num_lines; ++f) {
        new_static_thresh[f] *= bias_curve[f];
        new_dynamic_thresh[f] *= bias_curve[f];
        spread_demo[f] *= bias_curve[f];
    }
}

void ChunkProcessor::build_threshold(const std::vector<floattype> &kernel,
                                     const int kernel_center,
                                     const floattype threshold_level,
                                     const floattype abs_threshold_level,
                                     const floattype speed,
                                     const floattype perceptual_curve)
{
    rms.set_decay_time(speed * sample_rate / (num_lines * 2));
    rms.tick(raw_freq_lines);
    
    std::fill(spread_energies.begin(), spread_energies.end(), 0);

    for (int f = 0; f < num_lines; ++f) {
        energies[band_assignments[f]] = rms.mean_values[f];
    }
    
    spread(kernel, kernel_center);
    make_spread_demo(kernel, kernel_center);

    floattype masking_threshold_scalar = threshold_level;
    
    if ((abs_threshold_level != prev_abs_threshold_level) ||
        (perceptual_curve != prev_abs_threshold_curve)) {
        calc_static_thresh(abs_threshold_level, perceptual_curve);
    }
    
    calc_dynamic_thresh(masking_threshold_scalar);
    
    apply_bias_curve();
    
    for (int f = 0; f < num_lines; ++f) {
        threshold[f] = std::max(new_static_thresh[f], new_dynamic_thresh[f]);
    }
    
}

void ChunkProcessor::apply_threshold(const floattype bit_reduction_above_threshold,
                                     const floattype gate_ratio)
{
    
    floattype raw, thresh, processed;
    for (int f = 0; f < num_lines; ++f) {
        
        raw = raw_freq_lines[f];
        thresh = threshold[f];
        
        processed = raw;
        if ((thresh > rms.mean_values[f]) && (rms.mean_values[f] != 0.0) && (thresh != 0.0)) {
            floattype db_thresh = power_to_db(thresh);
            floattype db_from_thresh = power_to_db(raw * raw) - db_thresh;
            if (db_from_thresh < 0) {
                db_from_thresh *= gate_ratio;
            }
            
            processed = db_to_amplitude(db_from_thresh + db_thresh);
            if (raw < 0) {
                processed = -processed;
            }
        }
        if ((bit_reduction_above_threshold != 0) && (processed != 0)) {
            floattype p = db_to_amplitude(std::floor(amplitude_to_db(processed) / bit_reduction_above_threshold) * bit_reduction_above_threshold);
            if (processed < 0) {
                processed = -p;
            } else {
                processed = p;
            }
        }
        processed_freq_lines[f] = processed;
        
    }
    
}

void ChunkProcessor::assign_bands()
{
    // Calculate which critical band each frequency line should be in.
    // TODO: doesn't this result in nothing being in band 0, and 2 bands being crammed
    // into the top band? i.e. off-by-one error.
    
    int band = 0;
    floattype freq;
    std::fill(lines_per_band.begin(),lines_per_band.end(),0);
    
    for (int f = 0; f < num_lines; ++f) {
        freq = line_to_freq((floattype)f);
        if ((freq > CRITICAL_BAND_CUTOFFS[band]) && (band < CRITICAL_BAND_CUTOFFS.size() - 1)) {
            ++band;
        }
        band_assignments[f] = band;
        lines_per_band[band]++;
    }
}

void ChunkProcessor::recover_packet()
{
    // Use the absolute values of the most recent packet but the magnitudes of the last transmitted packet.
    for (int t = 0; t < num_lines; ++t) {
        if (((processed_freq_lines[t] > 0) && (prev_processed_lines[t] < 0)) ||
            ((processed_freq_lines[t] < 0) && (prev_processed_lines[t] > 0))) {
            processed_freq_lines[t] = -prev_processed_lines[t];
        } else {
            processed_freq_lines[t] = prev_processed_lines[t];
        }
    }
}

floattype ChunkProcessor::freq_to_line(floattype freq)
{
    return num_lines * freq / (sample_rate / 2);
}

floattype ChunkProcessor::line_to_freq(floattype line)
{
    return line * (sample_rate / 2) / num_lines;
}

void ChunkProcessor::build_bias(floattype new_bias)
{
    bias_curve.resize(num_lines);
    floattype left = freq_to_line(60);
    floattype right = freq_to_line(20000);
    const floattype PI = 3.14159265359;
    
    // DUCK: by lowering the threshold when the bias is in the middle, we keep the overall threshold low-point
    // about the same at all bias settings, so that audio doesn't come in and out wildly when the bias is changed.
    // This way, users can change the sound with the bias slider alone, without having to move the threshold level
    // sliders simultaneously.
    floattype duck = std::cos(new_bias * PI / 4.0);
    const floattype duck_amount = -2.0;
    const floattype sharpness = 5;
    floattype input, rawcurve;
    for (int f = 0; f < num_lines; ++f) {
        // Input rescales left -- right to a log scale between 0 and 1.
        input = std::log((floattype) f / left) / std::log(right / left);
        rawcurve = (atan((input - 0.5) * sharpness) / PI) * 6 * (-new_bias);
        bias_curve[f] = std::pow(10.0, rawcurve + duck * duck_amount);
        // bias_curve[f] = std::max(((atan((input - 0.5) * sharpness) / PI) * 2 * (-new_bias) + 1) / 2, 0.0);
    }
}


floattype amplitude_to_db(const floattype amplitude)
{
    return 20.0 * std::log10(std::abs(amplitude));
}

floattype power_to_db(const floattype power)
{
    return 10.0 * std::log10(power);
}

floattype db_to_amplitude(const floattype db)
{
    return std::pow(10.0, db / 20.0);
}

floattype db_to_power(const floattype db)
{
    return std::pow(10.0, db / 10.0);
}
