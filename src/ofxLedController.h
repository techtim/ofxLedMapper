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
#include "ofxGui.h"
#include "ofxTextInputField.h"
#include "ofxDmx.h"

#ifdef USE_DMX_FTDI
#include "ofxDmxFtdi.h"
#endif

#include "ofxLedGrabObject.h"

#define POINT_RAD 10
#define RPI_IP "192.168.2.10"
#define RPI_PORT 3000

enum {
    LED_RGB,
    LED_RBG,
    LED_BRG,
    LED_GRB,
    LED_GBR
};



class ofxLedController {
public:
    ofxLedController();
    ~ofxLedController();
    void setup(const int& __id, const string & _path, const ofRectangle& _region, int _maxWidth=0, int _maxHeight=0);
    
    void save(string path);
    void load(string path);
    
    void draw();
    void mousePressed(ofMouseEventArgs & args);
    void mouseDragged(ofMouseEventArgs & args);
    void mouseReleased(ofMouseEventArgs & args);
    void keyPressed(ofKeyEventArgs& data);
    void keyReleased(ofKeyEventArgs& data);
    
    void notifyParameterChanged(ofAbstractParameter & param);
    
    void addLine(ofxLedGrabObject * tmpObj);
    void addLine(int x1, int y1, int x2, int y2);
    
    unsigned int getLedsCount() { return totalLeds; };
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
//    void guiEvent(ofParameterGroup &e);
    
    void setSelected(bool state) {bSelected = state;};


    
    ofImage grabImg;
    vector<ofxLedGrabObject *> Lines;
    
    ofxTextInputField ipText;
    
    void parseXml (ofxXmlSettings & XML);
    
    void notifyDMXChanged(bool & param);
    void notifyUDPChanged(bool & param);
    
private:
    ofRectangle region;
//    float height, xPos, yPos;
    ofColor lineColor;
//    unsigned int offsetBegin, offsetEnd;
    float offBeg, offEnd;
    
    unsigned int totalLeds, pointsCount;
    int maxWidth, maxHeight;
    
    unsigned int _id;
    float fLedType;

    string udpHost;
    unsigned int udpPort;
    bool bSelected, bShowGui;
    bool bRecordPoints, bDeletePoints, bRecordCircles;
    bool bUdpSetup, bDmxSetup;
    
    int currentLine;
    
    ofVec2f posClicked;
//    float pixelsInLed, ledOffset;
    
    ofxUDPManager udpConnection;
    
    // DMX
    ofxDmx dmx;
//    ofxDmxFtdi dmxFtdi;
    unsigned char dmxFtdiVal[513];

    unsigned char *output;
    
    ofxXmlSettings XML;
    string path;
    // GUI
    ofxPanel gui;
    ofParameterGroup guiGroup;
    ofParameter<ofColor> color;
    ofParameter<float> pixelsInLed;
    ofParameter<float> Alpha;
    ofParameter<ofColor> color2;
    ofParameter<bool> bUdpSend;
    ofParameter<bool> bDmxSend;
    ofParameter<bool> bDoubleLine;
    ofParameter<int> xPos, yPos;
    ofParameter<int> width, height;
    ofParameter<int> ledType;
    ofParameter<bool> record;
    ofParameter<int> offsetBegin;
    ofParameter<int> offsetEnd;
    ofParameter<string> ipAddress;
    ofParameter<int> port;
    ofParameter<int> dmxChannel;
    
    float curPixelsInLed , curXPos, curYPos;
};

#endif /* defined(ofx__ledGipsy__LedController__) */
