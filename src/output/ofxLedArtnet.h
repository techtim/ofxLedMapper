//
// Created by Timofey Tavlintsev on 29/10/2018.
//
#pragma once

#include "Common.h"
#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxNetwork.h"

namespace LedMapper {

class ofxLedArtnet {
    bool m_bSetup;
    string m_ip;
    ofxUDPManager m_frameConnection;

public:
    ofxLedArtnet();

    void setup(const string ip);

    bool send(const ofPixels &data) const;

    void bindGui(ofxDatGui *gui);

    size_t getNumChannels() const;
};

} // namespace LedMapper