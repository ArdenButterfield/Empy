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

#include <JuceHeader.h>
#include <memory>

#include "utils.h"

typedef union ControllerUnion {
    juce::Slider slider;
    juce::ComboBox combobox;
} Controller;

enum ControllerType {
    slider,
    combobox
};

struct ControlParameter {
    ControlParameter() {
        ;
    }
    juce::Identifier identifier;
    juce::RangedAudioParameter* audio_parameter;
    
    std::unique_ptr<juce::Component> controller;
    ControllerType controller_type;
    
    floattype min_val;
    floattype max_val;
    
    std::string name;
    std::string description;
        
    juce::Label controller_label;
    
    bool active;
    bool focused;

};
