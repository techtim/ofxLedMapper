//
//  ofxLedMapper.cpp
//  GipsyTree
//
//  Created by Tim TVL on 25/12/15.
//
//

#include "ofxLedMapper.h"

ofxLedMapper::ofxLedMapper() {
    ofxLedMapper::ofxLedMapper(1);
}


ofxLedMapper::ofxLedMapper(int __id) {
    _id = __id;
    gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    gui->setWidth(LM_GUI_WIDTH);
    
    gui->addHeader(LMGUIListControllers);
    currentCtrl = 0;
    toggleDebugController = gui->addToggle(LMGUIToggleDebug, false);
    auto btn = gui->addButton(LMGUIButtonAdd);
    btn->onButtonEvent(this, &ofxLedMapper::onButtonClick);
    
    listControllers = make_unique<ofxDatGuiScrollView>(LMGUIListControllers, 5);
    listControllers->onScrollViewEvent(this, &ofxLedMapper::onScrollViewEvent);
    listControllers->setWidth(LM_GUI_WIDTH);
    listControllers->setPosition(gui->getPosition().x, gui->getPosition().y+gui->getHeight());
    listControllers->setBackgroundColor(ofColor(0));

    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    ofAddListener(ofEvents().windowResized, this, &ofxLedMapper::windowResized);

    load();

    bSetup=true;
}


ofxLedMapper::~ofxLedMapper() {
    Controllers.clear();
}

void ofxLedMapper::draw() {

    gui->update();
    gui->draw();
    listControllers->update();
    listControllers->draw();
    if (!Controllers.empty()) {
        if (!toggleDebugController->getChecked()) {
            for (auto &ctrl : Controllers ) {
                ctrl->draw();
                ctrl->sendUdp();
            }
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
    string folder_path = "Ctrls"+ofToString(_id);
    Controllers.push_back(make_unique<ofxLedController>(Controllers.size(), folder_path));
    Controllers[Controllers.size()-1]->setGuiPosition(listControllers->getX(), listControllers->getY()+listControllers->getHeight());
    listControllers->add(ofToString(Controllers.size()-1));
    return true;
}

void ofxLedMapper::onScrollViewEvent(ofxDatGuiScrollViewEvent e) {
    if (e.parent->getName() == LMGUIListControllers) {
        // check if item from list selected
        for (int i=0; i<listControllers->getNumItems(); listControllers->get(i++)->setBackgroundColor(ofColor(0)));
        
        listControllers->get(e.target->getLabel())->setBackgroundColor(ofColor(50));
        currentCtrl = ofToInt(e.target->getLabel());
        for (auto &ctrl:Controllers) ctrl->setSelected(false);
        if (currentCtrl < Controllers.size()) {
            Controllers[currentCtrl]->setSelected(true);
        }
    }
}

void ofxLedMapper::onButtonClick(ofxDatGuiButtonEvent e) {
    if (e.target->getName() == LMGUIButtonAdd) {
        add();
    }
}

bool ofxLedMapper::load() {
    string folder_path = "Ctrls"+ofToString(_id);
    dir.open(folder_path);
    // check if dir exists, if not create dir and return
    if (!dir.exists()) {
        dir.createDirectory(folder_path);
        return false;
    }
    dir.sort();

    Controllers.clear();
    listControllers->clear();
    for(int i = 0; i < (int)dir.size(); i++){
        string pth = dir.getPath(i);
        if (dir.getPath(i).find(LCFileName) != string::npos) {
            add();
        }
    }
//    gui.loadFromFile(folder_path+"/ofxLedMapper"+ofToString(_id)+".xml");

    return Controllers.empty() ? false : true;
}

bool ofxLedMapper::save() {
    string folder_path = "Ctrls"+ofToString(_id);
//    gui.saveToFile(folder_path+"/ofxLedMapper"+ofToString(_id)+".xml");
    for (auto &ctrl : Controllers ) {
        ctrl->save(folder_path);
    }

    return true;
}

void ofxLedMapper::setGuiPosition(int x, int y) {
    gui->setPosition(x, y);
    listControllers->setPosition(gui->getPosition().x, gui->getPosition().y+gui->getHeight());
}

void ofxLedMapper::keyPressed(ofKeyEventArgs& data) {
    switch (data.key) {
        case OF_KEY_LEFT:
            currentCtrl--;
            break;
        case OF_KEY_RIGHT:
            currentCtrl++;
            break;
    }
    if (currentCtrl>=Controllers.size()) currentCtrl=Controllers.size()-1;
}

void ofxLedMapper::keyReleased(ofKeyEventArgs& data) {
    
}

void ofxLedMapper::windowResized(ofResizeEventArgs &args) {
    listControllers->setPosition(args.width-LM_GUI_WIDTH, gui->getPosition().y+gui->getHeight());
    Controllers[currentCtrl]->setGuiPosition(args.width-LM_GUI_WIDTH, listControllers->getY()+listControllers->getHeight());
}

