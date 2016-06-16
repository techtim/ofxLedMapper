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
#include "ofxDatGui.h"
#include "ofxLedController.h"

//typedef unique_ptr<ofxLedController> ofxLedController_ptr;

#define LM_GUI_WIDTH 200

#define LMGUIListControllers "Controllers"
#define LMGUIToggleDebug "Debug controller"
#define LMGUIButtonAdd "Add Controller"

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
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);

    void setGuiPosition(int x, int y);
    
    void keyPressed(ofKeyEventArgs& data);
    void keyReleased(ofKeyEventArgs& data);
    void windowResized(ofResizeEventArgs &args);
private:

    vector<unique_ptr<ofxLedController>> Controllers;
    unsigned int currentCtrl;
    ofxXmlSettings XML;
    ofDirectory dir;
    
    int _id;
    bool bSetup = false;
    // GUI
    unique_ptr<ofxDatGui> gui;
    unique_ptr<ofxDatGuiScrollView> listControllers;
    ofxDatGuiToggle * toggleDebugController;
    bool bAdd;
    bool bShowControllers;
};


