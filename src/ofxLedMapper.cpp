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
    , m_grabTypeSelected(LMGrabType::GRAB_SELECT)
    , m_currentCtrl(0)
#ifndef LED_MAPPER_NO_GUI
    , m_gui(nullptr)
    , m_guiController(nullptr)
    , m_iconsMenu(nullptr)
#endif
    , m_configFolderPath(LedMapper::LM_CONFIG_PATH)
{
    /// Disable all textures be rect
    // ofDisableArbTex();

    setupGui();

    ofAddListener(ofEvents().mousePressed, this, &ofxLedMapper::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedMapper::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedMapper::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);

    /// add default ctrl
    add(0, m_configFolderPath);

    m_bSetup = true;
}

ofxLedMapper::~ofxLedMapper()
{
    ofLogVerbose("[ofxLedMapper] Detor: remove event listeners");

    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofRemoveListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
    ofRemoveListener(ofEvents().mousePressed, this, &ofxLedMapper::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &ofxLedMapper::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this, &ofxLedMapper::mouseDragged);
}

void ofxLedMapper::update()
{
    if (!m_bSetup || !m_gui->getVisible())
        return;

#ifndef LED_MAPPER_NO_GUI
    m_gui->update();
    m_listControllers->update();
    m_iconsMenu->update();
    if (m_guiController != nullptr)
        m_guiController->update();
#endif
}

void ofxLedMapper::draw()
{
    if (!m_bSetup || m_controllers.empty())
        return;

// If no debug toggled show lines from all m_controllers
#ifndef LED_MAPPER_NO_GUI
    if (m_toggleDebugController->getChecked()) {
        // If debug toggled show lines from selected controller and its gui
        m_controllers.at(m_currentCtrl)->draw();
    }
    else
#endif
    {
        for (auto &ctrl : m_controllers)
            ctrl.second->draw();
    }
}

void ofxLedMapper::drawGui()
{
#ifndef LED_MAPPER_NO_GUI
    m_gui->draw();

    m_listControllers->draw();

    m_iconsMenu->draw();
    if (m_guiController != nullptr)
        m_guiController->draw();
#endif
}

void ofxLedMapper::send(const ofTexture &texIn)
{
#ifndef LED_MAPPER_NO_GUI
    /// Send only to selected controller when Debug Toggle enabled
    if (m_toggleDebugController->getChecked()) {
        m_controllers.at(m_currentCtrl)->send(texIn);
        return;
    }
    if (m_togglePlay->getChecked())
#endif
    {
        for (auto &ctrl : m_controllers) {
            ctrl.second->send(texIn);
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
    ctrl->disableEvents();
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

    if (m_currentCtrl == _ctrlId)
        setCurrentController(_ctrlId - 1);

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
    m_gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    m_guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    m_gui->setTheme(m_guiTheme.get());
    m_gui->setWidth(LM_GUI_WIDTH);
    m_gui->setAutoDraw(false);

    m_gui->addHeader(LMGUIListControllers);

    auto button = m_gui->addButton(LMGUIButtonSave);
    button->onButtonEvent([this](ofxDatGuiButtonEvent) { this->save(); });
    button->setLabelAlignment(ofxDatGuiAlignment::CENTER);
    button = m_gui->addButton(LMGUIButtonLoad);
    button->onButtonEvent([this](ofxDatGuiButtonEvent) { this->load(); });
    button->setLabelAlignment(ofxDatGuiAlignment::CENTER);

    m_togglePlay = m_gui->addToggle(LMGUITogglePlay, true);

    m_toggleDebugController = m_gui->addToggle(LMGUIToggleDebug, false);

    m_gui->addButton(LMGUIButtonAdd)->onButtonEvent([this](ofxDatGuiButtonEvent) {
        this->add(m_configFolderPath);
    });
    m_gui->addButton(LMGUIButtonDel)->onButtonEvent([this](ofxDatGuiButtonEvent) {
        this->remove(m_currentCtrl);
    });

    m_listControllers = make_unique<ofxDatGuiScrollView>(LMGUIListControllers, 5);
    m_listControllers->setTheme(m_guiTheme.get());
    m_listControllers->onScrollViewEvent([this](ofxDatGuiScrollViewEvent e) {
        this->setCurrentController(ofToInt(e.target->getName()));
    });

    m_listControllers->setWidth(LM_GUI_WIDTH);
    m_listControllers->setBackgroundColor(ofColor(30));

    m_gui->update();

    /// Mouse Grab style buttons avalable when controllers tab selected
    /// and draw ledMappers gui
    m_iconsMenu = make_unique<ofxDatGui>(ofxDatGuiAnchor::BOTTOM_RIGHT);
    m_iconsMenu->setTheme(m_guiTheme.get());
    m_iconsMenu->setWidth(LM_GUI_ICON_WIDTH);
    m_iconsMenu->setAutoDraw(false);

    m_iconsMenu->addToggleImage(LMGUIMouseSelect, "gui/mouse_select.jpg")
        ->onToggleEvent(
            [this](ofxDatGuiToggleEvent) { m_grabTypeSelected = LMGrabType::GRAB_SELECT; });
    m_iconsMenu->addToggleImage(LMGUIMouseGrabLine, "gui/mouse_grab_line.jpg")
        ->onToggleEvent(
            [this](ofxDatGuiToggleEvent) { m_grabTypeSelected = LMGrabType::GRAB_LINE; });
    m_iconsMenu->addToggleImage(LMGUIMouseGrabMatrix, "gui/mouse_grab_matrix.jpg")
        ->onToggleEvent(
            [this](ofxDatGuiToggleEvent) { m_grabTypeSelected = LMGrabType::GRAB_MATRIX; });
    //    m_iconsMenu->addToggleImage(LMGUIMouseGrabCircle, "gui/mouse_grab_circle.jpg")
    //        ->onToggleEvent(
    //            [this](ofxDatGuiToggleEvent) { m_grabTypeSelected = LMGrabType::GRAB_CIRCLE; });

    m_iconsMenu->update();

    setGuiPosition(m_gui->getPosition().x, m_gui->getPosition().y + m_gui->getHeight());
#endif
}

void ofxLedMapper::setGuiPosition(int x, int y)
{
#ifndef LED_MAPPER_NO_GUI
    m_gui->setPosition(x, y);
    m_listControllers->setPosition(x, y + m_gui->getHeight());
    m_iconsMenu->setPosition(ofxDatGuiAnchor::BOTTOM_RIGHT);
    if (m_guiController)
        m_guiController->setPosition(m_listControllers->getX(),
                                     m_listControllers->getY() + m_listControllers->getHeight());
#endif
}
void ofxLedMapper::setGuiActive(bool active)
{
#ifndef LED_MAPPER_NO_GUI
    m_gui->setVisible(active);
    m_grabTypeSelected = LMGrabType::GRAB_SELECT;
#endif
}

void ofxLedMapper::setCurrentController(unsigned int _curCtrl)
{
    if (m_controllers.empty()) {
        add(m_configFolderPath);
        return;
    }

    if (m_controllers.find(_curCtrl) == m_controllers.end())
        _curCtrl = m_controllers.begin()->first;

    /// hide Controller gui if select already selected
    bool isDoubleSelect
        = m_currentCtrl == _curCtrl && m_controllers.at(m_currentCtrl)->isSelected();

    ofLogVerbose() << "[ofxLedMapper] Set CurrentController from" << m_currentCtrl << " to "
                   << _curCtrl;
    m_currentCtrl = _curCtrl;
    for (auto &ctrl : m_controllers)
        ctrl.second->setSelected(false);

    if (!isDoubleSelect)
        m_controllers.at(m_currentCtrl)->setSelected(true);

#ifndef LED_MAPPER_NO_GUI
    /// reset gui pointer to hide active Controllers gui
    if (isDoubleSelect) {
        m_guiController.reset();
    }
    else {
        if (!m_guiController) {
            m_guiController = GenerateOutputGui();
        }
        m_controllers.at(m_currentCtrl)->bindGui(m_guiController.get());
    }
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
        ->setBackgroundColor(m_controllers.at(m_currentCtrl)->isStatusOk()
                                 ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN)
                                 : ofColor::fromHex(LedMapper::LM_COLOR_RED));

#endif
}

//
// ------------------------------ SAVE & LOAD ------------------------------
//
bool ofxLedMapper::load(string folderPath)
{
    m_configFolderPath = folderPath;
    return load();
}

bool ofxLedMapper::load()
{
    ofLogVerbose() << "[ofxLedMapper] Load from folder=" << m_configFolderPath;
    m_dir.open(m_configFolderPath);
    // check if dir exists, if not create dir and return
    if (!m_dir.exists()) {
        ofLogVerbose() << "[ofxLedMapper] No such folder!";
        m_dir.createDirectory(m_configFolderPath);
        return false;
    }

    m_dir.listDir();

    if (m_dir.size() == 0) {
        ofLogVerbose() << "[ofxLedMapper] Empty folder=" << m_dir.path();
        return false;
    }

    m_dir.sort();

    /// clear current ctrls
    if (!m_controllers.empty()) {
        m_controllers.clear();
#ifndef LED_MAPPER_NO_GUI
        m_listControllers->clear();
#endif
    }

    regex ctrl_name(".*" + ofToString(LCFileName) + "([0-9]+).*"); // ([^\\.]+)
    smatch base_match;
    for (size_t i = 0; i < m_dir.size(); ++i) {
        string pth = m_dir.getPath(i);
        ofLogVerbose() << "[ofxLedMapper] Check file=" << pth;
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
        setCurrentController(0); /// double select to deselect
        return true;
    }

    return false;
}

bool ofxLedMapper::save(string folderPath)
{
    m_configFolderPath = folderPath;
    return save();
}

bool ofxLedMapper::save()
{
    m_dir.open(m_configFolderPath);
    // check if dir exists, create dir when not
    if (!m_dir.exists()) {
        m_dir.createDirectory(m_configFolderPath);
    }
    for (auto &ctrl : m_controllers) {
        ctrl.second->save(m_configFolderPath);
    }

    return true;
}
//
// ------------------------------ COPY / PASTE ------------------------------
//
void ofxLedMapper::copyGrabs()
{
    m_copyPasteGrabs.clear();
    for (const auto &grab : m_controllers.at(m_currentCtrl)->peekCurrentGrabs())
        if (grab->isSelected()) {
            auto newGrab
                = move(ofxLedController::GetUniqueTypedGrab(grab->getType(), grab.operator*()));
            m_copyPasteGrabs.emplace_back(move(newGrab));
        }
    ofLogVerbose() << "Copied controller #" << m_currentCtrl
                   << " grabs, size=" << m_copyPasteGrabs.size();
}

void ofxLedMapper::pasteGrabs()
{
    if (m_copyPasteGrabs.empty())
        return;
    /// deselect currently selected
    m_controllers.at(m_currentCtrl)->setGrabsSelected(false);
    for (auto &grab : m_copyPasteGrabs) {
        grab->set(grab->getFrom() + ofVec2f(10), grab->getTo() + ofVec2f(10));
        ofLogVerbose() << "Pasting Grab: " << *grab;
        m_controllers.at(m_currentCtrl)
            ->addGrab(
                move(ofxLedController::GetUniqueTypedGrab(grab->getType(), grab.operator*())));
    }

    ofLogVerbose() << "Copied controller #" << m_currentCtrl
                   << " grabs, size=" << m_copyPasteGrabs.size();
}

//
// ------------------------------ EVENTS ------------------------------
//

/// mouse and keyboard events

void ofxLedMapper::mousePressed(ofMouseEventArgs &args)
{
    if (args.x > ofGetWidth() - LM_GUI_WIDTH)
        return;
    m_controllers.at(m_currentCtrl)->setGrabType(m_grabTypeSelected);
    m_controllers.at(m_currentCtrl)->mousePressed(args);
}

void ofxLedMapper::mouseDragged(ofMouseEventArgs &args)
{
    if (args.x > ofGetWidth() - LM_GUI_WIDTH)
        return;
    m_controllers.at(m_currentCtrl)->mouseDragged(args);
}
void ofxLedMapper::mouseReleased(ofMouseEventArgs &args)
{
    if (args.x > ofGetWidth() - LM_GUI_WIDTH)
        return;
    if (!args.hasModifier(OF_KEY_SHIFT)) {
        m_controllers.at(m_currentCtrl)->mouseReleased(args);
    }
}

void ofxLedMapper::keyPressed(ofKeyEventArgs &data)
{
    switch (data.key) {
        case OF_KEY_UP:
            setCurrentController(m_currentCtrl - 1);
            break;
        case OF_KEY_DOWN:
            setCurrentController(m_currentCtrl + 1);
            break;
        case OF_KEY_ESC:
            /// selection of the same Controller deselects all
            setCurrentController(m_currentCtrl);
            break;
        case '1':
            m_grabTypeSelected = LMGrabType::GRAB_SELECT;
            break;
        case '2':
            m_grabTypeSelected = LMGrabType::GRAB_LINE;
            break;
        case '3':
            m_grabTypeSelected = LMGrabType::GRAB_MATRIX;
            break;
        case '4':
            // m_grabTypeSelected = LMGrabType::GRAB_CIRCLE;
            break;
        case 'v':
#ifndef WIN32
            if (data.hasModifier(LM_KEY_CONTROL)) /// don't work on win
#endif
            {
                pasteGrabs();
            }
            break;
        case 'c':
#ifndef WIN32
            if (data.hasModifier(LM_KEY_CONTROL)) /// don't work on win
#endif 
			{
				copyGrabs();
			}
            break;
        case OF_KEY_BACKSPACE:
            m_controllers.at(m_currentCtrl)->deleteSelectedGrabs();
            break;
        default:
            break;
    }
}

void ofxLedMapper::keyReleased(ofKeyEventArgs &data) {}

void ofxLedMapper::windowResized(ofResizeEventArgs &args) {}

} // namespace LedMapper
