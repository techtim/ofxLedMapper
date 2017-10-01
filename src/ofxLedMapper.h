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
#include "Common.h"


class ofxLedMapper {

public:
    ofxLedMapper();
    ofxLedMapper(int __id);
    ~ofxLedMapper();

    void update(const ofPixels &grabImg);
    void draw();
    bool add(string folder_path);
    bool add(unsigned int _ctrlId, string folder_path);
    bool remove(unsigned int _ctrlId);
    bool load();
    bool save();

    bool checkUniqueId(unsigned int _ctrlId);

    void setupGui();
    void setGuiPosition(int x, int y);
    void setCurrentController(unsigned int _curCtrl);
    void updateControllersListGui();

#ifndef LED_MAPPER_NO_GUI
    void onScrollViewEvent(ofxDatGuiScrollViewEvent e);
    void onButtonClick(ofxDatGuiButtonEvent e);
    void onSliderEvent(ofxDatGuiSliderEvent e);
#endif

    void keyPressed(ofKeyEventArgs& data);
    void keyReleased(ofKeyEventArgs& data);
    void windowResized(ofResizeEventArgs &args);

private:

    map<size_t, unique_ptr<ofxLedController>> Controllers;
    unsigned int currentCtrl;
    ofxXmlSettings XML;
    ofDirectory dir;
    string configFolderPath;
    int _id;
    bool bSetup = false;
#ifndef LED_MAPPER_NO_GUI
    // GUI
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGuiScrollView> m_listControllers;
    unique_ptr<ofxDatGuiTheme> guiTheme;
    ofxDatGuiToggle* m_toggleDebugController;
    ofxDatGuiToggle* m_togglePlay;
    ofxDatGuiSlider* m_fpsSlider;
#endif
    bool bAdd;
    bool bShowControllers;
};


