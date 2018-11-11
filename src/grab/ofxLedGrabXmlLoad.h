//
// Created by Timofey Tavlintsev on 11/11/2018.
//

#pragma once

#include "Common.h"
#include "grab/ofxLedGrabs.h"
#include "ofMain.h"
#include "ofxXmlSettings.h"

namespace LedMapper {

/// Deprecated load from XML
static bool ParseXmlToGrabObjects(string xmlPath,
                                  vector<vector<unique_ptr<ofxLedGrab>>> &channelGrabObjects,
                                  ofJson &config)
{
    ofxXmlSettings XML;
    if (!XML.loadFile(xmlPath)) {
        ofLogError() << "[ofxLedController] No config with path=" << xmlPath;
        return false;
    }

    ofLogNotice("[ofxLedController] Load file=" + xmlPath);

    XML.pushTag("CONF", 0);

    config["colorType"] = s_grabColorTypes[XML.getValue("colorType", 0, 0)];
    config["pixInLed"] = XML.getValue("PixInLed", 2.f, 0);
    config["fps"] = XML.getValue("fps", 25, 0);
    config["bSend"] = XML.getValue("UdpSend", false, 0) ? true : false;

    config["ipAddress"] = XML.getValue("IpAddress", RPI_IP, 0);
    config["port"] = XML.getValue("port", RPI_PORT, 0);
    config["ledType"] = XML.getValue("LedType", "", 0);

    XML.popTag();

    int numDragTags = XML.getNumTags("STROKE:LN");

    if (numDragTags == 0)
        return true;

    ofLogVerbose("[ofxLedController] Load lines from XML");
    // we push into the last STROKE tag this temporarirly treats the tag as the document root.
    XML.pushTag("STROKE", numDragTags - 1);

    // we see how many points we have stored in <LN> tags
    int numPtTags = XML.getNumTags("LN");

    if (numPtTags > 0) {
        for (int i = 0; i < numPtTags; i++) {
            // the last argument of getValue can be used to specify
            // which tag out of multiple tags you are refering to.
            unique_ptr<ofxLedGrab> tmpObj{ nullptr };
            if (XML.getValue("LN:TYPE", LMGrabType::GRAB_EMPTY, i) == LMGrabType::GRAB_LINE) {
                tmpObj = make_unique<ofxLedGrabLine>(
                    ofVec2f(XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i)),
                    ofVec2f(XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)));
                tmpObj->load(XML, i);
            }
            else if (XML.getValue("LN:TYPE", LMGrabType::GRAB_EMPTY, i)
                     == LMGrabType::GRAB_CIRCLE) {
                tmpObj = make_unique<ofxLedGrabCircle>(
                    ofVec2f(XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i)),
                    ofVec2f(XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)));
                tmpObj->load(XML, i);
            }
            else if (XML.getValue("LN:TYPE", LMGrabType::GRAB_EMPTY, i)
                     == LMGrabType::GRAB_MATRIX) {
                tmpObj = make_unique<ofxLedGrabMatrix>(
                    ofVec2f(XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i)),
                    ofVec2f(XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)));
                tmpObj->load(XML, i);
            }

            if (tmpObj == nullptr)
                ofLogError() << "ofxLedController Malformed XML config, LN:TYPE unknown or empty"
                             << XML.getValue("LN:TYPE", 0, i);

            int chan = XML.getValue("LN:CHANNEL", 0, i);
            tmpObj->setObjectId(channelGrabObjects[chan].size());
            tmpObj->setChannel(chan);
            channelGrabObjects[chan].emplace_back(move(tmpObj));
        }
    }
    XML.popTag();

    return true;
}

} // namespace LedMapper