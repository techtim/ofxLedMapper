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

#include "ofxLedController.h"
#include "grab/ofxLedGrabXmlLoad.h"
#include "ofxLedPixelGrab.h"

namespace LedMapper {

ofxLedController::ofxLedController(int _id, LedOutputType outputType, const string &_path)
    : m_id(_id)
    , m_path(_path)
    , m_bSelected(false)
    , m_bSend(false)
    , m_statusOk(false)
    , m_bDirtyPoints(false)
    , m_colorLine(ofColor(ofRandom(0, 100), ofRandom(50, 200), ofRandom(150, 255)))
    , m_colorActive(ofColor(0, m_colorLine.g, m_colorLine.b, 200))
    , m_colorInactive(ofColor(m_colorLine.r, m_colorLine.g, 0, 200))
    , m_currentGrabType(LMGrabType::GRAB_SELECT)
    , m_grabBounds(0, 0, 100, 100)
    , m_pixelsInLed(5.f)
    , m_fps(25.f)
    , m_totalLeds(0)
    , m_statusChanged(nullptr)
    , m_currentChannelNum(0)
    , m_ledPoints(0)
    , m_selectionRect(0, 0, 0, 0)
{
    m_ledOut = CreateLedOutput(outputType);
    m_channelList = LedOutputGetChannels(m_ledOut);
    auto maxPixelsOut = LedOutputGetMaxPixels(m_ledOut);
    m_maxPixInChannel = maxPixelsOut / m_channelList.size();
    m_channelsTotalLeds.resize(m_channelList.size(), 0);
    m_channelGrabObjects.resize(m_channelList.size());

    setColorType(GRAB_COLOR_TYPE::RGB);
    setFps(m_fps);

    m_fboLeds.allocate(500, ceil(maxPixelsOut / 500.f), GL_RGB);
    m_fboLeds.begin();
    ofClear(0, 0, 0, 255);
    m_fboLeds.end();

    load(m_path);

    setCurrentChannel(m_currentChannelNum);

    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
}

ofxLedController::~ofxLedController()
{
    ofLogVerbose("[ofxLedController] Dtor: clear lines + remove event listeners + remove gui");
    disableEvents();
    m_channelGrabObjects.clear();
}

/// Disable mouse/key events to explicitly call from ofxLedMapper
void ofxLedController::disableEvents()
{
    ofRemoveListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
}

/// callback from ofxLedMapper to notify Controllers List to check color on violated connection
void ofxLedController::setOnControllerStatusChange(function<void(void)> callback)
{
    m_statusChanged = callback;
}

#ifndef LED_MAPPER_NO_GUI
/// Bing generated GUI to controller
void ofxLedController::bindGui(ofxDatGui *gui)
{
    gui->clear();

    gui->addHeader("Controller " + to_string(m_id));

    gui->addToggle(LCGUIButtonSend, m_bSend)->onToggleEvent([this](ofxDatGuiToggleEvent e) {
        m_bSend = e.checked;
        if (m_bSend)
            LedOutputResetup(m_ledOut);
    });

    auto slider = gui->addSlider(LMGUISliderFps, 10, 60);
    slider->setPrecision(0);
    slider->setValue(m_fps);
    slider->onSliderEvent([this](ofxDatGuiSliderEvent e) { this->setFps(e.value); });

    slider = gui->addSlider(LCGUISliderPix, 1, 50);
    slider->setPrecision(1);
    slider->setValue(m_pixelsInLed);
    slider->onSliderEvent([this](ofxDatGuiSliderEvent e) { this->setPixInLed(e.value); });

    auto dropdown = gui->addDropdown(LCGUIDropColorType, s_grabColorTypes);
    dropdown->select(m_colorType);
    dropdown->onDropdownEvent(
        [this](ofxDatGuiDropdownEvent e) { this->setColorType(GetColorType(e.child)); });

    LedOutputBindGui(m_ledOut, gui);

    dropdown = gui->addDropdown(LCGUIDropChannelNum, m_channelList);
    dropdown->select(m_currentChannelNum);
    dropdown->onDropdownEvent(
        [this](ofxDatGuiDropdownEvent e) { this->setCurrentChannel(e.child); });

    unique_ptr<ofxDatGuiTheme> guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    gui->setTheme(guiTheme.get(), true);
}
#endif

/// Draw all controllers grab objects
void ofxLedController::draw()
{
    int chanNum = 0;

    m_bSelected ? ofSetColor(m_colorActive) : ofSetColor(50);

    GLfloat pointSize;
    glGetFloatv(GL_POINT_SIZE, &pointSize);
    glPointSize(m_pixelsInLed / 2);
    m_vboLeds.draw(OF_MESH_FILL);
    glPointSize(pointSize);

    ofColor color;
    for (auto &channelGrabs : m_channelGrabObjects) {
        color = (chanNum == m_currentChannelNum ? m_colorActive : m_colorInactive);
        for (auto &grab : channelGrabs) {
            grab->draw(color);
        }
        ++chanNum;
    }

    if (!m_bSelected)
        return;

    if (!m_selectionRect.isEmpty()) {
        ofSetColor(200, 200, 200, 50);
        ofDrawRectangle(m_selectionRect);
    }

    /// draw grabbed texture
    ofSetColor(255);
    m_fboLeds.draw(0, ofGetHeight() - m_fboLeds.getHeight());
}

/// Send by UDP grab points data updated with grabbedImg
void ofxLedController::send(const ofTexture &texIn)
{
    updateGrabPoints();

    auto now = ofGetSystemTimeMillis();

    if (!m_bSend || now - m_lastFrameTime < m_msecInFrame)
        return;

    m_lastFrameTime = now;

    auto grabbedPixs = updatePixels(texIn);

    bool prevStatus = m_statusOk;

    m_statusOk = LedOutputSend(m_ledOut, move(grabbedPixs));

    if (m_statusOk != prevStatus && m_statusChanged != nullptr)
        m_statusChanged();
}

/// Update grab points from grab objects and put them to VBO
void ofxLedController::updateGrabPoints()
{
    if (!m_bDirtyPoints)
        return;

    m_bDirtyPoints = false;
    m_totalLeds = 0;
    m_ledPoints.clear();

    m_vboLeds.clear();
    m_vboLeds.setMode(OF_PRIMITIVE_POINTS);
    m_vboLeds.setUsage(GL_DYNAMIC_DRAW);

    for (size_t i = 0; i < m_channelGrabObjects.size(); ++i) {
        m_channelsTotalLeds[i] = 0;
        for (auto &object : m_channelGrabObjects[i]) {
            auto grabPoints = object->getLedPoints();
            if (m_channelsTotalLeds[i] + grabPoints.size() > m_maxPixInChannel)
                break;

            m_channelsTotalLeds[i] += grabPoints.size();
            m_ledPoints.reserve(m_ledPoints.size() + grabPoints.size());
            m_vboLeds.addVertices(grabPoints);

            std::move(grabPoints.begin(), grabPoints.end(), std::back_inserter(m_ledPoints));
        }
        m_totalLeds += m_channelsTotalLeds[i];
    }

    /// set minimal bounds
    ofVec2f res(100.f, 100.f);
    for_each(m_ledPoints.begin(), m_ledPoints.end(), [&res](const auto &p1) {
        if (res.x < p1.x)
            res.x = p1.x;
        if (res.y < p1.y)
            res.y = p1.y;
    });
    m_grabBounds.set(0, 0, res.x + 1, res.y + 1);
}

/// Update color for grab points, draw vbo mesh of points, grab texIn pixels in points positions
/// put grabbed in fbo by mesh vertex id
ChannelsToPix ofxLedController::updatePixels(const ofTexture &texIn)
{
    m_fboLeds.begin();
    ofClear(0, 0, 0, 255);

    m_shaderGrab.begin();
    m_shaderGrab.setUniformTexture("texIn", texIn, 1);
    m_shaderGrab.setUniform2i("inTexResolution", texIn.getWidth(), texIn.getHeight());
    m_shaderGrab.setUniform2i("outTexResolution", m_fboLeds.getWidth(), m_fboLeds.getHeight());

    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(255);

    m_vboLeds.draw(OF_MESH_POINTS);

    ofDisableBlendMode();

    ofPopStyle();

    m_shaderGrab.end();
    m_fboLeds.end();

    m_fboLeds.readToPixels(m_pixels);

    ChannelsToPix output(m_channelsTotalLeds.size());

    size_t ledsOffset = 0;
    size_t ledChannel = 0;

    /// Pack grabbed pixels from texture to channels
    for (auto ledsInChan : m_channelsTotalLeds) {
        ledsInChan *= 3; // for GL_RGB
        ledsInChan
            = ledsOffset + ledsInChan < m_pixels.size() ? ledsInChan : m_pixels.size() - ledsOffset;
        if (ledsInChan <= 0)
            break;

        std::move(m_pixels.begin() + ledsOffset, m_pixels.begin() + ledsOffset + ledsInChan,
                  std::back_inserter(output[ledChannel]));
        ledsOffset += ledsInChan;
        ledChannel++;
    }

    return output;
}

/// Make controllers grab objects highligted and editable
void ofxLedController::setSelected(bool state)
{
    if (m_bSelected == state)
        return;

    m_bSelected = state;
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grab->setActive(m_bSelected);
}

void ofxLedController::setGrabsSelected(bool state)
{
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grab->setSelected(state);
}

void ofxLedController::setFps(float fps)
{
    m_fps = fps;
    m_msecInFrame = 1000 / m_fps;
}

//
// --- Load & Save ---
//
void ofxLedController::save(const string &path)
{
    ofJson config;
    config["colorType"] = s_grabColorTypes[m_colorType];
    config["pixInLed"] = m_pixelsInLed;
    config["fps"] = m_fps;
    config["bSend"] = m_bSend;
    config["outputType"] = GetLedOutputType(m_ledOut);
    LedOutputSave(m_ledOut, config);

    ofJson grabs_array = ofJson::array();
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grabs_array.emplace_back(grab);

    if (!grabs_array.empty())
        config["grabs"] = grabs_array;

    ofstream jsonFile(path + LCFileName + ofToString(m_id) + ".json");
    jsonFile << config.dump(4);
    jsonFile.close();

    ofLogNotice() << "[ofxLedController] Save config to " << path << LCFileName << m_id << ".json";
}

void ofxLedController::load(const string &path)
{
    m_channelGrabObjects.clear();
    m_channelGrabObjects.resize(m_channelList.size());

    auto json
        = ofLoadJson(ofFilePath::addTrailingSlash(path) + LCFileName + ofToString(m_id) + ".json");
    if (json.empty()) {
        /// fallback for older XML config
        if (!ParseXmlToGrabObjects(ofFilePath::addTrailingSlash(path) + LCFileName
                                       + ofToString(m_id) + ".xml",
                                   m_channelGrabObjects, json))
            return;
    }

    LedOutputType outputType = json.contains("outputType")
                                   ? json.at("outputType").get<LedOutputType>()
                                   : LedOutputTypeLedmap;
    m_ledOut = CreateLedOutput(outputType);
    LedOutputLoad(m_ledOut, json);

    setColorType(GetColorType(json.count("colorType") ? json.at("colorType").get<string>() : ""));

    m_pixelsInLed = json.count("pixInLed") ? json.at("pixInLed").get<float>() : 2.0;
    m_fps = json.count("fps") ? json.at("fps").get<int>() : 25;
    m_bSend = json.count("bSend") ? json.at("bSend").get<bool>() : false;

    if (!json.count("grabs") || !json.at("grabs").is_array())
        return;
    ofLogNotice() << json.dump();
    vector<unique_ptr<ofxLedGrab>> jsonGrabs;
    jsonGrabs = json.at("grabs").get<vector<unique_ptr<ofxLedGrab>>>();
    jsonGrabs.erase(std::remove_if(jsonGrabs.begin(), jsonGrabs.end(),
                                   [](unique_ptr<ofxLedGrab> &grab) { return grab == nullptr; }),
                    jsonGrabs.end());

    unordered_map<size_t, size_t> chanIdCntr;
    for (auto &grab : jsonGrabs) {
        auto chan = grab->getChannel();
        if (chan >= m_channelList.size())
            continue;
        grab->setPixelsInLed(m_pixelsInLed);
        grab->setObjectId(chanIdCntr[chan]++);
        m_channelGrabObjects[chan].emplace_back(move(grab));
    }

    markDirtyGrabPoints();
    updateGrabPoints();
}

//
// --- Event handlers ---
//
void ofxLedController::mousePressed(ofMouseEventArgs &args)
{
    if (!m_bSelected)
        return;

    /// don't add grabs if pressed into existing
    if (std::count_if(begin(*m_currentChannel), end(*m_currentChannel),
                      [&args](const auto &grab) { return grab->mousePressed(args); }))
        return;

    switch (m_currentGrabType) {
        case LMGrabType::GRAB_EMPTY:
        case LMGrabType::GRAB_SELECT:
            /// set position of selection rectangle
            m_selectionRect.set(args, 0, 0);
            break;
        case LMGrabType::GRAB_LINE: {
            /// deselect previous active grabs
            setGrabsSelected(false);
            addGrab(make_unique<ofxLedGrabLine>(args, args, m_pixelsInLed));
            break;
        }
        case LMGrabType::GRAB_MATRIX: {
            setGrabsSelected(false);
            addGrab(make_unique<ofxLedGrabMatrix>(args, args, m_pixelsInLed));
            break;
        }
        case LMGrabType::GRAB_CIRCLE: {
            setGrabsSelected(false);
            addGrab(make_unique<ofxLedGrabCircle>(args, args, m_pixelsInLed));
            break;
        }
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs &args)
{
    if (!m_bSelected || m_currentChannel->empty())
        return;

    if (std::count_if(begin(*m_currentChannel), end(*m_currentChannel),
                      [&args](const auto &grab) { return grab->mouseDragged(args); })) {
        markDirtyGrabPoints();
    }
    else {
        updateSelectionRect(m_selectionRect, args);
    }
}

void ofxLedController::mouseReleased(ofMouseEventArgs &args)
{
    if (m_currentChannel->empty())
        return;

    if (!m_selectionRect.isEmpty()) {
        std::for_each(begin(*m_currentChannel), end(*m_currentChannel), [this](const auto &grab) {
            if (m_selectionRect.intersects(grab->getFrom(), grab->getTo()))
                grab->setSelected(true);
        });
        m_selectionRect.set(0, 0, 0, 0);
        return;
    }

    /// delete zero length grab, that was created with one click - not counted
    if (m_currentChannel->back()->points().empty()) {
        m_currentChannel->pop_back();
        markDirtyGrabPoints();
    }

    std::count_if(begin(*m_currentChannel), end(*m_currentChannel),
                  [&args](const auto &grab) { return grab->mouseReleased(args); });
}

void ofxLedController::keyPressed(ofKeyEventArgs &data)
{
    switch (data.key) {
        case '1':
            m_currentGrabType = LMGrabType::GRAB_LINE;
            break;
        case '2':
            m_currentGrabType = LMGrabType::GRAB_CIRCLE;
            break;
        case '3':
            m_currentGrabType = LMGrabType::GRAB_MATRIX;
            break;
        case OF_KEY_BACKSPACE:
            deleteSelectedGrabs();
            break;
        default:
            break;
    }
}

void ofxLedController::keyReleased(ofKeyEventArgs &data)
{ /* no-op */
}

void ofxLedController::setPixInLed(const float pixInled)
{
    m_pixelsInLed = pixInled;
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grab->setPixelsInLed(m_pixelsInLed);

    /// TODO call only when change objects
    markDirtyGrabPoints();
}

unique_ptr<ofxLedGrab> ofxLedController::GetUniqueTypedGrab(const ofxLedGrab *grab)
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

void ofxLedController::addGrab(unique_ptr<ofxLedGrab> &&object)
{
    object->setObjectId(m_currentChannel->size());
    object->setChannel(m_currentChannelNum);
    object->setActive(true);
    object->setSelected(true);
    m_currentChannel->emplace_back(move(object));
    markDirtyGrabPoints();
}

void ofxLedController::deleteSelectedGrabs()
{
    /// remove selected if has
    auto it = std::remove_if(begin(*m_currentChannel), end(*m_currentChannel),
                             [](auto &grab) { return grab->isSelected(); });
    if (it == end(*m_currentChannel))
        return;

    m_currentChannel->erase(it, end(*m_currentChannel));
    /// update grab ids
    size_t grabCntr = 0;
    std::for_each(begin(*m_currentChannel), end(*m_currentChannel),
                  [&grabCntr](auto &grab) { (*grab).setObjectId(grabCntr++); });

    markDirtyGrabPoints();
    return;
}

void ofxLedController::setColorType(GRAB_COLOR_TYPE type)
{
    m_colorType = type;
    m_shaderGrab = GetShaderForColorGrab(m_colorType);
}

void ofxLedController::setCurrentChannel(int chan)
{
    /// get mod from chan to be in bounds
    m_currentChannelNum = chan % m_channelList.size();
    m_currentChannel = &m_channelGrabObjects[m_currentChannelNum];
}

void ofxLedController::updateSelectionRect(ofRectangle &rect, const ofMouseEventArgs &args)
{
    if (args.x <= rect.x) {
        rect.setWidth(rect.x + rect.width - args.x);
        rect.setX(args.x);
    }
    else {
        rect.setWidth(args.x - rect.x);
    }

    if (args.y <= rect.y) {
        rect.setHeight(rect.y + rect.height - args.y);
        rect.setY(args.y);
    }
    else {
        rect.setHeight(args.y - rect.y);
    }
}

} // namespace LedMapper
