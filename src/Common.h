/*
 Copyright (C) 2017 Timofey Tavlintsev [http://tvl.io]

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#ifndef LED_MAPPER_NO_GUI

#include "ofxDatGui.h"
#define LM_GUI_WIDTH 200
#define LMGUIListControllers "Controllers"
#define LMGUIToggleDebug "Debug controller"
#define LMGUITogglePlay "Play"
#define LMGUISliderFps "FPS"
#define LMGUIButtonAdd "Add Controller"

#define LCGUIButtonSend "Send"
#define LCGUIButtonDoubleLine "Double Line"
#define LCGUITextIP "IP"
#define LCGUITextPort "Port"
#define LCGUISliderPix "Pix in led"
#define LCGUIDropColorType "Color Type"
#define LCGUIDropChannelNum "Channel"
#define LCGUIButtonDmx "DMX"
#define LCFileName "Ctrl-"

#endif

// typedef unique_ptr<ofxLedController> ofxLedController_ptr;

#define LMCtrlsFolderPath "Ctrls"

#define RPI_IP "192.168.2.10"
#define RPI_PORT 3000

#if defined(WIN32) || defined(TARGET_RASPBERRY_PI)
#include "ofJson.h"
#endif

namespace LedMapper {

static const std::string APP_NAME = "LedMapper";

#ifdef WIN32
static const std::string CONFIG_PATH = "C:\\Users\\Public\\Documents\\";
#elif defined(__APPLE__)
static const std::string CONFIG_PATH = "/Users/Shared/";
#endif

/// JSON Utils
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct Point {
    uint16_t x;
    uint16_t y;
    bool operator<(const Point &rhs) const
    {
        if (x == rhs.x) {
            return y < rhs.y;
        }
        else {
            return x < rhs.x;
        }
    }
};

static void to_json(ofJson &j, const Point &p) { j = ofJson{ { "x", p.x }, { "y", p.y } }; }

static void from_json(const ofJson &j, Point &p)
{
    p.x = j.at("x").get<uint16_t>();
    p.y = j.at("y").get<uint16_t>();
}

/// COLORS
static const int LM_COLOR_GREEN = 0x009688;
static const int LM_COLOR_GREEN_DARK = 0x004d40;
static const int LM_COLOR_GREEN_LIGHT = 0x6BE6B4;
static const int LM_COLOR_RED = 0xbf093a;
static const int LM_COLOR_RED_DARK = 0x870427;

class ofxDatGuiThemeLM : public ofxDatGuiTheme {
public:
    ofxDatGuiThemeLM()
    {
        stripe.visible = false;
        color.label = hex(0x9C9DA1);
        color.icons = hex(0x9C9DA1);
        color.background = hex(0x28292E);
        color.guiBackground = hex(0x1E1F24);
        color.inputAreaBackground = hex(0x42424A);
        color.slider.fill = hex(LM_COLOR_GREEN_LIGHT); // 107 230 180
        color.slider.text = hex(0x9C9DA1);
        color.textInput.text = hex(0x9C9DA1);
        color.textInput.highlight = hex(0x28292E);
        color.colorPicker.border = hex(0xEEEEEE);
        color.textInput.backgroundOnActive = hex(0x1D1E22);
        color.backgroundOnMouseOver = hex(0x42424A);
        color.backgroundOnMouseDown = hex(0x1D1E22);
        color.matrix.hover.button = hex(0x9C9DA1);
        color.graph.fills = hex(0x9C9DA1);
        stripe.button = hex(0x64ffda);
        stripe.toggle = hex(0x64ffda);
        init();
    }
};

} // LedMapper