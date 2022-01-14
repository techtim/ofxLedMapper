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
    size_t m_universesInChannel;
    size_t m_startUniverse;
    ofxUDPManager m_frameConnection;
    uint8_t m_seqNumber;

public:
    ofxLedArtnet();

    void setup(const string ip);
    void resetup() { setup(m_ip); }
    bool send(ChannelsToPix &&output);
    bool sendUniverse(vector<char> &pixels, size_t offset, size_t universe);

    void bindGui(ofxDatGui *gui);

    vector<string> getChannels() noexcept;
    static size_t getMaxPixelsOut() noexcept;

    void saveJson(ofJson &config) const;
    void loadJson(const ofJson &config);
};

} // namespace LedMapper