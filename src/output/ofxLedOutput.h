//
// Created by Timofey Tavlintsev on 30/10/2018.
//

#pragma once

#include "ofxLedRpi.h"

/// In development
#include "ofxLedArtnet.h"
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

// template <typename T> void sendOutput(T &x, ChannelsToPix &&outputData)
//{
//    ofLogError() << "Un implemented 'draw' for class" << typeid(x).name();
//}
// template <typename T> void bindGui(T &x, ofxDatGui *gui) {
//    ofLogError() << "Un implemented 'bindGui' for class" << typeid(x).name();
//}
// template <typename T> void saveJson(const T &x, ofJson &config) {
//    ofLogError() << "Un implemented 'saveJson' for class" << typeid(x).name();
//}
//
// template <typename T> void loadJson(T &x, const ofJson &config) {
//    ofLogError() << "Un implemented 'loadJson' for class" << typeid(x).name();
//}

enum OutputType {
    LedRpi,
    ArtNet
};

class OutputWrapper {
public:
    template <typename T>
    OutputWrapper(T x)
        : self_(make_unique<Output<T>>(move(x)))
    {
    }
    friend void sendOutput(OutputWrapper &x, ChannelsToPix &&outData)
    {
        x.self_->sendOutput_(move(outData));
    }
    friend void bindGui(OutputWrapper &x, ofxDatGui *gui) { x.self_->bindGui_(gui); }
    friend void saveJson(const OutputWrapper &x, ofJson &config) { x.self_->saveJson_(config); }
    friend void loadJson(OutputWrapper &x, const ofJson &config) { x.self_->loadJson_(config); }
    friend vector<string> getChannels(const OutputWrapper &x) { return x.self_->getChannels_(); }
    friend size_t getMaxPixelsOut(const OutputWrapper &x) { return x.self_->getMaxPixelsOut_(); }

private:
    struct concept_t {
        virtual ~concept_t() = default;
        virtual void sendOutput_(ChannelsToPix &&outData) = 0;
        virtual void bindGui_(ofxDatGui *gui) = 0;
        virtual void saveJson_(ofJson config) const = 0;
        virtual void loadJson_(const ofJson config) = 0;
        virtual vector<string> getChannels_() = 0;
        virtual size_t getMaxPixelsOut_() = 0;
    };

    template <typename T> struct Output final : concept_t {
        Output(T x)
            : out_(move(x))
        {
        }
        void sendOutput_(ChannelsToPix &&outData) override { out_.send(move(outData)); }
        void bindGui_(ofxDatGui *gui) override { out_.bindGui(gui); };
        void saveJson_(ofJson config) const { out_.saveJson(config); }
        void loadJson_(const ofJson config) { out_.loadJson(config); }
        vector<string> getChannels_() { return out_.getChannels(); }
        size_t getMaxPixelsOut_() { return out_.getMaxPixelsOut(); }
        T out_;
    };
    unique_ptr<concept_t> self_;
};