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

#include "RootMeanSquare.h"

RootMeanSquare::RootMeanSquare()
{
    mean_values.resize(0);
    rms_coeff = 1;
    decay_time = 0;
}

int RootMeanSquare::set_decay_time(floattype num_samples)
{
    if (num_samples == decay_time) {
        return 0;
    }
    if (num_samples > 0) {
        decay_time = num_samples;
        
        rms_coeff = 1 - std::exp(-2.2 / num_samples);
        return 0;
    }
    if (num_samples == 0) {
        decay_time = num_samples;
        rms_coeff = 1;
        return 0;
    }
    return 1;
}


void RootMeanSquare::tick(std::vector<floattype>& sample_in)
{
    if (sample_in.size() != mean_values.size()) {
        mean_values.resize(sample_in.size(), 0);
    }
    floattype energy;
    for (int i = 0; i < mean_values.size(); ++i) {
        energy = sample_in[i] * sample_in[i];
        mean_values[i] += rms_coeff * (energy - mean_values[i]);
    }
}
