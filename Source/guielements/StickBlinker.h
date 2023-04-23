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

#include "../EmpyModel.h"

//==============================================================================
/*
*/
class StickBlinker  : public juce::Component, public juce::Timer
{
public:
    StickBlinker();
    ~StickBlinker() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setEmpyModel(EmpyModel* em);
    void timerCallback() override;
private:
    EmpyModel* empyModel;
    bool on;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StickBlinker)
};
