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

#pragma once

// Control parameters is a std::array rather than a std::vector, so we store the
// number of parameters in it and pass that around.
// TODO: Might be worth just changing it to be a vector?
const int NUM_CONTROL_PARAMETERS = 12;

#define USE_DOUBLE 0

#if USE_DOUBLE
// So we can change easily between working with values as floats or doubles.
typedef double floattype;
#else
typedef float floattype;
#endif
