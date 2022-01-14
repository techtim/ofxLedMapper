//
// Created by Timofey Tavlintsev on 30/10/2018.
//

#pragma once

#include "ofxLedArtnet.h"
#include "ofxLedRpi.h"
// #include "ofxLedDmx.h"

#include "EASTL/variant.h"

namespace LedMapper {
    
enum LedOutputType {
    LedOutputTypeLedmap,
    LedOutputTypeArtnet,
};
    
using LedOutput = eastl::variant<ofxLedRpi, ofxLedArtnet>;

static LedOutput CreateLedOutput(LedOutputType type)
{
    switch (type) {
        case LedOutputTypeLedmap:
            return ofxLedRpi();
        case LedOutputTypeArtnet:
            return ofxLedArtnet();
        default:
            assert(false);
    }
}
    
static LedOutputType GetLedOutputType(const LedOutput &output){
    if (eastl::holds_alternative<ofxLedRpi>(output))
        return LedOutputTypeLedmap;
    if (eastl::holds_alternative<ofxLedArtnet>(output))
        return LedOutputTypeArtnet;
    else
        assert(false);
}

static void LedOutputResetup(LedOutput &output)
{
    eastl::visit([](auto &out) { out.resetup(); }, output);
}

static bool LedOutputSend(LedOutput &output, ChannelsToPix pixels)
{
    bool result = false;
    eastl::visit([&result, &pixels](auto &out) { result = out.send(move(pixels)); }, output);
    return result;
}

static vector<string> LedOutputGetChannels(LedOutput &output)
{
    vector<string> channels;
    eastl::visit([&channels](auto &out) { channels = out.getChannels(); }, output);
    return channels;
}

static size_t LedOutputGetMaxPixels(LedOutput &output)
{
    size_t maxPixels;
    eastl::visit([&maxPixels](auto &out) { maxPixels = out.getMaxPixelsOut(); }, output);
    return maxPixels;
}

static void LedOutputSave(LedOutput &output, ofJson &config)
{
    eastl::visit([&config](auto &out) { out.saveJson(config); }, output);
}

static void LedOutputLoad(LedOutput &output, const ofJson &config)
{
    eastl::visit([&config](auto &out) { out.loadJson(config); }, output);
}
    
#ifndef LED_MAPPER_NO_GUI
/// Static function to generate universal container for controllers GUI

static void LedOutputBindGui(LedOutput &output, ofxDatGui *gui)
{
    eastl::visit([&gui](auto &out) { out.bindGui(gui); }, output);
}

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
