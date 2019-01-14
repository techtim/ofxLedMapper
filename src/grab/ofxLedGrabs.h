//
// Created by Timofey Tavlintsev on 11/11/2018.
//

#pragma once

#include "ofxLedGrabObject.h"

namespace LedMapper {

static unique_ptr<ofxLedGrab> GetUniqueTypedGrab(const ofxLedGrab *grab)
{
    switch (grab->getType()) {
        case LMGrabType::GRAB_LINE:
            return make_unique<ofxLedGrabLine>(*(dynamic_cast<const ofxLedGrabLine *>(grab)));
        case LMGrabType::GRAB_MATRIX:
            return make_unique<ofxLedGrabMatrix>(*(dynamic_cast<const ofxLedGrabMatrix *>(grab)));
        case LMGrabType::GRAB_CIRCLE:
            return make_unique<ofxLedGrabCircle>(*(dynamic_cast<const ofxLedGrabCircle *>(grab)));
        default:
            break;
    }
    assert(false);
    return nullptr;
}

} // namespace LedMapper