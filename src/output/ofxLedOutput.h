//
// Created by Timofey Tavlintsev on 30/10/2018.
//

#pragma once

#include "ofxLedArtnet.h"
#include "ofxLedRpi.h"
/// In development
// #include "ofxLedDmx.h"

#include "ofxLedGrabObject.h"

namespace LedMapper {

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

template <typename T> void sendOutput(T &x, ChannelsToPix &&outputData)
{
    ofLogError() << "Un implemented 'draw' for class" << typeid(x).name();
}
template <typename T> void bindGui(T &x, ofxDatGui *gui)
{
    ofLogError() << "Un implemented 'bindGui' for class" << typeid(x).name();
}
template <typename T> void saveJson(const T &x, ofJson &config)
{
    ofLogError() << "Un implemented 'saveJson' for class" << typeid(x).name();
}

template <typename T> void loadJson(T &x, const ofJson &config)
{
    ofLogError() << "Un implemented 'loadJson' for class" << typeid(x).name();
}

enum OutputType { LedRpi, ArtNet };

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

/// Expose all ofxLedController methods for heterogeneous use here
class CtrlWrapper {
public:
    template <typename T>
    CtrlWrapper(T x)
        : self_(make_unique<Ctrl<T>>(move(x)))
    {
    }

    friend void save(CtrlWrapper &x, const string &path) { x.self_->save_(path); }
    friend void load(CtrlWrapper &x, const string &path) { x.self_->load_(path); }
    friend void addGrab(CtrlWrapper &x, unique_ptr<ofxLedGrab> &&object)
    {
        x.self_->addGrab_(move(object));
    }
    friend void setOnControllerStatusChange(CtrlWrapper &x, function<void(void)> callback)
    {
        x.self_->setOnControllerStatusChange_(callback);
    }
#ifndef LED_MAPPER_NO_GUI
    friend void bindGui(CtrlWrapper &x, ofxDatGui *gui) { x.self_->bindGui_(gui); }
#endif
    friend void setSelected(CtrlWrapper &x, bool state) { x.self_->setSelected_(state); }
    friend void setGrabsSelected(CtrlWrapper &x, bool state) { x.self_->setGrabsSelected_(state); }
    friend void setGrabType(CtrlWrapper &x, LMGrabType type) { x.self_->setGrabType_(type); }
    friend void deleteSelectedGrabs(CtrlWrapper &x) { x.self_->deleteSelectedGrabs_(); }
    friend void draw(CtrlWrapper &x) { x.self_->draw_(); }
    friend void disableEvents(CtrlWrapper &x) { x.self_->disableEvents_(); }
    friend bool isSelected(CtrlWrapper &x) { return x.self_->isSelected_(); }
    friend bool isStatusOk(CtrlWrapper &x) { return x.self_->isStatusOk_(); }
    friend bool isSending(CtrlWrapper &x) { return x.self_->isSending_(); }
    friend string getIP(CtrlWrapper &x) { return x.self_->getIP_(); }
    friend const vector<unique_ptr<ofxLedGrab>> &peekCurrentGrabs(CtrlWrapper &x)
    {
        return x.self_->peekCurrentGrabs_();
    }

private:
    struct concept_t {
        virtual ~concept_t() = default;
        virtual void save_(const string &path) = 0;
        virtual void load_(const string &path) = 0;
        virtual void addGrab_(unique_ptr<ofxLedGrab> &&object) = 0;
        virtual void setOnControllerStatusChange_(function<void(void)> callback) = 0;
        virtual void bindGui_(ofxDatGui *gui) = 0;
        virtual void setSelected_(bool state) = 0;
        virtual void setGrabsSelected_(bool state) = 0;
        virtual void setGrabType_(LMGrabType type) = 0;
        virtual void deleteSelectedGrabs_() = 0;
        virtual void draw_() = 0;
        virtual void disableEvents_() = 0;
        virtual bool isSelected_() = 0;
        virtual bool isStatusOk_() = 0;
        virtual bool isSending_() = 0;
        virtual string getIP_() = 0;
        virtual const vector<unique_ptr<ofxLedGrab>> &peekCurrentGrabs_() = 0;
    };

    template <typename T> struct Ctrl final : concept_t {
        Ctrl(T x)
            : out_(move(x))
        {
        }

        void save_(const string &path) { out_.save(path); }
        void load_(const string &path) { out_.load(path); }
        void addGrab_(unique_ptr<ofxLedGrab> &&object) { out_.addGrab(move(object)); }
        void setOnControllerStatusChange_(function<void(void)> callback)
        {
            out_.setOnControllerStatusChange(callback);
        }
        void bindGui_(ofxDatGui *gui) { out_.bindGui(gui); }
        void setSelected_(bool state) { out_.setSelected(state); }
        void setGrabsSelected_(bool state) { out_.setGrabsSelected(state); }
        void setGrabType_(LMGrabType type) { out_.setGrabType(type); }
        void deleteSelectedGrabs_() { out_.deleteSelectedGrabs(); }
        void draw_() { out_.draw(); }
        void disableEvents_() { out_.disableEvents(); }
        bool isSelected_() { return out_.isSelected(); }
        bool isStatusOk_() { return out_.isStatusOk(); }
        bool isSending_() { return out_.isSending(); }
        string getIP_() { return out_.getIP(); }
        const vector<unique_ptr<ofxLedGrab>> &peekCurrentGrabs_() { return out_.peekCurrentGrabs(); }

        T out_;
    };

    unique_ptr<concept_t> self_;
//    shared_ptr<const concept_t> self_;
};

} // namespace LedMapper