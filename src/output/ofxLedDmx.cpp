//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#include "ofxLedDmx.h"

void ofxLedDmx::setup(const string &serialName)
{
#ifdef USE_DMX_FTDI
    else if (dmxFtdi.open()) { ofLog(OF_LOG_NOTICE, "******** DMX FDI SETUP ********"); }
#elif USE_DMX
    if (serialName == "" && dmx.connect(0, 512)) {
        ofLog(OF_LOG_NOTICE, "******** DMX PRO Default SETUP ********");
    }
    else if (dmx.connect(serialName, 512)) { // DMX_MODULES * DMX_CHANNELS);
        dmx.update(true); // black on startup
        ofLog(OF_LOG_NOTICE, "******** DMX PRO " + serialName + " SETUP ********");
    }
#endif // DMX
}

void ofxLedDmx::send(const ofPixels &grabbedImg)
{
    if (!bDmxSend)
        return;

    updatePixels(grabbedImg);

#ifdef USE_DMX
    if (dmx.isConnected()) {
        int cntr = getTotalLeds() * 3;
        for (int i = 1; i < 513; i++) {
            if (i > 512)
                return;
            dmxFtdiVal[i] = i > cntr ? (char)0 : (char)m_output[i - 1];
            dmx.setLevel(i, dmxFtdiVal[i]);
        }
        dmx.update();
    }
    else {
        ofLogVerbose("NO USB->DMX");
    }
#elif USE_DMX_FTDI
    if (dmxFtdi.isOpen()) {
        int cntr = getTotalLeds() * 3;
        for (int i = 1; i < 513; i++) {
            if (i > 512)
                return;
            dmxFtdiVal[i] = i > cntr ? (char)0 : (char)m_output[i - 1];
        }
        dmxFtdi.writeDmx(dmxFtdiVal, 513);
    }
    else {
        ofLogVerbose("NO USB->DMX");
    }
#endif // DMX
}