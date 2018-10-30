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

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxXmlSettings.h"

#include "Common.h"

#ifdef USE_DMX_FTDI
#include "ofxDmxFtdi.h"
#elif USE_DMX
#include "ofxDmx.h"
#endif

#include "ofxLedGrabObject.h"

namespace LedMapper {

using OnControllerStatusChange = function<void(void)>;
using ChannelsGrabObjects = vector<vector<unique_ptr<ofxLedGrab>>>;

/// Class represents connection to one client recieving led data and
/// control transmittion params like fps, pixel color order, LED IC Type

template<typename LedOut>
class ofxLedController {
public:
    enum COLOR_TYPE { RGB = 0, RBG = 1, BRG = 2, BGR = 3, GRB = 4, GBR = 5 };
    enum LED_TYPE { DATA, DATACLOCK };

    ofxLedController() = delete;
    ofxLedController(const ofxLedController &) = delete;
    ofxLedController(ofxLedController &&) = delete;
    ofxLedController(const int &__id, const string &_path);
    void setOnControllerStatusChange(function<void(void)> callback);
    ~ofxLedController();

    void save(const string &path);
    void load(const string &path);

    static unique_ptr<ofxLedGrab> GetUniqueTypedGrab(int type, const ofxLedGrab &grab);
    void addGrab(unique_ptr<ofxLedGrab> &&object);
    void deleteSelectedGrabs();
    void draw();

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

    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);

    ofVec2f getGuiSize() const { return ofVec2f(LM_GUI_WIDTH, 100); }
#endif

    const ChannelsGrabObjects &peekGrabObjects() const;
    const vector<unique_ptr<ofxLedGrab>> &peekCurrentGrabs() const { return *m_currentChannel; };

    unsigned int getId() const;
    unsigned int getTotalLeds() const;

    bool isSelected() const { return bSelected; }
    bool isStatusOk() const { return m_statusOk; }
    bool isSending() const { return bUdpSend; }
    
    void setupUdp(const string &host, unsigned int port);
    void sendLedType(const string &ledType);
    void sendUdp();
    void sendUdp(const ofPixels &sidesGrabImg);
    void sendArtnet(const ofPixels &data);

    string getIP() const { return m_curUdpIp; }

    void setupDmx(const string &port_name);
    void sendDmx(const ofPixels &grabbedImg);

    void markDirtyGrabPoints() { m_bDirtyPoints = true; }
    void updateGrabPoints();
    void updatePixels(const ofPixels &grabbedImg);

    void setFps(float fps);
    void setSelected(bool state);
    void setGrabsSelected(bool state);
    void setPixelsBetweenLeds(float dist) { m_pixelsInLed = dist; };
    void setGrabType(LMGrabType type) { m_currentGrabType = type; }

    COLOR_TYPE getColorType(int num) const;
    void setColorType(COLOR_TYPE);

    const ofRectangle &peekBounds() const { return m_grabBounds; }

private:
    unsigned int m_id;
    ofColor m_colorLine, m_colorActive, m_colorInactive;

    vector<char> m_output;
    LedOut m_ledOut;
    unsigned int m_totalLeds, m_pointsCount, m_outputHeaderOffset;

    std::function<void(vector<char> &output, ofColor &color)> m_colorUpdator;
    function<void(void)> m_statusChanged;

    void setCurrentChannel(int);
    ChannelsGrabObjects m_channelGrabObjects;
    vector<unique_ptr<ofxLedGrab>> *m_currentChannel;
    size_t m_currentChannelNum;
    string m_currentLedType;

    vector<uint16_t> m_channelTotalLeds;
    vector<LedMapper::Point> m_ledPoints;

    bool bSelected, bDeletePoints;
    bool bUdpSetup, bDmxSetup;

    LMGrabType m_currentGrabType;

    ofRectangle m_grabBounds;

    ofxXmlSettings XML;
    string path;
    void parseXml(ofxXmlSettings &XML);

    ofxUDPManager m_frameConnection, m_confConnection;

    COLOR_TYPE m_colorType;
    float m_pixelsInLed;
    bool bUdpSend, bDmxSend, bDoubleLine, m_statusOk, m_bDirtyPoints;
    int m_fps;

    int dmxChannel;
    string m_udpIp, m_curUdpIp;
    int m_udpPort, m_curUdpPort;

    uint64_t m_lastFrameTime, m_msecInFrame;

// DMX
#ifdef USE_DMX
    ofxDmx dmx;
#elif USE_DMX_FTDI
    ofxDmxFtdi dmxFtdi;
    unsigned char dmxFtdiVal[513];
#endif
};

} // namespace LedMapper
