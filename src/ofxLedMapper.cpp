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

namespace LedMapper {

ofxLedMapper::ofxLedMapper()
    : m_bSetup(false)

#ifndef LED_MAPPER_NO_GUI
    , m_gui(nullptr)
    , m_guiController(nullptr)
#endif

#ifdef WIN32
    , m_configFolderPath(LedMapper::CONFIG_PATH + LedMapper::APP_NAME + "\\")
#elif defined(__APPLE__)
    , m_configFolderPath(LedMapper::CONFIG_PATH + LedMapper::APP_NAME + "/")
#endif

{
    setupGui();

    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);

    /// add default ctrl
    add(0, m_configFolderPath);
    setCurrentController(0);

    m_bSetup = true;
}

ofxLedMapper::~ofxLedMapper()
{
    ofLogVerbose("[ofxLedMapper] Detor: remove event listeners");

    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofRemoveListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
}

void ofxLedMapper::draw()
{
    if (!m_bSetup)
        return;

    if (m_controllers.empty())
        return;

// If no debug toggled show lines from all m_controllers
#ifndef LED_MAPPER_NO_GUI
    if (m_toggleDebugController->getChecked() && m_currentCtrl < m_controllers.size()) {
        // If debug toggled show lines from selected controller and its gui
        m_controllers[m_currentCtrl]->draw();
    }
    else
#endif
    {
        for (auto &ctrl : m_controllers)
            ctrl.second->draw();
    }
}

void ofxLedMapper::drawGui() {
#ifndef LED_MAPPER_NO_GUI
        m_gui->update();
        m_gui->draw();
        m_listControllers->update();
        m_listControllers->draw();
        if (m_guiController) {
            m_guiController->update();
            m_guiController->draw();
        }
#endif
}
    
void ofxLedMapper::update(const ofPixels &grabImg)
{
#ifndef LED_MAPPER_NO_GUI
    /// Send only to selected controller when Debug Toggle enabled
    if (m_toggleDebugController->getChecked()) {
        m_controllers[m_currentCtrl]->sendUdp(grabImg);
        return;
    }
    if (m_togglePlay->getChecked())
#endif
    {
        for (auto &ctrl : m_controllers) {
            ctrl.second->sendUdp(grabImg);
        }
    }
}

bool ofxLedMapper::add(string folder_path)
{
    add(m_controllers.size(), folder_path);
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
    m_listControllers->add(ofToString(ctrlId));
#endif

    m_controllers[ctrlId] = move(ctrl);

    return true;
}

bool ofxLedMapper::remove(unsigned int _ctrlId)
{
    auto it = m_controllers.find(_ctrlId);
    if (it == m_controllers.end()) {
        ofLogError() << "[ofxLedMapper] no ctrl to remove with id=" << _ctrlId;
        return false;
    }
    ofLogNotice() << "[ofxLedMapper] remove ctrl with id=" << _ctrlId;
    m_controllers.erase(it);

#ifndef LED_MAPPER_NO_GUI
    m_listControllers->remove(m_listControllers->get(ofToString(_ctrlId)));
    m_guiController.reset();
#endif

    return true;
}

// Return true if ID not found
bool ofxLedMapper::checkUniqueId(unsigned int _ctrlId)
{
    if (m_controllers.empty())
        return true;
    return m_controllers.find(_ctrlId) == m_controllers.end();
}

//
// ------------------------------ GUI ------------------------------
//
void ofxLedMapper::setupGui()
{
#ifndef LED_MAPPER_NO_GUI
    m_guiController = ofxLedController::GenerateGui();
    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    m_gui->setTheme(guiTheme.get());
    m_gui->setWidth(LM_GUI_WIDTH);

    m_gui->addHeader(LMGUIListControllers);
    m_currentCtrl = 0;

    auto button = m_gui->addButton(LMGUIButtonSave);
    button->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    button->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    button = m_gui->addButton(LMGUIButtonLoad);
    button->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    button->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    m_togglePlay = m_gui->addToggle(LMGUITogglePlay, true);
    m_togglePlay->onButtonEvent(this, &ofxLedMapper::onButtonClick);

    m_toggleDebugController = m_gui->addToggle(LMGUIToggleDebug, false);
    m_toggleDebugController->onButtonEvent(this, &ofxLedMapper::onButtonClick);

    m_gui->addButton(LMGUIButtonAdd)->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    m_gui->addButton(LMGUIButtonDel)->onButtonEvent(this, &ofxLedMapper::onButtonClick);

    m_listControllers = make_unique<ofxDatGuiScrollView>(LMGUIListControllers, 5);
    m_listControllers->setTheme(guiTheme.get());
    m_listControllers->onScrollViewEvent(this, &ofxLedMapper::onScrollViewEvent);
    m_listControllers->setWidth(LM_GUI_WIDTH);
    m_listControllers->setNumVisible(10);
    m_listControllers->setBackgroundColor(ofColor(10));

    m_gui->update();

    setGuiPosition(m_gui->getPosition().x, m_gui->getPosition().y + m_gui->getHeight());
#endif
}

void ofxLedMapper::setGuiPosition(int x, int y)
{
#ifndef LED_MAPPER_NO_GUI
    m_gui->setPosition(x, y);
    m_listControllers->setPosition(x, y + m_gui->getHeight());
    m_listControllers->update();
    if (m_guiController)
        m_guiController->setPosition(m_listControllers->getX() - LM_GUI_WIDTH,
                                     m_listControllers->getY());
#endif
}

void ofxLedMapper::setCurrentController(unsigned int _curCtrl)
{
    if (m_controllers.empty())
        return;

    if (m_controllers.find(_curCtrl) == m_controllers.end()) {
        return;
    }

    m_currentCtrl = _curCtrl;
    for (auto &ctrl : m_controllers)
        ctrl.second->setSelected(false);

    m_controllers[m_currentCtrl]->setSelected(true);

#ifndef LED_MAPPER_NO_GUI
    if (!m_guiController) {
        m_guiController = ofxLedController::GenerateGui();
        m_guiController->setPosition(m_listControllers->getX() - LM_GUI_WIDTH,
                                     m_listControllers->getY());
    }
    m_controllers[m_currentCtrl]->bindGui(m_guiController.get());
#endif

    updateControllersListGui();
}

void ofxLedMapper::updateControllersListGui()
{
#ifndef LED_MAPPER_NO_GUI
    m_listControllers->sort();
    for (int i = 0; i < m_listControllers->getNumItems(); ++i) {
        auto child = m_listControllers->get(i);
        child->setBackgroundColor(m_controllers[ofToInt(child->getName())]->isStatusOk()
                                      ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN_DARK)
                                      : ofColor::fromHex(LedMapper::LM_COLOR_RED_DARK));
    }
    m_listControllers->get(ofToString(m_currentCtrl))
        ->setBackgroundColor(m_controllers[m_currentCtrl]->isStatusOk()
                                 ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN)
                                 : ofColor::fromHex(LedMapper::LM_COLOR_RED));

#endif
}

//
// ------------------------------ SAVE & LOAD ------------------------------
//
bool ofxLedMapper::load()
{
    dir.open(m_configFolderPath);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(m_configFolderPath);
        return false;
    }
    dir.listDir();
    dir.sort();

    /// clear current ctrls
    if (!m_controllers.empty()) {
        m_controllers.clear();
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
            add(ofToInt(base_match[1].str()), m_configFolderPath);
        }
    }
    if (!m_controllers.empty()) {
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

    dir.open(m_configFolderPath);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(m_configFolderPath);
    }
    for (auto &ctrl : m_controllers) {
        ctrl.second->save(m_configFolderPath);
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
    if (e.target->getName() == LMGUIButtonSave) {
        save();
    }

    if (e.target->getName() == LMGUIButtonLoad) {
        load();
    }

    if (e.target->getName() == LMGUIButtonAdd) {
        add(m_configFolderPath);
    }

    if (e.target->getName() == LMGUIButtonDel) {
        remove(m_currentCtrl);
    }
}

void ofxLedMapper::onSliderEvent(ofxDatGuiSliderEvent e) {}
#endif

void ofxLedMapper::keyPressed(ofKeyEventArgs &data)
{
    switch (data.key) {
        case OF_KEY_UP:
            setCurrentController(m_currentCtrl - 1);
            break;
        case OF_KEY_DOWN:
            setCurrentController(m_currentCtrl + 1);
            break;
    }
}

void ofxLedMapper::keyReleased(ofKeyEventArgs &data) {}

void ofxLedMapper::windowResized(ofResizeEventArgs &args)
{
    //#ifndef LED_MAPPER_NO_GUI
    //    m_listControllers->setPosition(args.width-LM_GUI_WIDTH,
    //    m_gui->getPosition().y+m_gui->getHeight());
    //#endif
}

} // namespace LedMapper