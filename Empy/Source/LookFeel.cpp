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

#include "LookFeel.h"

EmpyLookAndFeel::EmpyLookAndFeel() :
main_font (juce::Typeface::createSystemTypefaceFor(BinaryData::bedsteadsemicondensed_otf, BinaryData::bedsteadsemicondensed_otfSize)),
bold_font(juce::Typeface::createSystemTypefaceFor(BinaryData::bedsteadboldsemicondensed_otf, BinaryData::bedsteadboldsemicondensed_otfSize)),
tooltip_font(juce::Typeface::createSystemTypefaceFor(BinaryData::GontserratRegular_ttf, BinaryData::GontserratRegular_ttfSize))
{
    // this->setColour(juce::ResizableWindow::backgroundColourId, PANEL_BACKGROUND_COLOR);
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xffb3b2a8));
    setColour(juce::ComboBox::backgroundColourId, BEVEL_LIGHT);
    setColour(juce::ComboBox::textColourId, BEVEL_BLACK);
    setColour(juce::ComboBox::outlineColourId, BEVEL_DARK);
    setColour(juce::ComboBox::focusedOutlineColourId, BEVEL_BLACK);
    setColour(juce::ComboBox::arrowColourId, BEVEL_DARK);
    
    setColour(juce::PopupMenu::backgroundColourId, BEVEL_LIGHT);
    setColour(juce::PopupMenu::textColourId, BEVEL_DARK);
    setColour(juce::PopupMenu::headerTextColourId, BEVEL_BLACK);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, BEVEL_WHITE);
    setColour(juce::PopupMenu::highlightedTextColourId, BEVEL_BLACK);
    
    setColour(juce::Slider::textBoxTextColourId, BEVEL_DARK);
    const float transparent_alpha = 0;
    setColour(juce::Slider::textBoxOutlineColourId,
              PANEL_BACKGROUND_COLOR.withAlpha(transparent_alpha));
    setColour(juce::Slider::textBoxHighlightColourId, BEVEL_BLACK);
    setColour(juce::Slider::textBoxBackgroundColourId,
              PANEL_BACKGROUND_COLOR.withAlpha(transparent_alpha));
    setColour(juce::Label::textWhenEditingColourId, BEVEL_BLACK);
    
    
    setColour(juce::CaretComponent::caretColourId, juce::Colour(0xff2e2edb));
    
}

EmpyLookAndFeel::~EmpyLookAndFeel()
{
}

void EmpyLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                        int x,
                                        int y,
                                        int width,
                                        int height,
                                        float sliderPos,
                                        const float rotaryStartAngle,
                                        const float rotaryEndAngle,
                                        juce::Slider& s)
{
    // Based on https://docs.juce.com/master/tutorial_look_and_feel_customisation.html
    
    juce::Colour fill = juce::Colours::white;
    juce::Colour outline = juce::Colours::black;
    juce::Colour notch = juce::Colour(0xff2d2d46);
    bool focused, active;
    for (const auto &c : *controlParameters) {
        if (c.controller.get() == &s) {
            focused = c.focused;
            active = c.active;
            break;
        }
    }
    if (!active) {
        fill = BEVEL_LIGHT;
        notch = BEVEL_DARK;
        outline = BEVEL_DARK;
    }
    
    const auto radius = (float) juce::jmin(width / 2, height / 2) * 0.8;
    const auto inner_radius = focused ? radius - 2 : radius;
    const auto centerx = (float) x + (float) width  * 0.5f;
    const auto centery = (float) y + (float) height * 0.5f;
    
    
    if (focused) {
        const float shadow_tilt = 0.2;
        
        const float penumbra_rad = 0.2;
        
        juce::Path highlight;
        highlight.startNewSubPath(centerx, centery);
        highlight.addArc(centerx - radius,
                      centery - radius,
                      radius * 2,
                      radius * 2,
                      PI * (shadow_tilt + 1),
                      PI * (shadow_tilt + 2));
        highlight.closeSubPath();
        
        g.setColour(fill);
        g.fillPath(highlight);
        
        juce::Path penumbra;
        penumbra.startNewSubPath(centerx, centery);
        penumbra.addArc(centerx - radius,
                      centery - radius,
                      radius * 2,
                      radius * 2,
                      PI * shadow_tilt - penumbra_rad,
                      PI * (shadow_tilt + 1) + penumbra_rad);
        penumbra.closeSubPath();
        
        g.setColour(BEVEL_LIGHT);
        g.fillPath(penumbra);
        
        juce::Path shadow;
        shadow.startNewSubPath(centerx, centery);
        shadow.addArc(centerx - radius,
                      centery - radius,
                      radius * 2,
                      radius * 2,
                      PI * shadow_tilt + penumbra_rad,
                      PI * (shadow_tilt + 1) - penumbra_rad);
        shadow.closeSubPath();
        
        g.setColour(BEVEL_DARK);
        g.fillPath(shadow);
        
    }
    
    g.setColour(fill);
    g.fillEllipse(centerx - inner_radius,
                  centery - inner_radius,
                  inner_radius * 2,
                  inner_radius * 2);

    if (active) {
        g.setColour(outline);
        g.drawEllipse(centerx - radius,centery - radius,radius*2,radius*2, 1);
    }
    
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (notch);

    
    juce::Path p;
    auto pointerLength = inner_radius;
    auto pointerThickness = 2.0f;
    p.addRectangle (-pointerThickness * 0.5f, -inner_radius, pointerThickness, pointerLength);
    p.applyTransform (juce::AffineTransform::rotation (angle).translated (centerx, centery));
    g.fillPath (p);
    
}

void EmpyLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                       int x,
                                       int y,
                                       int width,
                                       int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const juce::Slider::SliderStyle style,
                                       juce::Slider& s
                                       )
{
    bool focused, active;
    for (const auto &c : *controlParameters) {
        if (c.controller.get() == &s) {
            focused = c.focused;
            active = c.active;
            break;
        }
    }
    
    juce::Colour white, light, dark, black, thumb_bg;
    if (active) {
        white = BEVEL_WHITE;
        light = BEVEL_LIGHT;
        dark = BEVEL_DARK;
        black = BEVEL_BLACK;
        thumb_bg = PANEL_BACKGROUND_COLOR;
    } else {
        white = BEVEL_LIGHT;
        light = BEVEL_LIGHT;
        dark = BEVEL_DARK;
        black = BEVEL_DARK;
        thumb_bg = BEVEL_LIGHT;
    }
    
    const int pixel_size = 2;
    const int tick_spacing = 10; // 11 ticks demarkating
    
    // Make bar and thumb
    juce::Rectangle<int> bar, thumb;
    
    const int extra_bar_length = pixel_size * 3;
    const int thumb_length = 5;
    const int thumb_width = 12;
    
    if (style == juce::Slider::SliderStyle(juce::Slider::LinearHorizontal)) {
        const int center_y = y + (height / 2);
        bar = juce::Rectangle<int>(x - extra_bar_length,
                                   center_y - pixel_size * 2,
                                   width + extra_bar_length * 2,
                                   pixel_size * 4);
        thumb = juce::Rectangle<int>(0,0,pixel_size * thumb_length, pixel_size * thumb_width).withCentre(juce::Point<int>(sliderPos, center_y));
        
        if (focused) {
            int tick_x;
            g.setColour(dark);
            for (int i = 0; i < tick_spacing + 1; ++i) {
                tick_x = x + width * i / tick_spacing;
                
                g.drawVerticalLine(tick_x, center_y - 10, center_y - 8);
                g.drawVerticalLine(tick_x, center_y + 8, center_y + 10);
            }
        }
    }
    if (style == juce::Slider::SliderStyle(juce::Slider::LinearVertical)) {
        const int center_x = x + (width / 2);
        bar = juce::Rectangle<int>(center_x - pixel_size * 2,
                                   y - extra_bar_length,
                                   pixel_size * 4,
                                   height + extra_bar_length * 2);
        thumb = juce::Rectangle<int>(0,0,pixel_size * thumb_width, pixel_size * thumb_length).withCentre(juce::Point<int>(center_x, sliderPos));
        
        if (focused) {
            int tick_y;
            g.setColour(dark);
            for (int i = 0; i < tick_spacing + 1; ++i) {
                tick_y = y + height * i / tick_spacing;
                
                g.drawHorizontalLine(tick_y, center_x - 10, center_x - 8);
                g.drawHorizontalLine(tick_y, center_x + 8, center_x + 10);
            }
        }
    }
    
    g.setColour(white);
    g.fillRect(bar);
    bar = bar.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(dark);
    g.fillRect(bar);
    bar = bar.withTrimmedTop(pixel_size).withTrimmedLeft(pixel_size);
    g.setColour(light);
    g.fillRect(bar);
    bar = bar.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(black);
    g.fillRect(bar);
    
    g.setColour(black);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(white);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedTop(pixel_size).withTrimmedLeft(pixel_size);
    g.setColour(dark);
    g.fillRect(thumb);
    thumb = thumb.withTrimmedBottom(pixel_size).withTrimmedRight(pixel_size);
    g.setColour(thumb_bg);
    g.fillRect(thumb);
    // Draw thumb
        
}

#if 0
void EmpyLookAndFeel::drawComboBox (juce::Graphics& g,
                                    int width,
                                    int height,
                                    const bool isButtonDown,
                                    int buttonX,
                                    int buttonY,
                                    int buttonW,
                                    int buttonH,
                                    juce::ComboBox& box)
{
    g.setColour(juce::Colours::orange);
    g.fillRect(buttonX, buttonY, buttonW, buttonH);
    g.setColour(juce::Colours::green);
    auto boxbounds = box.getScreenBounds();
    g.drawRect(boxbounds,2);
}
#endif

void EmpyLookAndFeel::set_control_parameters(std::array<ControlParameter, NUM_CONTROL_PARAMETERS>* c)
{
    controlParameters = c;
}
