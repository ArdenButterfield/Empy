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
#include <juce_graphics/juce_graphics.h>
#include "StickBlinker.h"

//==============================================================================
StickBlinker::StickBlinker()
{
    on = false;
    startTimer(60);
    this->setClickingTogglesState(true);
}

StickBlinker::~StickBlinker()
{
}

void StickBlinker::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto lookFeel = EmpyLookAndFeel();
    juce::Colour fillColor;
    if (shouldDrawButtonAsDown) {
        fillColor = lookFeel.BEVEL_BLACK;
    } else if (getToggleState()) {
        fillColor = lookFeel.BEVEL_DARK;
    } else if (shouldDrawButtonAsHighlighted) {
        fillColor = lookFeel.BEVEL_WHITE;
    } else {
        fillColor = lookFeel.PANEL_BACKGROUND_COLOR;
    }
    g.setColour(fillColor);
    g.fillRect(getLocalBounds());

    if (!getToggleState() && on) {
        g.setColour(lookFeel.BEVEL_DARK);
        g.fillRect(getLocalBounds().withSizeKeepingCentre(10,10));
    }

    g.setColour(lookFeel.BEVEL_BLACK);
    g.drawRect(getLocalBounds(), 2);
    g.setColour(lookFeel.BEVEL_WHITE);
    g.drawRect(getLocalBounds().withTrimmedRight(2).withHeight(2));
    g.drawRect(getLocalBounds().withTrimmedBottom(2).withWidth(2));
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
    if (empyModel != nullptr) {
        bool new_on = empyModel->is_stuck();
        if (new_on != on) {
            on = new_on;
            repaint();
        }
    }
}
