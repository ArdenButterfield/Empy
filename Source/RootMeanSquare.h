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
 The RootMeanSquare class maintains a rolling average of the levels of all frequency lines. At any point, the decay time can be changed to a new number of samples. The tick function accepts an array of samples. If the array is the same size as the previous arrays, the class will update the values of the mean_values array to match the frequency line values in the new frame; if the incoming sample is a different size, the class will resize the mean_values array accordingly, and start from zero.
 
 Note that the mean_values array stores square of amplitude (that is, the power), not the amplitude itself. This is because we use the values in the power domain to calculate the threshold, and so leaving it like this saves redundant square roots and multiplications.
 */

#pragma once
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "utils.h"

class RootMeanSquare
{
public:
    RootMeanSquare();

    int set_decay_time(floattype num_samples);
    void tick(std::vector<floattype>& sample_in);
    
    std::vector<floattype> mean_values;
    
private:
    floattype rms_coeff;
    floattype decay_time;
};
