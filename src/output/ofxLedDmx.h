//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#pragma once
#include "ofMain.h"
#include "ofxDatGui.h"

class ofxLedDmx {
/*
#ifdef USE_DMX_FTDI
    ofxDmxFtdi dmxFtdi;
    unsigned char dmxFtdiVal[513];
#elif
    ofxDmx dmx;
#endif
*/
public:
    ofxLedDmx(){};
    void setup(const string &serialName);
    void send(const ofPixels &grabbedImg);
    void bindGui(ofxDatGui *gui);
};
