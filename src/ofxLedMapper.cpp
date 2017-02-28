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
    ofxLedMapper::ofxLedMapper(1);
}

ofxLedMapper::ofxLedMapper(int __id) {
    _id = __id;
    
    setupGui();
    
    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);

    bSetup=true;
}

ofxLedMapper::~ofxLedMapper() {
    ofLogVerbose("[ofxLedMapper] Detor: clear controllers + remove event listeners");
    Controllers.clear();
    listControllers->clear();
    gui->clear();
    delete gui;
    delete listControllers;

    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofRemoveListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);
}

void ofxLedMapper::draw() {

    gui->update();
    gui->draw();
    listControllers->update();
    listControllers->draw();
    fpsSlider->getValue();
    
    if (!Controllers.empty()) {
        // If no debug toggled show lines from all controllers
        if (!toggleDebugController->getChecked()) {
            for (auto &ctrl : Controllers ) {
                ctrl->draw();
                
                if (togglePlay->getChecked())  ctrl->sendUdp();
            }
        // If debug toggled show lines from selected controller and its gui
        } else if (currentCtrl<Controllers.size()) {
            Controllers[currentCtrl]->draw();
            Controllers[currentCtrl]->sendUdp();
        }
    }
}

void ofxLedMapper::update(const ofPixels &grabImg) {
     for (auto &ctrl : Controllers ) {
         ctrl->updatePixels(grabImg);
     }
}

bool ofxLedMapper::add() {
    add(Controllers.size());
    return true;
}

bool ofxLedMapper::add(unsigned int _ctrlId) {
    string folder_path = LMCtrlsFolderPath+ofToString(_id);
    unsigned int ctrlId = _ctrlId;
    while (!checkUniqueId(ctrlId)) {
        ctrlId++;
    }
    Controllers.push_back(make_unique<ofxLedController>(ctrlId, folder_path));
    Controllers[Controllers.size()-1]->setGuiPosition(listControllers->getX(), listControllers->getY()+listControllers->getHeight());
    listControllers->add(ofToString(ctrlId));
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
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
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
    listControllers->setBackgroundColor(ofColor(0));
}

void ofxLedMapper::setGuiPosition(int x, int y) {
    gui->setPosition(x, y);
    listControllers->setPosition(gui->getPosition().x, gui->getPosition().y+gui->getHeight());
}

void ofxLedMapper::setCurrentController(unsigned int _curCtrl) {
    if (Controllers.empty()) return;

    currentCtrl = _curCtrl;
    if (currentCtrl>=Controllers.size()) currentCtrl=Controllers.size()-1;

    for (int i=0; i<listControllers->getNumItems(); listControllers->get(i++)->setBackgroundColor(ofColor(0)));
    
    listControllers->get(currentCtrl)->setBackgroundColor(ofColor(75));
    
    for (auto &ctrl:Controllers) ctrl->setSelected(false);
    Controllers[currentCtrl]->setSelected(true);
}

//
// ------------------------------ SAVE & LOAD ------------------------------
//
bool ofxLedMapper::load() {
    string folder_path = LMCtrlsFolderPath+ofToString(_id);
    dir.open(folder_path);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(folder_path);
        return false;
    }
    dir.sort();

    if (!Controllers.empty()) {
        Controllers.clear();
        listControllers->clear();
    }
    
    regex ctrl_name(".*"+ofToString(LCFileName)+"([0-9]+).*"); // ([^\\.]+)

    smatch base_match;
    for(int i = 0; i < (int)dir.size(); i++){
        string pth = dir.getPath(i);

        regex_match(dir.getPath(i), base_match, ctrl_name);
        if (base_match.size() > 1) {
            ofLogVerbose("[ofxLedMapper] Load: add controller "+ base_match[1].str());
            add(ofToInt(base_match[1].str()));

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
    string folder_path = LMCtrlsFolderPath+ofToString(_id);
    for (auto &ctrl : Controllers ) {
        ctrl->save(folder_path);
    }

    return true;
}

//
// ------------------------------ EVENTS ------------------------------
//
void ofxLedMapper::onScrollViewEvent(ofxDatGuiScrollViewEvent e) {
    if (e.parent->getName() == LMGUIListControllers) {
        // check if item from list selected
        setCurrentController(ofToInt(e.target->getName()));
    }
}

void ofxLedMapper::onButtonClick(ofxDatGuiButtonEvent e) {
    if (e.target->getName() == LMGUIButtonAdd) {
        add();
    }
}

void ofxLedMapper::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target->getName() == LMGUISliderFps) {
        ofSetFrameRate(e.target->getValue());
    }
}

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
    listControllers->setPosition(args.width-LM_GUI_WIDTH, gui->getPosition().y+gui->getHeight());
    Controllers[currentCtrl]->setGuiPosition(args.width-LM_GUI_WIDTH, listControllers->getY()+listControllers->getHeight());
}

