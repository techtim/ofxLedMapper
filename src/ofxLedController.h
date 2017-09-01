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
    
    ofxLedController(const int& __id, const string & _path);
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
    void setupUdp(string host, unsigned int port);
    void sendUdp();
    void sendUdp(const ofPixels &sidesGrabImg);
    
    void setupDmx(string port_name);
    void sendDmx(const ofPixels &grabbedImg);
    
    void showGui(bool _show) {bShowGui = _show; bSelected = _show;};
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
    
    void notifyDMXChanged(bool & param);
    
private:
    unsigned int _id;
    ofColor lineColor;
    float offBeg, offEnd;
    
    vector<char> output;
    
    unsigned int totalLeds, pointsCount;

    std::function<void(vector<char> &output, ofColor &color)> colorUpdator;
    
    vector<unique_ptr<ofxLedGrabObject>> m_lines;
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
    ofxDatGui* gui;
#endif
    COLOR_TYPE colorType;
    float pixelsInLed;
    bool bUdpSend, bDmxSend, bDoubleLine;
    int xPos, yPos, width, height;
    int offsetBegin, offsetEnd;
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
