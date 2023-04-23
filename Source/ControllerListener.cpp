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

#include "ControllerListener.h"

ControllerListener::ControllerListener(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* cp,
                                       InfoPanel* ip,
                                       TitlePanel* tp) :
        control_parameters(cp),
        infoPanel(ip),
        titlePanel(tp)
{
    for (auto &c : *control_parameters) {
        c.controller->addMouseListener(this, true);
    }
    titlePanel->addMouseListener(this, true);
}

ControllerListener::~ControllerListener()
{
    for (auto &c : *control_parameters) {
        c.controller->removeMouseListener(this);
    }
    titlePanel->removeMouseListener(this);
}

void ControllerListener::mouseEnter(const juce::MouseEvent& event)
{
    for (auto &c : *control_parameters) {
        if (event.eventComponent == c.controller.get()) {
            infoPanel->change_text(&c.name, &c.description);
            c.focused = true;
        }
    }
    if (event.eventComponent == titlePanel) {
        infoPanel->change_text(&titleName, &titleDescription);
    }
}

void ControllerListener::mouseExit(const juce::MouseEvent& event)
{
    
    for (auto &c : *control_parameters) {
        if ((event.eventComponent == c.controller.get()) &&
            !((c.controller_type == combobox) && (c.controller->isMouseOverOrDragging(true)))) {
            c.focused = false;
            infoPanel->setNoText();
        }
    }
    if (event.eventComponent == titlePanel) {
        infoPanel->setNoText();
    }
}

void ControllerListener::mouseDoubleClick(const juce::MouseEvent& event)
{
    // When we double-click, we want to reset the component to its default value.
    // TODO: combobox not registering double-clicks.
    for (auto &c : *control_parameters) {
        if (c.controller->isMouseOverOrDragging(true)) {
            c.audio_parameter->setValueNotifyingHost(c.audio_parameter->getDefaultValue());
            // I'm not sure if this next part is actually necessary. If it's ommited, the slider gets
            // set to the correct value when the timer gets called back in PluginEditor. Setting it
            // here just saves a frame or two (I think), making the action look a little more
            // responsive.
            if (c.controller_type == combobox) {
                auto ap_choice = static_cast<juce::AudioParameterChoice*>(c.audio_parameter);
                juce::ComboBox* combobox = static_cast<juce::ComboBox *>(c.controller.get());
                combobox->setSelectedId(ap_choice->getIndex(), juce::dontSendNotification);
            } else {
                auto ap_float = static_cast<juce::AudioParameterFloat*>(c.audio_parameter);
                juce::Slider* slider = static_cast<juce::Slider *>(c.controller.get());
                slider->setValue(*ap_float, juce::dontSendNotification);
            }
        }
    }
}
