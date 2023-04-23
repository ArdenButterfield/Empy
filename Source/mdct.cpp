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

#include "mdct.h"

bool isPowerOfTwo(int n)
{
    if (n <= 0) {
        return false;
    }
    while (n > 1) {
        if (n % 2) {
            return false;
        }
        n /= 2;
    }
    return true;
}

void sineWindow(std::vector<floattype>& window, int window_len)
{
    floattype scale = PI / (floattype) window_len;
    for (int i = 0; i < window_len; i++) {
        window[i] = sin(scale * ((floattype) i + 0.5));
    }
}

ModifiedDiscreteCosineTransform::ModifiedDiscreteCosineTransform(int num_samples)
{
    if (num_samples % 4 || !isPowerOfTwo(num_samples)) {
        throw std::invalid_argument("number of samples for MDCT must be a power of 2, and a multiple of 4");
    }
    rot.resize(num_samples);
    window.resize(num_samples);
    
    c.resize(num_samples / 4);
    transformed_c.resize(num_samples / 4);
    
    // The number of points in a fourier transform is 2**n where n is the order.
    // The constructor expects the order of the transform.
    int fourier_order = round(log2(num_samples / 4));
    fourier = std::make_unique<juce::dsp::FFT>(fourier_order);
    
    window_len = num_samples;
    
    sineWindow(window, window_len);
    
    const floattype tau = 2.0 * PI;
    fcomp j = fcomp(0,-1);
    // Prepare a sequence of points rotated around the unit circle, "w" in the
    // python implementation.
    //   t = np.arange(0, N4)
    //   w = np.exp(-1j * 2 * np.pi * (t + 1. / 8.) / N)
    rotation_points.resize(window_len / 4);
    for (int index = 0; index < window_len / 4; ++index) {
        fcomp step = ((floattype)index + (1.0 / 8.0)) / (floattype)window_len;
        rotation_points[index] = std::exp(j * step * tau);
    }
}

ModifiedDiscreteCosineTransform::~ModifiedDiscreteCosineTransform()
{
}

void ModifiedDiscreteCosineTransform::transform(std::vector<floattype>& time_vals, std::vector<floattype>& freq_vals, int start_pos)
{
    //   N4 = N // 4
    //   rot = np.roll(x, N4)
    int rot_index, input_index;
    floattype real, imag;

    for (int i = 0; i < window_len; ++i) {
        rot_index = (i + (window_len / 4)) % window_len;
        input_index = (i + start_pos) % window_len;
        rot[rot_index] = time_vals[input_index] * window[i];
    }
    // rot[:N4] = -rot[:N4]
    for (int i = 0; i < window_len / 4; ++i) {
        rot[i] = -rot[i];
    }
    
    //   c = np.take(rot, 2 * t) - np.take(rot, N - 2 * t - 1) \
    //   - 1j * (np.take(rot, M + 2 * t) - np.take(rot, M - 2 * t - 1))
    for (int t = 0; t < window_len / 4; ++t) {
        real = rot[2 * t] - rot[window_len - 2 * t - 1];
        imag = -(rot[window_len / 2 + 2 * t] - rot[window_len / 2 - 2 * t - 1]);
        c[t] = fcomp(real, imag) * (floattype)0.5 * rotation_points[t];
    }

    //   c = (2. / np.sqrt(N)) * w * np.fft.fft(0.5 * c * w, N4)
#if USE_DOUBLE
    std::vector<std::complex<float>> c_float (c.begin(), c.end());
    std::vector<std::complex<float>> trans_c_float(transformed_c.begin(), transformed_c.end());
    fourier->perform(&(c_float[0]), &(trans_c_float[0]), false);
    for (int i = 0; i < transformed_c.size(); ++i) {
        transformed_c[i] = trans_c_float[i];
    }
#else
    fourier->perform(&(c[0]), &(transformed_c[0]), false);
#endif

    floattype scale = 2.0 / sqrt(window_len);
    for (int i = 0; i < window_len/4; ++i) {
        transformed_c[i] = scale * rotation_points[i] * transformed_c[i];
    }
    //   y = np.zeros(M)
    //   y[2 * t] = np.real(c[t])
    //   y[M - 2 * t - 1] = -np.imag(c[t])
    //   return y
    for (int t = 0; t < window_len / 4; ++t) {
        freq_vals[2 * t] = transformed_c[t].real();
        freq_vals[window_len / 2 - 2 * t - 1] = -transformed_c[t].imag();
    }
}

void ModifiedDiscreteCosineTransform::inverseTransform(std::vector<floattype>& time_vals, std::vector<floattype>& freq_vals, int start_pos)
{
    //   c = np.take(x, 2 * t) + 1j * np.take(x, N - 2 * t - 1)
    //   c = 0.5 * w * c
    floattype real, imag;
    for (int t = 0; t < window_len / 4; ++t) {
        real = freq_vals[2 * t];
        imag = freq_vals[window_len / 2 - 2 * t - 1];
        transformed_c[t] = fcomp(real, imag) * (floattype)0.5 * rotation_points[t];
    }
    //   c = np.fft.fft(c, M)
#if USE_DOUBLE
    std::vector<std::complex<float>> c_float (c.begin(), c.end());
    std::vector<std::complex<float>> trans_c_float(transformed_c.begin(), transformed_c.end());
    fourier->perform(&(trans_c_float[0]), &(c_float[0]), false);
    for (int i = 0; i < transformed_c.size(); ++i) {
        c[i] = c_float[i];
    }
#else
    fourier->perform(&(transformed_c[0]), &(c[0]), false);
#endif
    
    //   c = ((8 / np.sqrt(N2)) * w) * c
    floattype scale = 8.0 / sqrt(window_len);
    for (int i = 0; i < window_len / 4; ++i) {
        c[i] *= rotation_points[i] * scale;
    }
    
    //   rot[2 * t] = np.real(c[t])
    //   rot[N + 2 * t] = np.imag(c[t])
    for (int t = 0; t < window_len / 4; ++t) {
        rot[2 * t] = c[t].real();
        rot[window_len / 2 + 2 * t] = c[t].imag();
    }
    
    //   t = np.arange(1, N2, 2)
    //   rot[t] = -rot[N2 - t - 1]
    for (int t = 1; t < window_len; t+=2) {
        rot[t] = -rot[window_len - t - 1];
    }
    
    //   t = np.arange(0, 3 * M)
    //   y = np.zeros(N2)
    //   y[t] = rot[t + M]
    //   t = np.arange(3 * M, N2)
    //   y[t] = -rot[t - 3 * M]
    int output_index;
    
    // Note: we *add* the transformed values to our output array instead of
    // replacing them, because we want two transforms to overlap for the MDCT
    // to work, and this cuts down on having to use a temp array and copy over
    // yadda yadda
    for (int t = 0; t < window_len * 3 / 4; ++t) {
        output_index = (t + start_pos) % window_len;
        time_vals[output_index] += rot[t + window_len / 4] * window[t];
    }
    
    for (int t = window_len * 3 / 4; t < window_len; ++t) {
        output_index = (t + start_pos) % window_len;
        time_vals[output_index] += -rot[t - 3 * window_len / 4] * window[t];
    }
    
}
