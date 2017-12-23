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

#include "ofxLedMapper.h"
#include <math.h>
#include <regex>

ofxLedMapper::ofxLedMapper(int __id)
    : _id(__id)
    , m_bSetup(false)
#ifdef WIN32
    , configFolderPath(LedMapper::CONFIG_PATH + LedMapper::APP_NAME + "\\")
#elif defined(__APPLE__)
    , configFolderPath(LedMapper::CONFIG_PATH + LedMapper::APP_NAME + "/")
#endif
{
    setupGui();

    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);

    /// add default ctrl
    add(0, configFolderPath);
    setCurrentController(0);

    m_bSetup = true;
}

ofxLedMapper::~ofxLedMapper()
{
    ofLogVerbose("[ofxLedMapper] Detor: clear controllers + remove event listeners");

    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofRemoveListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
}

void ofxLedMapper::draw()
{
    if (!m_bSetup)
        return;
#ifndef LED_MAPPER_NO_GUI
    m_gui->update();
    m_gui->draw();
    m_listControllers->update();
    m_listControllers->draw();
#endif
    if (!Controllers.empty()) {
// If no debug toggled show lines from all controllers
#ifndef LED_MAPPER_NO_GUI
        if (m_toggleDebugController->getChecked() && currentCtrl < Controllers.size()) {
            Controllers[currentCtrl]->draw();
            // If debug toggled show lines from selected controller and its gui
        }
        else
#endif
        {
            for (auto &ctrl : Controllers)
                ctrl.second->draw();
        }
    }
}

void ofxLedMapper::update(const ofPixels &grabImg)
{
#ifndef LED_MAPPER_NO_GUI
    if (m_toggleDebugController->getChecked()) {
        Controllers[currentCtrl]->sendUdp(grabImg);
        return;
    }
    if (m_togglePlay->getChecked())
#endif
    {
        for (auto &ctrl : Controllers) {
            ctrl.second->sendUdp(grabImg);
        }
    }
}

bool ofxLedMapper::add(string folder_path)
{
    add(Controllers.size(), folder_path);
    return true;
}

bool ofxLedMapper::add(unsigned int _ctrlId, string folder_path)
{
    unsigned int ctrlId = _ctrlId;
    while (!checkUniqueId(ctrlId)) {
        ctrlId++;
    }
    auto ctrl = make_unique<ofxLedController>(ctrlId, folder_path);

#ifndef LED_MAPPER_NO_GUI
    function<void(void)> fnc = [this](void) { this->updateControllersListGui(); };
    ctrl->setOnControllerStatusChange(fnc);
    ctrl->setGuiPosition(m_listControllers->getX(),
                         m_listControllers->getY() + m_listControllers->getHeight());
    m_listControllers->add(ofToString(ctrlId));
#endif

    Controllers[ctrlId] = move(ctrl);

    return true;
}

bool ofxLedMapper::remove(unsigned int _ctrlId) { return true; }

// Return true if ID not found
bool ofxLedMapper::checkUniqueId(unsigned int _ctrlId)
{
    if (Controllers.empty())
        return true;
    return Controllers.find(_ctrlId) == Controllers.end();
}

//
// ------------------------------ GUI ------------------------------
//
void ofxLedMapper::setupGui()
{
#ifndef LED_MAPPER_NO_GUI
    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    m_gui->setTheme(guiTheme.get());
    m_gui->setWidth(LM_GUI_WIDTH);

    m_gui->addHeader(LMGUIListControllers);
    currentCtrl = 0;

    m_togglePlay = m_gui->addToggle(LMGUITogglePlay, true);
    m_togglePlay->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    m_fpsSlider = m_gui->addSlider(LMGUISliderFps, 10, 60);
    m_fpsSlider->onSliderEvent(this, &ofxLedMapper::onSliderEvent);

    m_toggleDebugController = m_gui->addToggle(LMGUIToggleDebug, false);
    m_toggleDebugController->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    auto btn = m_gui->addButton(LMGUIButtonAdd);
    btn->onButtonEvent(this, &ofxLedMapper::onButtonClick);

    m_listControllers = make_unique<ofxDatGuiScrollView>(LMGUIListControllers, 5);
    m_listControllers->onScrollViewEvent(this, &ofxLedMapper::onScrollViewEvent);
    m_listControllers->setWidth(LM_GUI_WIDTH);
    m_listControllers->setNumVisible(10);
    m_listControllers->setPosition(m_gui->getPosition().x,
                                   m_gui->getPosition().y + m_gui->getHeight());
    m_listControllers->setBackgroundColor(ofColor(10));
#endif
}

void ofxLedMapper::setGuiPosition(int x, int y)
{
#ifndef LED_MAPPER_NO_GUI
    m_gui->setPosition(x, y);
    m_listControllers->setPosition(x, y + m_gui->getHeight());
    m_listControllers->update();
    if (currentCtrl < Controllers.size())
        Controllers[currentCtrl]->setGuiPosition(m_listControllers->getX() - LM_GUI_WIDTH,
                                                 m_listControllers->getY());
#endif
}

void ofxLedMapper::setCurrentController(unsigned int _curCtrl)
{
    if (Controllers.empty())
        return;

    _curCtrl %= Controllers.size();

    if (Controllers.find(_curCtrl) == Controllers.end())
        return;

    currentCtrl = _curCtrl;
    for (auto &ctrl : Controllers)
        ctrl.second->setSelected(false);

    Controllers[currentCtrl]->setSelected(true);
    Controllers[currentCtrl]->setGuiPosition(m_listControllers->getX() - LM_GUI_WIDTH,
                                             m_listControllers->getY());

    updateControllersListGui();
}

void ofxLedMapper::updateControllersListGui()
{
#ifndef LED_MAPPER_NO_GUI
    m_listControllers->sort();
    for (int i = 0; i < m_listControllers->getNumItems(); ++i) {
        m_listControllers->get(ofToString(i))
            ->setBackgroundColor(Controllers[i]->isStatusOk()
                                     ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN_DARK)
                                     : ofColor::fromHex(LedMapper::LM_COLOR_RED_DARK));
    }
    m_listControllers->get(ofToString(currentCtrl))
        ->setBackgroundColor(Controllers[currentCtrl]->isStatusOk()
                                 ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN)
                                 : ofColor::fromHex(LedMapper::LM_COLOR_RED));

#endif
}

//
// ------------------------------ SAVE & LOAD ------------------------------
//
bool ofxLedMapper::load()
{
    dir.open(configFolderPath);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(configFolderPath);
        return false;
    }
    dir.listDir();
    dir.sort();

    /// clear current ctrls
    if (!Controllers.empty()) {
        Controllers.clear();
#ifndef LED_MAPPER_NO_GUI
        m_listControllers->clear();
#endif
    }

    regex ctrl_name(".*" + ofToString(LCFileName) + "([0-9]+).*"); // ([^\\.]+)
    smatch base_match;
    for (int i = 0; i < (int)dir.size(); i++) {
        string pth = dir.getPath(i);
        regex_match(pth, base_match, ctrl_name);
        if (base_match.size() > 1) {
            ofLogVerbose("[ofxLedMapper] Load: add controller " + base_match[1].str());
            add(ofToInt(base_match[1].str()), configFolderPath);
        }
    }
    if (!Controllers.empty()) {
#ifndef LED_MAPPER_NO_GUI
        m_listControllers->sort();
#endif
        setCurrentController(0);
        return true;
    }

    return false;
}

bool ofxLedMapper::save()
{

    dir.open(configFolderPath);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(configFolderPath);
    }
    for (auto &ctrl : Controllers) {
        ctrl.second->save(configFolderPath);
    }

    return true;
}

//
// ------------------------------ EVENTS ------------------------------
//
#ifndef LED_MAPPER_NO_GUI
void ofxLedMapper::onScrollViewEvent(ofxDatGuiScrollViewEvent e)
{
    if (e.parent->getName() == LMGUIListControllers) {
        // check if item from list selected
        setCurrentController(ofToInt(e.target->getName()));
    }
}

void ofxLedMapper::onButtonClick(ofxDatGuiButtonEvent e)
{
    if (e.target->getName() == LMGUIButtonAdd) {
        add(configFolderPath);
    }
}

void ofxLedMapper::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if (e.target->getName() == LMGUISliderFps) {
        ofSetFrameRate(e.target->getValue());
    }
}
#endif

void ofxLedMapper::keyPressed(ofKeyEventArgs &data)
{
    switch (data.key) {
        case OF_KEY_UP:
            setCurrentController(currentCtrl - 1);
            break;
        case OF_KEY_DOWN:
            setCurrentController(currentCtrl + 1);
            break;
    }
}

void ofxLedMapper::keyReleased(ofKeyEventArgs &data) {}

void ofxLedMapper::windowResized(ofResizeEventArgs &args)
{
    //#ifndef LED_MAPPER_NO_GUI
    //    m_listControllers->setPosition(args.width-LM_GUI_WIDTH,
    //    m_gui->getPosition().y+m_gui->getHeight());
    //    if (currentCtrl < Controllers.size())
    //        Controllers[currentCtrl]->setGuiPosition(m_listControllers->getX()-LM_GUI_WIDTH,
    //        m_listControllers->getY());
    //#endif
}
