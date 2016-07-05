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
#define LMGUITogglePlay "Play"
#define LMGUISliderFps "FPS"
#define LMGUIButtonAdd "Add Controller"
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
    
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
    
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
    ofxDatGui* gui;
    ofxDatGuiScrollView* listControllers;
    ofxDatGuiToggle * toggleDebugController;
    ofxDatGuiToggle * togglePlay;
    ofxDatGuiSlider * fpsSlider;
    bool bAdd;
    bool bShowControllers;
};


