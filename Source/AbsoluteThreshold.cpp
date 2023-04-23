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

#include "AbsoluteThreshold.h"

AbsoluteThreshold::AbsoluteThreshold()
{
    ;
}

AbsoluteThreshold::~AbsoluteThreshold()
{
    ;
}

void AbsoluteThreshold::fill_threshold(std::vector<floattype>& thresh, floattype sample_rate)
{
    
    floattype freq;
    for (int l = 0; l < thresh.size(); ++l) {
        freq = (sample_rate * (floattype)l) / ((floattype) thresh.size());
        // This whole method isn't very efficient, since we're having to read through the table for every single entry
        // in the thresh vector. But we won't be calculating this very often, so I'm not bothering to write a more
        // efficient method right now.
        thresh[l] = get_threshold(freq);
    }
}

floattype AbsoluteThreshold::interpolate(floattype x1, floattype y1, floattype x2, floattype y2, floattype x_mid)
{
    if (x1 == x2) {
        return y1;
    }
    return ((y2 - y1) / (x2 - x1)) * (x_mid - x1) + y1;
}

floattype AbsoluteThreshold::get_threshold(floattype frequency)
{
    std::array<floattype, 2>* prev_entry = &thresh_table[0];
    for (auto& entry : thresh_table) {
        if (entry[0] > frequency) {
            return interpolate((*prev_entry)[0], (*prev_entry)[1], entry[0], entry[1], frequency);
        } else {
            prev_entry = &entry;
        }
    }
    return (*prev_entry)[1];
    
}
