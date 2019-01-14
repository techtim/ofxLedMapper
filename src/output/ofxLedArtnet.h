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
    ofxUDPManager m_frameConnection;

public:
    ofxLedArtnet();
    ~ofxLedArtnet();

    void setup(const string ip);
    bool resetup();
    bool send(ChannelsToPix &&output);
    bool sendUniverse(vector<char> &pixels, size_t offset, size_t universe);

    void bindGui(ofxDatGui *gui);

    vector<string> getChannels() noexcept;
    size_t getMaxPixelsOut() noexcept;
    string getIP() const noexcept { return m_ip; }

    void saveJson(ofJson &config) const;
    void loadJson(const ofJson &config);
};

} // namespace LedMapper