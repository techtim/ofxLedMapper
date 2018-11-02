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

#include "Common.h"
#include "ofMain.h"
#include "ofxLedGrabObject.h"
#include "ofxXmlSettings.h"
#include "output/ofxLedOutput.h"

namespace LedMapper {

using OnControllerStatusChange = function<void(void)>;
using ChannelsGrabObjects = vector<vector<unique_ptr<ofxLedGrab>>>;

/// Class represents connection to one client recieving led data and
/// control transmittion params like fps, pixel color order, LED IC Type

class ofxLedController {
public:
    ofxLedController(const int _id, const string &_path);
    ofxLedController() = delete;
    ofxLedController(const ofxLedController &) = delete;
    ofxLedController(ofxLedController &&) = delete;

    void setOnControllerStatusChange(function<void(void)> callback);
    ~ofxLedController();

    void save(const string &path);
    void load(const string &path);

    static unique_ptr<ofxLedGrab> GetUniqueTypedGrab(int type, const ofxLedGrab &grab);
    void addGrab(unique_ptr<ofxLedGrab> &&object);
    void deleteSelectedGrabs();
    void draw();

    void send(const ofTexture &texIn);

    /// mouse and keyboard events
    void mousePressed(ofMouseEventArgs &args);
    void mouseDragged(ofMouseEventArgs &args);
    void mouseReleased(ofMouseEventArgs &args);
    void keyPressed(ofKeyEventArgs &data);
    void keyReleased(ofKeyEventArgs &data);
    /// turn off global ofListeners for mouse and keyboard events
    /// used by ofxLedMapper
    void disableEvents();

#ifndef LED_MAPPER_NO_GUI
    static unique_ptr<ofxDatGui> GenerateGui();
    void bindGui(ofxDatGui *gui);
    ofVec2f getGuiSize() const { return ofVec2f(LM_GUI_WIDTH, 100); }
#endif

    const ChannelsGrabObjects &peekGrabObjects() const { return m_channelGrabObjects; };
    const vector<unique_ptr<ofxLedGrab>> &peekCurrentGrabs() const { return *m_currentChannel; };

    bool isSelected() const { return m_bSelected; }
    bool isStatusOk() const { return m_statusOk; }
    bool isSending() const { return m_bSend; }

    string getIP() const { return m_ledOut.getIP(); }
    unsigned int getId() const { return m_id; }
    unsigned int getTotalLeds() const { return m_totalLeds; }

    void markDirtyGrabPoints() { m_bDirtyPoints = true; }
    void setPixInLed(const float pixInled);
    void updateGrabPoints();
    ChannelsToPix updatePixels(const ofTexture &);

    void setFps(float fps);
    void setSelected(bool state);
    void setGrabsSelected(bool state);
    void setGrabType(LMGrabType type) { m_currentGrabType = type; }

    GRAB_COLOR_TYPE getColorType(int num) const;
    void setColorType(GRAB_COLOR_TYPE);

    const ofRectangle &peekBounds() const { return m_grabBounds; }

private:
    unsigned int m_id;
    string m_path;

    bool m_bSelected, m_bSend, m_statusOk, m_bDirtyPoints;
    ofColor m_colorLine, m_colorActive, m_colorInactive;

    unsigned int m_totalLeds;
    vector<char> m_output;
    ofxLedRpi m_ledOut;

    ofVboMesh m_vboLeds;
    ofShader m_shaderGrab;
    ofFbo m_fboLeds;
    ofPixels m_pixels;

    std::function<void(vector<char> &output, ofColor &color)> m_colorUpdator;
    function<void(void)> m_statusChanged;

    void setCurrentChannel(int);
    ChannelsGrabObjects m_channelGrabObjects;
    vector<unique_ptr<ofxLedGrab>> *m_currentChannel;
    size_t m_currentChannelNum;

    vector<string> m_channelList;
    vector<uint16_t> m_channelsTotalLeds;
    vector<glm::vec3> m_ledPoints;

    LMGrabType m_currentGrabType;
    ofRectangle m_grabBounds;

    ofxXmlSettings XML;
    void parseXml(ofxXmlSettings &XML);

    GRAB_COLOR_TYPE m_colorType;
    float m_pixelsInLed;
    int m_fps;

    uint64_t m_lastFrameTime, m_msecInFrame;
};

} // namespace LedMapper
