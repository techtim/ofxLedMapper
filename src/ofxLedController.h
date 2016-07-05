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
#include "ofxDatGui.h"

#ifdef USE_DMX_FTDI
    #include "ofxDmxFtdi.h"
#elif USE_DMX
    #include "ofxDmx.h"
#endif

#include "ofxLedGrabObject.h"

#define RPI_IP "192.168.2.10"
#define RPI_PORT 3000

#define LC_GUI_WIDTH 200

#define LCGUIButtonSend "Send"
#define LCGUIButtonDoubleLine "Double Line"
#define LCGUITextIP "IP"
#define LCGUITextPort "Port"
#define LCGUISliderPix "Pix in led"
#define LCGUIDropLedType "Led Type"
#define LCGUIButtonDmx "DMX"
#define LCFileName "Ctrl-"

enum LED_TYPE {
    LED_RGB,
    LED_RBG,
    LED_BRG,
    LED_BGR,
    LED_GRB,
    LED_GBR
};

static vector<string> ledTypes = {"RGB","RBG","BRG","BGR","GRB","GBR"};

class ofxLedController {
public:
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
    
    void onDropdownEvent(ofxDatGuiDropdownEvent e);
    void onButtonEvent(ofxDatGuiButtonEvent e);
    void onTextInputEvent(ofxDatGuiTextInputEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    void notifyParameterChanged(ofAbstractParameter & param);
   
    unsigned int getId() const;
    unsigned int getTotalLeds() const;
    unsigned char * getOutput();
    bool bSetuped;
    bool isSelected() {return bSelected; }
    void setupUdp(string host, unsigned int port);
    void sendUdp();
    void sendUdp(const ofPixels &sidesGrabImg);
    
    void setupDmx(string port_name);
    void sendDmx(const ofPixels &sidesGrabImg);
    
    void showGui(bool _show) {bShowGui = _show; bSelected = _show;};
    void updatePixels(const ofPixels &sidesGrabImg);

    const ofPixels & getPixels();
    void setPixels(const ofPixels & _pix);
    void setPixelsBetweenLeds(float dist) { pixelsInLed = dist; };
    
    void setSelected(bool state);

    void setGuiPosition(int x, int y);
    
    ofImage grabImg;
    
    void parseXml (ofxXmlSettings & XML);
    LED_TYPE getLedType(int num);
    void notifyDMXChanged(bool & param);
    
private:
    unsigned int _id;
    ofColor lineColor;
    float offBeg, offEnd;
    unsigned char *output;
    unsigned int totalLeds, pointsCount;
    
    vector<unique_ptr<ofxLedGrabObject>> Lines;
    int currentLine;
    
    bool bSelected, bShowGui;
    bool bRecordPoints, bDeletePoints, bRecordCircles;
    bool bSetupGui, bUdpSetup, bDmxSetup;
    
    ofVec2f posClicked;

    ofxXmlSettings XML;
    string path;

    ofxUDPManager udpConnection;
    // GUI
    ofxDatGui* gui;

    LED_TYPE ledType;
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
