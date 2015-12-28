//
//  ofxLedController.h
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxNetwork.h"
#include "ofxGui.h"
#include "ofxLedController.h"

typedef shared_ptr<ofxLedController> ofxLedController_ptr;

class ofxLedMapper {

public:
    ofxLedMapper();
    ofxLedMapper(int __id);
    ~ofxLedMapper();

    void update(const ofPixels &grabImg);
    void draw();
    bool load();
    bool save();
    bool add();
    
    void notifyParameterChanged(ofAbstractParameter & param);

private:

    vector<ofxLedController_ptr> Controllers;

    ofxXmlSettings XML;
    ofDirectory dir;
    
    int _id;
    bool bSetup = false;
    // GUI
    ofxPanel gui;
    ofParameterGroup guiGroup;
    ofParameter<ofColor> color;
    ofParameter<float> pixelsInLed;
    ofParameter<bool> bAdd;
    ofParameter<bool> bBlend;
    ofParameter<bool> bShowControllers;
    ofParameter<int> xPos, yPos;
    ofParameter<int> width, height;
    
    float curPixelsInLed , curXPos, curYPos;
};


