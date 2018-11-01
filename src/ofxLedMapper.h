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

    void update();
    void draw();
    void drawGui();
    void send(const ofTexture &);
    bool add(string folder_path);
    bool add(unsigned int _ctrlId, string folder_path);
    bool remove(unsigned int _ctrlId);
    bool load(string folderPath);
    bool load();
    bool save(string folderPath);
    bool save();

    bool checkUniqueId(unsigned int _ctrlId);

    void setupGui();
    void setGuiPosition(int x, int y);
    void setGuiActive(bool);
    void setCurrentController(unsigned int _curCtrl);
    void updateControllersListGui();

    void mousePressed(ofMouseEventArgs &args);
    void mouseDragged(ofMouseEventArgs &args);
    void mouseReleased(ofMouseEventArgs &args);
    void keyPressed(ofKeyEventArgs &data);
    void keyReleased(ofKeyEventArgs &data);
    void windowResized(ofResizeEventArgs &args);

private:
    map<size_t, unique_ptr<ofxLedController>> m_controllers;
    unsigned int m_currentCtrl;
    ofxXmlSettings XML;
    ofDirectory m_dir;
    string m_configFolderPath;
    bool m_bSetup, m_bControlPressed;
    LMGrabType m_grabTypeSelected;

    void copyGrabs();
    void pasteGrabs();
    void removeGrabs();
    vector<unique_ptr<ofxLedGrab>> m_copyPasteGrabs;
#ifndef LED_MAPPER_NO_GUI
    // GUI

    unique_ptr<ofxDatGui> m_gui, m_guiController, m_iconsMenu;
    unique_ptr<ofxDatGuiScrollView> m_listControllers;
    unique_ptr<ofxDatGuiTheme> m_guiTheme;
    ofxDatGuiToggle *m_toggleDebugController;
    ofxDatGuiToggle *m_togglePlay;
#endif

};

} // namespace LedMapper