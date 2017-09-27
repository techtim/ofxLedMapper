//
//  ofxLedMapper.cpp
//  GipsyTree
//
//  Created by Tim TVL on 25/12/15.
//
//

#include "ofxLedMapper.h"
#include <regex>
#include <math.h>

ofxLedMapper::ofxLedMapper() {
    ofxLedMapper(1);
}

ofxLedMapper::ofxLedMapper(int __id)
: _id(__id)
, configFolderPath(LedMapper::CONFIG_PATH+LedMapper::APP_NAME+"/")
{
    setupGui();
    
    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
    
    bSetup=true;
}

ofxLedMapper::~ofxLedMapper() {
    ofLogVerbose("[ofxLedMapper] Detor: clear controllers + remove event listeners");
    Controllers.clear();

#ifndef LED_MAPPER_NO_GUI
    listControllers->clear();
    gui->clear();
    delete gui;
    delete listControllers;
#endif
    
    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofRemoveListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
}

void ofxLedMapper::draw() {
    
#ifndef LED_MAPPER_NO_GUI
    gui->update();
    gui->draw();
    listControllers->update();
    listControllers->draw();
#endif
    if (!Controllers.empty()) {
        // If no debug toggled show lines from all controllers
#ifndef LED_MAPPER_NO_GUI
        if (toggleDebugController->getChecked() && currentCtrl<Controllers.size()) {
            Controllers[currentCtrl]->draw();
            Controllers[currentCtrl]->sendUdp();
        // If debug toggled show lines from selected controller and its gui
        } else
#endif
        {
            for (auto &ctrl : Controllers ) {
                ctrl->draw();
#ifndef LED_MAPPER_NO_GUI
                if (togglePlay->getChecked())
#endif
                    ctrl->sendUdp();
            }
        }
    }
}

void ofxLedMapper::update(const ofPixels &grabImg) {
     for (auto &ctrl : Controllers ) {
         ctrl->updatePixels(grabImg);
     }
}

bool ofxLedMapper::add(string folder_path) {
    add(Controllers.size(), folder_path);
    return true;
}

bool ofxLedMapper::add(unsigned int _ctrlId, string folder_path) {
    unsigned int ctrlId = _ctrlId;
    while (!checkUniqueId(ctrlId)) {
        ctrlId++;
    }
    auto ctrl = make_unique<ofxLedController>(ctrlId, folder_path);
    
#ifndef LED_MAPPER_NO_GUI
    function<void(void)> fnc = [this](void){this->updateControllersListGui();};
    ctrl->setOnControllerStatusChange(fnc);
    ctrl->setGuiPosition(listControllers->getX(), listControllers->getY()+listControllers->getHeight());
    listControllers->add(ofToString(ctrlId));
#endif
    
    Controllers.emplace_back(move(ctrl));
    
    return true;
}

bool ofxLedMapper::remove(unsigned int _ctrlId) {
    return true;
}

// Return true if ID not found
bool ofxLedMapper::checkUniqueId(unsigned int _ctrlId) {
    if (Controllers.empty()) return true;
    auto iter = std::find_if(Controllers.begin(),
                             Controllers.end(),
                             [&](unique_ptr<ofxLedController> const& ctrl) {
                                 return ctrl->getId() == _ctrlId;
                             }
                             );
    return iter == Controllers.end();
}

//
// ------------------------------ GUI ------------------------------
//
void ofxLedMapper::setupGui() {
#ifndef LED_MAPPER_NO_GUI
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
    gui->setTheme(new LedMapper::ofxDatGuiThemeLedMapper());
    gui->setWidth(LM_GUI_WIDTH);

    gui->addHeader(LMGUIListControllers);
    currentCtrl = 0;
    
    togglePlay = gui->addToggle(LMGUITogglePlay, true);
    togglePlay->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    fpsSlider = gui->addSlider(LMGUISliderFps, 10, 60);
    fpsSlider->onSliderEvent(this, &ofxLedMapper::onSliderEvent);

    toggleDebugController = gui->addToggle(LMGUIToggleDebug, false);
    toggleDebugController->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    auto btn = gui->addButton(LMGUIButtonAdd);
    btn->onButtonEvent(this, &ofxLedMapper::onButtonClick);

    listControllers = new ofxDatGuiScrollView(LMGUIListControllers, 5);
    listControllers->onScrollViewEvent(this, &ofxLedMapper::onScrollViewEvent);
    listControllers->setWidth(LM_GUI_WIDTH);
    listControllers->setPosition(gui->getPosition().x, gui->getPosition().y+gui->getHeight());
    listControllers->setBackgroundColor(ofColor(10));
#endif
}

void ofxLedMapper::setGuiPosition(int x, int y) {
#ifndef LED_MAPPER_NO_GUI
    gui->setPosition(x, y);
    listControllers->setPosition(gui->getPosition().x, gui->getPosition().y+gui->getHeight());
#endif
}

void ofxLedMapper::setCurrentController(unsigned int _curCtrl) {
    if (Controllers.empty()) return;

    currentCtrl = _curCtrl;
    if (currentCtrl>=Controllers.size()) currentCtrl=Controllers.size()-1;

    for (auto &ctrl:Controllers) ctrl->setSelected(false);
    Controllers[currentCtrl]->setSelected(true);

    updateControllersListGui();
}

void ofxLedMapper::updateControllersListGui() {
#ifndef LED_MAPPER_NO_GUI
    for (int i=0; i<listControllers->getNumItems(); ++i){
        listControllers->get(i)->setBackgroundColor(Controllers[i]->isStatusOk()
                                                    ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN_DARK)
                                                    : ofColor::fromHex(LedMapper::LM_COLOR_RED_DARK));
    }
    listControllers->get(currentCtrl)->setBackgroundColor(Controllers[currentCtrl]->isStatusOk()
                                                          ? ofColor::fromHex(LedMapper::LM_COLOR_GREEN)
                                                          : ofColor::fromHex(LedMapper::LM_COLOR_RED));
    
#endif

}

//
// ------------------------------ SAVE & LOAD ------------------------------
//
bool ofxLedMapper::load() {
    string folder_path = LedMapper::CONFIG_PATH+LedMapper::APP_NAME+"/";
    dir.open(folder_path);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(folder_path);
        return false;
    }
    dir.sort();

    if (!Controllers.empty()) {
        Controllers.clear();
#ifndef LED_MAPPER_NO_GUI
        listControllers->clear();
#endif
    }
    
    regex ctrl_name(".*"+ofToString(LCFileName)+"([0-9]+).*"); // ([^\\.]+)

    smatch base_match;
    for(int i = 0; i < (int)dir.size(); i++){
        string pth = dir.getPath(i);

        regex_match(dir.getPath(i), base_match, ctrl_name);
        if (base_match.size() > 1) {
            ofLogVerbose("[ofxLedMapper] Load: add controller "+ base_match[1].str());
            add(ofToInt(base_match[1].str()), folder_path);

        }
    }
    if (!Controllers.empty()) {
        setCurrentController(0);
        return true;
    } else {
        return false;
    }
}

bool ofxLedMapper::save() {
//    string folder_path = LMCtrlsFolderPath+ofToString(_id);
//    string folder_path = LedMapper::CONFIG_PATH+LedMapper::APP_NAME+"/";
    dir.open(configFolderPath);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(configFolderPath);
    }
    for (auto &ctrl : Controllers ) {
        ctrl->save(configFolderPath);
    }

    return true;
}

//
// ------------------------------ EVENTS ------------------------------
//
#ifndef LED_MAPPER_NO_GUI
void ofxLedMapper::onScrollViewEvent(ofxDatGuiScrollViewEvent e) {
    if (e.parent->getName() == LMGUIListControllers) {
        // check if item from list selected
        setCurrentController(ofToInt(e.target->getName()));
    }
}

void ofxLedMapper::onButtonClick(ofxDatGuiButtonEvent e) {
    if (e.target->getName() == LMGUIButtonAdd) {
        add(configFolderPath);
    }
}

void ofxLedMapper::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target->getName() == LMGUISliderFps) {
        ofSetFrameRate(e.target->getValue());
    }
}
#endif

void ofxLedMapper::keyPressed(ofKeyEventArgs& data) {
    switch (data.key) {
        case OF_KEY_UP:
            setCurrentController(currentCtrl-1);
            break;
        case OF_KEY_DOWN:
            setCurrentController(currentCtrl+1);
            break;
    }
}

void ofxLedMapper::keyReleased(ofKeyEventArgs& data) {
    
}

void ofxLedMapper::windowResized(ofResizeEventArgs &args) {
#ifndef LED_MAPPER_NO_GUI
    listControllers->setPosition(args.width-LM_GUI_WIDTH, gui->getPosition().y+gui->getHeight());
    Controllers[currentCtrl]->setGuiPosition(args.width-LM_GUI_WIDTH, listControllers->getY()+listControllers->getHeight());
#endif
}


