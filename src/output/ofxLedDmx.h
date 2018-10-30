//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#pragma once

class ofxLedDmx {
    ofxLedDmx(){};
    void setup(const string &serialName);
    void send(const ofPixels &grabbedImg);
    void bindGui(ofxDatGui *gui);
};
