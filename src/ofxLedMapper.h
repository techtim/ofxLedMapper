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
#include "ofxLedController.h"

#ifndef LED_MAPPER_NO_GUI

#include "ofxDatGui.h"
#define LM_GUI_WIDTH 200
#define LMGUIListControllers "Controllers"
#define LMGUIToggleDebug "Debug controller"
#define LMGUITogglePlay "Play"
#define LMGUISliderFps "FPS"
#define LMGUIButtonAdd "Add Controller"

#endif

//typedef unique_ptr<ofxLedController> ofxLedController_ptr;

#define LMCtrlsFolderPath "Ctrls"

class ofxLedMapper {

public:
    ofxLedMapper();
    ofxLedMapper(int __id);
    ~ofxLedMapper();

    void update(const ofPixels &grabImg);
    void draw();
    bool add();
    bool add(unsigned int _ctrlId);
    bool remove(unsigned int _ctrlId);
    bool load();
    bool save();
    
    bool checkUniqueId(unsigned int _ctrlId);

    void setupGui();
    void setGuiPosition(int x, int y);
    void setCurrentController(unsigned int _curCtrl);

#ifndef LED_MAPPER_NO_GUI
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
#endif
    
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
#ifndef LED_MAPPER_NO_GUI
    // GUI
    ofxDatGui* gui;
    ofxDatGuiScrollView* listControllers;
    ofxDatGuiToggle * toggleDebugController;
    ofxDatGuiToggle * togglePlay;
    ofxDatGuiSlider * fpsSlider;
#endif
    bool bAdd;
    bool bShowControllers;
};


