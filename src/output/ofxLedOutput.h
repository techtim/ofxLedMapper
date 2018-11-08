//
// Created by Timofey Tavlintsev on 30/10/2018.
//

#pragma once

#include "ofxLedRpi.h"

/// In development
// #include "ofxLedArtnet.h"
// #include "ofxLedDmx.h"


#ifndef LED_MAPPER_NO_GUI
/// Static function to generate ubniversal container for controllers GUI
static unique_ptr<ofxDatGui> GenerateOutputGui()
{
    ofLogVerbose() << "Generate scene gui";

    unique_ptr<ofxDatGui> gui = make_unique<ofxDatGui>();
    gui->setPosition(ofGetWidth() - 200, ofGetHeight() / 4);
    gui->setAutoDraw(false);
    gui->setWidth(LM_GUI_WIDTH);
    /// set theme for gui and apply emediatly to all added components

    return move(gui);
}
#endif
