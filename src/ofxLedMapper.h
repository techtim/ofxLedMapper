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

#include "Common.h"
#include "ofMain.h"
#include "ofxLedController.h"
#include "ofxNetwork.h"
#include "ofxXmlSettings.h"

namespace LedMapper {

class ofxLedMapper {

public:
    ofxLedMapper();
    ~ofxLedMapper();

    void update(const ofPixels &grabImg);
    void draw();
    void drawGui();
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

    void keyPressed(ofKeyEventArgs &data);
    void keyReleased(ofKeyEventArgs &data);
    void windowResized(ofResizeEventArgs &args);

private:
    map<size_t, unique_ptr<ofxLedController>> m_controllers;
    unsigned int m_currentCtrl;
    ofxXmlSettings XML;
    ofDirectory dir;
    string m_configFolderPath;
    bool m_bSetup;

#ifndef LED_MAPPER_NO_GUI
    // GUI
    unique_ptr<ofxDatGui> m_gui;
    unique_ptr<ofxDatGui> m_guiController;
    unique_ptr<ofxDatGuiScrollView> m_listControllers;
    unique_ptr<ofxDatGuiTheme> guiTheme;
    ofxDatGuiToggle *m_toggleDebugController;
    ofxDatGuiToggle *m_togglePlay;
#endif

    bool bAdd;
    bool bShowControllers;
};

} // namespace LedMapper