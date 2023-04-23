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
/*
 Owned by the PluginEditor, this class listens for mouse events with the controllers, and
 then acts accordingly: it shows the description on the info panel, highlights the active slider,
 and resets the knob to its default value if it is double-clicked.
 */
#pragma once

#include <JuceHeader.h>

#include "ControlParameter.h"
#include "guielements/BigPanel.h"
#include "LookFeel.h"


class ControllerListener : public juce::MouseListener
{
public:
    ControllerListener(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* cp,
                       InfoPanel* ip,
                       TitlePanel* tp);
    ~ControllerListener();
    void mouseEnter(const juce::MouseEvent& event);
    void mouseExit(const juce::MouseEvent& event);
    void mouseDoubleClick(const juce::MouseEvent& event);
private:
    std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* control_parameters;
    InfoPanel* infoPanel;
    TitlePanel* titlePanel;
    std::string titleName = "Empy";
    std::string titleDescription = "By Arden Butterfield, 2023";
};
