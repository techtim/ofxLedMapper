//
//  ofxLedController.h
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#ifndef __ofxLedController__
#define __ofxLedController__

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxNetwork.h"

#include "Common.h"

#ifdef USE_DMX_FTDI
    #include "ofxDmxFtdi.h"
#elif USE_DMX
    #include "ofxDmx.h"
#endif

#include "ofxLedGrabObject.h"

using OnControllerStatusChange = function<void(void)>;

class ofxLedController {
public:
    enum COLOR_TYPE {
        RGB = 0,
        RBG = 1,
        BRG = 2,
        BGR = 3,
        GRB = 4,
        GBR = 5
    };
    enum LED_TYPE{
        DATA,
        DATACLOCK
    };
    ofxLedController() = delete;
    ofxLedController(const ofxLedController &) = delete;
    ofxLedController(const int& __id, const string & _path);
    void setOnControllerStatusChange(function<void(void)> callback);
    ~ofxLedController();
    
    void save(string path);
    void load(string path);
    
    void setupGui();
    void draw();
    void mousePressed(ofMouseEventArgs & args);
    void mouseDragged(ofMouseEventArgs & args);
    void mouseReleased(ofMouseEventArgs & args);
    void keyPressed(ofKeyEventArgs& data);
    void keyReleased(ofKeyEventArgs& data);

#ifndef LED_MAPPER_NO_GUI
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
#endif
   
    unsigned int getId() const;
    unsigned int getTotalLeds() const;
    unsigned char * getOutput();
    bool bSetuped;
    bool isSelected() const { return bSelected; }
    bool isStatusOk() const { return m_statusOk; }
    void setupUdp(string host, unsigned int port);
    void sendUdp();
    void sendUdp(const ofPixels &sidesGrabImg);
    
    void setupDmx(string port_name);
    void sendDmx(const ofPixels &grabbedImg);
    
    void showGui(bool _show) { bShowGui = _show; bSelected = _show; }
    
    void markDirtyGrabPoints() { m_bDirtyPoints = true; }
    void updateGrabPoints();
    void updatePixels(const ofPixels &grabbedImg);

    const ofPixels & getPixels();
    void setPixels(const ofPixels & _pix);
    void setPixelsBetweenLeds(float dist) { pixelsInLed = dist; };
    
    void setSelected(bool state);

    void setGuiPosition(int x, int y);
    
    ofImage grabImg;
    
    void parseXml (ofxXmlSettings & XML);

    COLOR_TYPE getColorType(int num) const;
    void setColorType(COLOR_TYPE);
    void setCurrentChannel(int);
    void notifyDMXChanged(bool & param);
    
private:
    unsigned int _id;
    ofColor lineColor;
    
    vector<char> m_output;
    
    unsigned int m_totalLeds, pointsCount, m_outputHeaderOffset;

    std::function<void(vector<char> &output, ofColor &color)> colorUpdator;

    function<void(void)> m_statusChanged;
    
    vector<vector<unique_ptr<ofxLedGrabObject>>> m_channelGrabObjects;
    vector<unique_ptr<ofxLedGrabObject>> *m_currentChannel;
    size_t m_currentChannelNum;
    vector<uint16_t> m_channelTotalLeds;
    vector<LedMapper::Point> m_ledPoints;
    int currentLine;
    
    bool bSelected, bShowGui;
    bool bDeletePoints;
    bool bSetupGui, bUdpSetup, bDmxSetup;
    
    ofxLedGrabObject::GRAB_TYPE m_recordGrabType;
    
    ofVec2f posClicked;

    ofxXmlSettings XML;
    string path;

    ofxUDPManager udpConnection;
    // GUI
#ifndef LED_MAPPER_NO_GUI
    shared_ptr<ofxDatGui> gui;
    shared_ptr<ofxDatGui> grabObjGui;
    unique_ptr<ofxDatGuiTheme> guiTheme;
#endif
    COLOR_TYPE colorType;
    float pixelsInLed;
    bool bUdpSend, bDmxSend, bDoubleLine, m_statusOk, m_bDirtyPoints;
    int xPos, yPos, width, height;

    int dmxChannel;
    string udpIpAddress, cur_udpIpAddress;
    int udpPort, cur_udpPort;
    
    // DMX
#ifdef USE_DMX
    ofxDmx dmx;
#elif USE_DMX_FTDI
    ofxDmxFtdi dmxFtdi;
#endif
    unsigned char dmxFtdiVal[513];
};

#endif /* defined(ofx__ledGipsy__LedController__) */
