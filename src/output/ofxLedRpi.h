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

#pragma once

#include "ofMain.h"
#include "ofxDatGui.h"
#include "ofxNetwork.h"
#include "Common.h"

namespace LedMapper {

class ofxLedRpi {
    bool m_bSetup;
    string m_ip, m_currentLedType;
    int m_port;
    ofxUDPManager m_frameConnection, m_confConnection;
    vector<char> m_output;

public:
    ofxLedRpi();
    ~ofxLedRpi();
    void setup(const string ip, const int port = RPI_PORT);
    void resetup();
    void bindGui(ofxDatGui *gui);

    bool send(ChannelsToPix &&output);
    void sendLedType(const string &ledType);

    vector<string> getChannels() const noexcept;

    string getIP() const noexcept { return m_ip; }
    int getPort() const noexcept { return m_port; }
    string getLedType() const noexcept { return m_currentLedType; }
};

} // namespace LedMapper