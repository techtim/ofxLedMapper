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
    gui.setup("ledMapper"+ofToString(_id)); // most of the time you don't need a name but don't forget to call setup
    //    gui.setName("control");
    gui.setSize(255, 400);
    currentCtrl == 0;
    gui.add(bAdd.set("Add", false));
    gui.add(bShowControllers.set("Show Controllers", true));
    gui.setPosition(ofGetWidth()-255, 0);

    ofAddListener(ofEvents().keyPressed, this, &ofxLedMapper::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedMapper::keyReleased);
    
    bSetup=true;
}


ofxLedMapper::~ofxLedMapper() {
    Controllers.clear();
}

void ofxLedMapper::draw() {
    if (bAdd) {
        bAdd = false;
        add();
    }

    gui.draw();
    int cntr;
    
    if (Controllers.size()>0) {
        if (bShowControllers) {
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

void ofxLedMapper::update(const ofPixels &grabImg){
     for (auto &ctrl : Controllers ) {
         ctrl->updatePixels(grabImg);
     }
}

bool ofxLedMapper::add() {
    string folder_path = "Ctrls"+ofToString(_id);
    ofxLedController * ledCtrl = new ofxLedController();
//    ofxLedController_ptr ledCtrl(new ofxLedController());
    ledCtrl->setup(Controllers.size(), folder_path, ofRectangle(0, 0, ofGetWidth(), ofGetHeight()));
    //            ledCtrl->setupUdp(RPI_HOST+ofToString(1), RPI_PORT);
    //            ledCtrl->load();
    Controllers.push_back(ledCtrl);
    return true;
}

void ofxLedMapper::notifyParameterChanged(ofAbstractParameter & param){
//    if (param.getName() == "DMX Chan"){
//        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++)
//            (*i)->setPixelsInLed(pixelsInLed);
//    }
    if (param.getName() == "Add"){
        this->add();
    }
    if (param.getName() == "Show Controllers"){
        
    }
}

bool ofxLedMapper::load() {
    string folder_path = "Ctrls"+ofToString(_id);
    dir.listDir(folder_path);
    dir.sort();
    Controllers.clear();
    for(int i = 0; i < (int)dir.size(); i++){
        string pth = dir.getPath(i);
        if (dir.getPath(i).find("ofxLedController") != string::npos) {
//            ofxLedController_ptr ledCtrl(new ofxLedController());
            ofxLedController * ledCtrl =new ofxLedController();
            ledCtrl->setup(Controllers.size(), folder_path, ofRectangle(0, 0, ofGetWidth(), ofGetHeight()));
//            ledCtrl->setupUdp(RPI_HOST+ofToString(1), RPI_PORT);
//            ledCtrl->load();
            
            Controllers.push_back(ledCtrl);
        }
    }
    gui.loadFromFile(folder_path+"/ofxLedMapper"+ofToString(_id)+".xml");

    return Controllers.size()>0 ? true : false;
}

bool ofxLedMapper::save() {
    string folder_path = "Ctrls"+ofToString(_id);
    gui.saveToFile(folder_path+"/ofxLedMapper"+ofToString(_id)+".xml");
    for (auto &ctrl : Controllers ) {
        ctrl->save(folder_path);
    }

    return true;
}

void ofxLedMapper::keyPressed(ofKeyEventArgs& data) {
    switch (data.key) {
        case OF_KEY_LEFT:
                currentCtrl--;
            break;
        case OF_KEY_RIGHT:
            currentCtrl++;
            if (currentCtrl>=Controllers.size()) currentCtrl=Controllers.size()-1;
            break;
    }
}
void ofxLedMapper::keyReleased(ofKeyEventArgs& data) {
    
}