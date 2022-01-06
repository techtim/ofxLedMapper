//
// Created by Timofey Tavlintsev on 30/10/2018.
//

#pragma once

#include "ofxLedArtnet.h"
#include "ofxLedRpi.h"
// #include "ofxLedDmx.h"

#include "EASTL/variant.h"

namespace LedMapper {

using ofxLedOutput = eastl::variant<ofxLedRpi, ofxLedArtnet>;

#ifndef LED_MAPPER_NO_GUI
/// Static function to generate ubniversal container for controllers GUI
static unique_ptr<ofxDatGui> GenerateOutputGui()
{
    ofLogVerbose() << "Generate Controller gui";

    auto gui = make_unique<ofxDatGui>();
    gui->setPosition(ofGetWidth() - LM_GUI_WIDTH, ofGetHeight() / 4);
    gui->setAutoDraw(false);
    gui->setWidth(LM_GUI_WIDTH);
    /// set theme for gui and apply emediatly to all added components

    return gui;
}

static unique_ptr<ofxDatGui> GenerateGrabGui()
{
    ofLogVerbose() << "Generate Grab gui";

    auto gui = make_unique<ofxDatGui>();
    gui->setPosition(ofGetWidth() - LM_GUI_WIDTH * 2, ofGetHeight() / 4);
    gui->setAutoDraw(false);
    gui->setWidth(LM_GUI_WIDTH);
    /// set theme for gui and apply emediatly to all added components

    return gui;
}

#endif
} // namespace LedMapper
