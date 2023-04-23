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
 This class performs the forward and inverse versions of the Modified Discrete Cosine Transform (MDCT). Similar to the Fast Fourier Transform, the MDCT converts between the time domain (in which we get our samples in the Plugin Processor) and the frequency domain (in which we process them in the ChunkProcessor).
 
 This implementation is based on the python implementation here: https://github.com/smagt/mdct There are certainly improvements to be made here, the biggest being that it currently uses the JUCE FFT implementation, which only works with floats (!) so we spend a lot of time converting our double array to a float array, just to convert it back again.
 */
#pragma once

#include <stdexcept>
#include <math.h>
#include "JuceHeader.h" // fft
#include <complex> // complex numbers
#include <cmath> // log2()
#include <vector>

#include "utils.h"

const floattype PI = 3.14159265359;
typedef std::complex<floattype> fcomp;


bool isPowerOfTwo(int n);
void sineWindow(std::vector<floattype>& window, int window_len);

class ModifiedDiscreteCosineTransform
{
public:
    // num_samples: number of time domain samples.
    ModifiedDiscreteCosineTransform(int num_samples);
    ~ModifiedDiscreteCosineTransform();
    
    // Replaces the values in freq_vals with the transform of time_vals,
    // assuming time_vals is a circular array starting at start_pos with
    // length window_len.
    void transform(std::vector<floattype>& time_vals, std::vector<floattype>& freq_vals, int start_pos);
    
    // Adds (NOT replaces) to the values of time_vals the inverse transform of
    // freq_vals, assuming time_vals is circular as before.
    void inverseTransform(std::vector<floattype>& time_vals, std::vector<floattype>& freq_vals, int start_pos);

private:
    std::vector<floattype> window;
    int window_len;
    std::vector<fcomp> rotation_points;
    std::vector<floattype> rot;
    std::vector<fcomp> c;
    std::vector<fcomp> transformed_c;
    
    std::unique_ptr<juce::dsp::FFT> fourier;
};
