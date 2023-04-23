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
#include <JuceHeader.h>
#include "StickBlinker.h"

//==============================================================================
StickBlinker::StickBlinker()
{
    on = false;
    startTimer(60);
}

StickBlinker::~StickBlinker()
{
}

void StickBlinker::paint (juce::Graphics& g)
{
    if (on) {
        g.setColour(juce::Colour(0xff808080));
        
    } else {
        g.setColour(juce::Colour(0xffd4d0c8));
    }
    g.fillEllipse(getLocalBounds().toFloat());
}

void StickBlinker::resized()
{
}

void StickBlinker::setEmpyModel(EmpyModel* em)
{
    empyModel = em;
}

void StickBlinker::timerCallback()
{
    bool new_on = empyModel->is_stuck();
    if (new_on != on) {
        on = new_on;
        repaint();
    }
}
