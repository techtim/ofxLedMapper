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

using OnControllerStatusChange = function<void(void)>;
using ChannelsGrabObjects = vector<vector<unique_ptr<ofxLedGrabObject>>>;

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

    void setupGui();
    void draw();
    void mousePressed(ofMouseEventArgs &args);
    void mouseDragged(ofMouseEventArgs &args);
    void mouseReleased(ofMouseEventArgs &args);
    void keyPressed(ofKeyEventArgs &data);
    void keyReleased(ofKeyEventArgs &data);

#ifndef LED_MAPPER_NO_GUI
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);

    ofVec2f getGuiSize() const { return m_gui != nullptr ? ofVec2f(m_gui->getWidth(), m_gui->getHeight()) : ofVec2f(0); }
#endif

    const ChannelsGrabObjects &peekGrabObjects() const;
    unsigned int getId() const;
    unsigned int getTotalLeds() const;
    unsigned char *getOutput();
    bool bSetuped;
    bool isSelected() const { return bSelected; }
    bool isStatusOk() const { return m_statusOk; }
    void setupUdp(const string &host, unsigned int port);
    void sendUdp();
    void sendUdp(const ofPixels &sidesGrabImg);
    string getIP() const { return m_curUdpIp; }

    void setupDmx(const string &port_name);
    void sendDmx(const ofPixels &grabbedImg);

    void showGui(bool _show)
    {
        bShowGui = _show;
        bSelected = _show;
    }

    void markDirtyGrabPoints() { m_bDirtyPoints = true; }
    void updateGrabPoints();
    void updatePixels(const ofPixels &grabbedImg);

    const ofPixels &getPixels();
    void setPixels(const ofPixels &_pix);
    void setPixelsBetweenLeds(float dist) { m_pixelsInLed = dist; };
    void setFps(float fps) { m_fps = fps; m_msecInFrame = 1000.0 / m_fps; }

    void setSelected(bool state);
    void setGuiPosition(int x, int y);

    ofImage grabImg;

    void parseXml(ofxXmlSettings &XML);

    COLOR_TYPE getColorType(int num) const;
    void setColorType(COLOR_TYPE);
    void setCurrentChannel(int);
    void notifyDMXChanged(bool &param);

    const ofRectangle &peekBounds() const { return m_grabBounds; }
private:
    unsigned int _id;
    ofColor lineColor;

    vector<char> m_output;

    unsigned int m_totalLeds, m_pointsCount, m_outputHeaderOffset;

    std::function<void(vector<char> &output, ofColor &color)> m_colorUpdator;

    function<void(void)> m_statusChanged;

    ChannelsGrabObjects m_channelGrabObjects;
    vector<unique_ptr<ofxLedGrabObject>> *m_currentChannel;
    size_t m_currentChannelNum;
    vector<uint16_t> m_channelTotalLeds;
    vector<LedMapper::Point> m_ledPoints;
    int currentLine;

    bool bSelected, bShowGui;
    bool bDeletePoints;
    bool bSetupGui, bUdpSetup, bDmxSetup;

    ofxLedGrabObject::GRAB_TYPE m_recordGrabType;

    ofRectangle m_grabBounds;
    ofVec2f posClicked;

    ofxXmlSettings XML;
    string path;

    ofxUDPManager udpConnection;

    COLOR_TYPE colorType;
    float m_pixelsInLed, m_fps;
    bool bUdpSend, bDmxSend, bDoubleLine, m_statusOk, m_bDirtyPoints;
    int xPos, yPos, width, height;

    int dmxChannel;
    string m_udpIp, m_curUdpIp;
    int m_udpPort, m_curUdpPort;

    uint64_t m_lastFrameTime, m_msecInFrame;
// GUI
#ifndef LED_MAPPER_NO_GUI
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGui> m_grabObjGui;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;
    ofxDatGuiTextInput *m_ipInput;
    ofxDatGuiTextInput *m_portInput;
#endif

// DMX
#ifdef USE_DMX
    ofxDmx dmx;
#elif USE_DMX_FTDI
    ofxDmxFtdi dmxFtdi;
    unsigned char dmxFtdiVal[513];
#endif

};

