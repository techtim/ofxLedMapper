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
#include <regex>

static const vector<string> s_colorTypes = { "RGB", "RBG", "BRG", "BGR", "GRB", "GBR" };
static const vector<string> s_channelList = { "channel 1", "channel 2" };

ofxLedController::ofxLedController(const int &__id, const string &_path)
    : bSelected(false)
    , bDeletePoints(false)
    , m_recordGrabType(ofxLedGrabObject::GRAB_TYPE::GRAB_EMPTY)
    , bUdpSend(false)
    , bDmxSend(false)
    , bDoubleLine(false)
    , m_statusOk(false)
    , bUdpSetup(false)
    , bDmxSetup(false)
    , m_udpIp(RPI_IP)
    , m_curUdpIp("")
    , m_udpPort(RPI_PORT)
    , m_curUdpPort(0)
    , m_grabBounds(0, 0, 100, 100)
    , m_pixelsInLed(5.f)
    , m_fps(25.f)
    , m_totalLeds(0)
    , m_pointsCount(0)
    , m_outputHeaderOffset(2)
    , m_colorUpdator(nullptr)
    , m_statusChanged(nullptr)
    , m_channelGrabObjects(s_channelList.size())
    , m_channelTotalLeds(s_channelList.size(), 0)
    , m_currentChannelNum(0)
    , m_ledPoints(0)
{
    m_id = __id;

    setColorType(COLOR_TYPE::RGB);
    setFps(m_fps);

    path = _path;

    m_colorLine = ofColor(ofRandom(0, 100), ofRandom(50, 200), ofRandom(150, 255));
    m_colorActive = ofColor(0, m_colorLine.g, m_colorLine.b, 200);
    m_colorInactive = ofColor(m_colorLine.r, m_colorLine.g, 0, 200);

    load(path);

    setCurrentChannel(m_currentChannelNum);

    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);

    grabImg.allocate(50, 20, OF_IMAGE_COLOR);

    udpConnection.Create();
}

ofxLedController::~ofxLedController()
{
    ofLogVerbose("[ofxLedMapper] Detor: clear lines + remove event listeners + remove gui");
    m_channelGrabObjects.clear();

    udpConnection.Close();

    ofRemoveListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
}

void ofxLedController::setOnControllerStatusChange(function<void(void)> callback)
{
    m_statusChanged = callback;
}

#ifndef LED_MAPPER_NO_GUI
unique_ptr<ofxDatGui> ofxLedController::GenerateGui()
{
    ofLogVerbose() << "Generate scene gui";

    unique_ptr<ofxDatGui> gui = make_unique<ofxDatGui>();
    unique_ptr<ofxDatGuiTheme> guiTheme = make_unique<LedMapper::ofxDatGuiThemeLM>();
    gui->setTheme(guiTheme.get(), true);
    gui->addHeader("Controller");
    gui->setPosition(ofGetWidth() - 200, ofGetHeight() / 2 - 20);
    gui->setAutoDraw(false);

    gui->addToggle(LCGUIButtonSend, false);

    auto slider = gui->addSlider(LCGUISliderPix, 1, 50);
    slider->setPrecision(1);
    slider = gui->addSlider(LMGUISliderFps, 10, 60);
    slider->setPrecision(0);

    gui->addDropdown(LCGUIDropColorType, s_colorTypes);

    gui->addTextInput(LCGUITextIP, "127.0.0.1");
    gui->addTextInput(LCGUITextPort, "3000");

    gui->addDropdown(LCGUIDropChannelNum, s_channelList);

    return move(gui);
}

void ofxLedController::bindGui(ofxDatGui *gui)
{
    auto slider = gui->getSlider(LMGUISliderFps);
    slider->setValue(m_fps);
    slider->bind(m_fps);
    slider->onSliderEvent(this, &ofxLedController::onSliderEvent);
    slider = gui->getSlider(LCGUISliderPix);
    slider->setValue(m_pixelsInLed);
    slider->bind(m_pixelsInLed);
    slider->onSliderEvent(this, &ofxLedController::onSliderEvent);

    auto toggle = gui->getToggle(LCGUIButtonSend);
    toggle->setChecked(bUdpSend);
    toggle->bind(bUdpSend);
    toggle->onButtonEvent(this, &ofxLedController::onButtonEvent);

    auto dropDown = gui->getDropdown(LCGUIDropColorType);
    if (dropDown->getName() != "X") {
        dropDown->select(m_colorType);
        dropDown->onDropdownEvent(this, &ofxLedController::onDropdownEvent);
    }

    dropDown = gui->getDropdown(LCGUIDropChannelNum);
    if (dropDown->getName() != "X") {
        dropDown->select(m_currentChannelNum);
        dropDown->onDropdownEvent(this, &ofxLedController::onDropdownEvent);
    }

    auto text = gui->getTextInput(LCGUITextIP);
    if (text->getName() != "X") {
        text->setText(m_udpIp);
        text->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    }

    text = gui->getTextInput(LCGUITextPort);
    if (text->getName() != "X") {
        text->setText(ofToString(m_udpPort));
        text->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    }

#if defined(USE_DMX_FTDI) && (USE_DMX)
//    toggle = m_gui->addToggle(LCGUIButtonDmx, false);
//    toggle->bind(bDmxSend);
//    slider = m_gui->addSlider("DMX Chan", 1, 0, 512);
//    slider->bind(dmxChannel);
#endif
}
#endif

void ofxLedController::draw()
{
    int chanNum = 0;
    ofColor color;
    for (auto &channelGrabs : m_channelGrabObjects) {
        color = (chanNum == m_currentChannelNum ? m_colorActive : m_colorInactive);
        for (auto &grab : channelGrabs) {
            grab->draw(color);
        }
        ++chanNum;
    }
}

void ofxLedController::updateGrabPoints()
{
    if (!m_bDirtyPoints)
        return;

    m_bDirtyPoints = false;
    m_totalLeds = 0;
    m_ledPoints.clear();

    for (size_t i = 0; i < m_channelGrabObjects.size(); ++i) {
        m_channelTotalLeds[i] = 0;
        for (auto &object : m_channelGrabObjects[i]) {
            m_channelTotalLeds[i] += object->points().size();
            auto grabPoints = object->getLedPoints();
            m_ledPoints.reserve(m_ledPoints.size() + grabPoints.size());
            std::move(grabPoints.begin(), grabPoints.end(), std::back_inserter(m_ledPoints));
        }
        m_totalLeds += m_channelTotalLeds[i];
    }

    /// set minimal bounds
    ofVec2f res(100.f, 100.f);
    for_each(m_ledPoints.begin(), m_ledPoints.end(), [&res](const LedMapper::Point &p1) {
        if (res.x < p1.x)
            res.x = p1.x;
        if (res.y < p1.y)
            res.y = p1.y;
    });
    m_grabBounds.set(0, 0, res.x + 1, res.y + 1);
}

void ofxLedController::updatePixels(const ofPixels &grabbedImg)
{
    if (grabbedImg.size() == 0)
        return;

    updateGrabPoints();

    m_output.clear();
    for (size_t i = 0; i < m_channelGrabObjects.size(); ++i) {
        // uint16_t number of leds per chan
        m_output.push_back(m_channelTotalLeds[i] & 0xff);
        m_output.push_back(m_channelTotalLeds[i] >> 8);
    }
    /// mark end of header (num leds per channel)
    m_output.emplace_back(0xff);
    m_output.emplace_back(0xff);
    m_outputHeaderOffset = 2 * m_channelGrabObjects.size() + 2;
    /// 2 * uint8 = uint16 leds number x 2 + grab points size
    m_output.reserve(m_outputHeaderOffset + m_totalLeds * 3);

    if (m_colorUpdator != nullptr)
        for (auto &point : m_ledPoints) {
            if (point.x >= grabbedImg.getWidth() || point.y >= grabbedImg.getHeight()) {
                ofLogWarning() << "add point outside of texture: [" << point.x << ", " << point.y
                               << "]";
                continue;
            }
            ofColor color = grabbedImg.getColor(point.x, point.y);
            m_colorUpdator(m_output, color);
        }
}

void ofxLedController::setupUdp(const string &host, unsigned int port)
{
    if (bUdpSend || m_curUdpIp != host || m_curUdpPort != port) {
        udpConnection.Close();
        udpConnection.Create();
        if (udpConnection.Connect(host.c_str(), port)) {
            ofLogVerbose("[ofxLedController] setupUdp connect to " + host);
        }
        udpConnection.SetSendBufferSize(4096 * 3);
        udpConnection.SetNonBlocking(true);
        //    }
        m_curUdpIp = host;
        m_curUdpPort = port;
        bUdpSetup = true;
    }
}

void ofxLedController::sendUdp(const ofPixels &grabbedImg)
{
    auto now = ofGetSystemTime();
    if (now - m_lastFrameTime < m_msecInFrame)
        return;

    m_lastFrameTime = now;
    updatePixels(grabbedImg);
    sendUdp();
}

void ofxLedController::sendUdp()
{
    if (!bUdpSend || !bUdpSetup)
        return;

    bool prevStatus = m_statusOk;

    //    for (size_t i=0; i < m_output.size(); ++i)
    //        printf("%zu - %d \n", i, m_output[i]);

    if (m_output.size() <= m_outputHeaderOffset)
        return;

    m_statusOk = (udpConnection.Send(m_output.data(), m_output.size()) != -1);
    if (m_statusOk != prevStatus && m_statusChanged != nullptr)
        m_statusChanged();
}

void ofxLedController::setSelected(bool state)
{
    if (bSelected == state)
        return;

    bSelected = state;
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs) {
            grab->setActive(bSelected);
        }
}

unsigned int ofxLedController::getId() const { return m_id; }

const ofPixels &ofxLedController::getPixels() { return grabImg.getPixels(); }

unsigned int ofxLedController::getTotalLeds() const { return m_totalLeds; }

const ChannelsGrabObjects &ofxLedController::peekGrabObjects() const
{
    return m_channelGrabObjects;
}

//
// --- Load & Save ---
//
void ofxLedController::save(const string &path)
{
    m_ledPoints.clear();
    m_ledPoints.reserve(m_totalLeds);

    XML.clear();
    int tagNum = XML.addTag("CONF");
    XML.setValue("CONF:colorType", m_colorType, tagNum);
    XML.setValue("CONF:PixInLed", m_pixelsInLed, tagNum);
    XML.setValue("CONF:fps", m_fps, tagNum);
    XML.setValue("CONF:UdpSend", bUdpSend, tagNum);
    XML.setValue("CONF:IpAddress", m_curUdpIp, tagNum);
    XML.setValue("CONF:Port", m_curUdpPort, tagNum);
    XML.popTag();

    int lastStrokeNum = XML.addTag("STROKE");
    int chanCtr = 0;
    for (auto &channelGrabs : m_channelGrabObjects) {
        for (auto &grab : channelGrabs) {
            /// xml
            if (XML.pushTag("STROKE", lastStrokeNum)) {
                grab->setChannel(chanCtr);
                grab->save(XML);
            }
            /// json
            auto grabPoints = grab->getLedPoints();
            m_ledPoints.insert(m_ledPoints.end(), grabPoints.begin(), grabPoints.end());
        }
        ++chanCtr;
    }

    ofJson config;
    config["ipAddress"] = m_udpIp;
    config["port"] = m_udpPort;
    config["colorType"] = m_colorType;
    config["points"] = m_ledPoints;
    ofstream jsonFile(path + ofToString(m_id) + ".json");
    jsonFile << config.dump(4);
    jsonFile.close();

    auto xmlPath = ofFilePath::addTrailingSlash(path) + LCFileName + ofToString(m_id) + ".xml";
    ofLogNotice() << "[ofxLedController] Save config to " << xmlPath;
    XML.save(xmlPath);
}

void ofxLedController::load(const string &path)
{
    auto xmlPath = ofFilePath::addTrailingSlash(path) + LCFileName + ofToString(m_id) + ".xml";

    if (!XML.loadFile(xmlPath)) {
        ofLogError() << "[ofxLedController] No config with path=" << xmlPath;
        return;
    }

    ofLogNotice("[ofxLedController] Load file=" + xmlPath);

    XML.pushTag("CONF", 0);

    setColorType(getColorType(XML.getValue("colorType", 0, 0)));

    m_pixelsInLed = XML.getValue("PixInLed", 1.f, 0);
    ofLogVerbose() << "UdpSend=" << (XML.getValue("UdpSend", false, 0) ? "true" : "false");
    bUdpSend = XML.getValue("UdpSend", false, 0) ? true : false;
    m_udpIp = XML.getValue("IpAddress", RPI_IP, 0);
    m_udpPort = XML.getValue("Port", RPI_PORT, 0);
    m_fps = XML.getValue("fps", 25, 0);
    XML.popTag();

    if (bUdpSend)
        setupUdp(m_udpIp, m_udpPort);
    if (bDmxSend)
        setupDmx("");

    parseXml(XML);

    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grab->setPixelsInLed(m_pixelsInLed);

    updateGrabPoints();
}

void ofxLedController::parseXml(ofxXmlSettings &XML)
{
    int numDragTags = XML.getNumTags("STROKE:LN");

    if (numDragTags > 0) {
        m_channelGrabObjects.clear();
        m_channelGrabObjects.resize(s_channelList.size());

        ofLogVerbose("[ofxLedController] Load lines from XML");
        // we push into the last STROKE tag
        // this temporarirly treats the tag as
        // the document root.
        XML.pushTag("STROKE", numDragTags - 1);

        // we see how many points we have stored in <LN> tags
        int numPtTags = XML.getNumTags("LN");

        if (numPtTags > 0) {
            for (int i = 0; i < numPtTags; i++) {
                // the last argument of getValue can be used to specify
                // which tag out of multiple tags you are refering to.
                unique_ptr<ofxLedGrabObject> tmpObj;
                if (XML.getValue("LN:TYPE", 2, i) == ofxLedGrabObject::GRAB_TYPE::GRAB_LINE) {
                    tmpObj = make_unique<ofxLedGrabLine>(
                        XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                        XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i));
                    tmpObj->load(XML, i);
                }
                else if (XML.getValue("LN:TYPE", 2, i)
                         == ofxLedGrabObject::GRAB_TYPE::GRAB_CIRCLE) {
                    tmpObj = make_unique<ofxLedGrabCircle>(
                        XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                        XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i));
                    tmpObj->load(XML, i);
                }
                else if (XML.getValue("LN:TYPE", 2, i)
                         == ofxLedGrabObject::GRAB_TYPE::GRAB_MATRIX) {
                    tmpObj = make_unique<ofxLedGrabMatrix>(
                        XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                        XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i));
                    tmpObj->load(XML, i);
                }

                tmpObj->setObjectId(i);
                int chan = XML.getValue("LN:CHANNEL", 0, i);
                m_channelGrabObjects[chan].emplace_back(move(tmpObj));
            }
        }
        XML.popTag();
    }
}

//
// --- Event handlers ---
//
void ofxLedController::mousePressed(ofMouseEventArgs &args)
{
    if (!bSelected)
        return;

    int x = args.x, y = args.y;

    unsigned int linesCntr = 0;

    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto it = channelGrabs.begin(); it != channelGrabs.end();) {
            if ((*it) == nullptr)
                continue;
            if ((*it)->mousePressed(args)) {
                if (bDeletePoints) {
                    it = channelGrabs.erase(it);
                    markDirtyGrabPoints();
                    continue;
                }
            }
            if (bDeletePoints)
                (*it)->setObjectId(linesCntr);
            ++linesCntr;
            ++it;
        }

    switch (m_recordGrabType) {
        case ofxLedGrabObject::GRAB_TYPE::GRAB_EMPTY:
            break;

        case ofxLedGrabObject::GRAB_TYPE::GRAB_LINE:
            if (m_pointsCount == 0) {
                m_pointsCount++;
                unique_ptr<ofxLedGrabLine> tmpLine
                    = make_unique<ofxLedGrabLine>(x, y, x, y, m_pixelsInLed, bDoubleLine);
                tmpLine->setObjectId(m_currentChannel->size());
                tmpLine->setChannel(m_currentChannelNum);
                m_currentChannel->emplace_back(move(tmpLine));
            }
            else {
                m_pointsCount = 0;
                if (!m_currentChannel->empty())
                    m_currentChannel->back()->setTo(x, y);
            }
            break;

        case ofxLedGrabObject::GRAB_TYPE::GRAB_MATRIX:
            if (m_pointsCount == 0) {
                m_pointsCount++;
                unique_ptr<ofxLedGrabMatrix> tmpLine
                    = make_unique<ofxLedGrabMatrix>(x, y, x, y, m_pixelsInLed);
                tmpLine->setObjectId(m_currentChannel->size());
                tmpLine->setChannel(m_currentChannelNum);
                m_currentChannel->emplace_back(move(tmpLine));
            }
            else {
                m_pointsCount = 0;
                if (!m_currentChannel->empty())
                    m_currentChannel->back()->setTo(x, y);
            }
            break;

        case ofxLedGrabObject::GRAB_TYPE::GRAB_CIRCLE:
            unique_ptr<ofxLedGrabCircle> tmpCircle
                = make_unique<ofxLedGrabCircle>(x, y, x + 20, y + 20, m_pixelsInLed);
            tmpCircle->setObjectId(m_currentChannel->size());
            tmpCircle->setChannel(m_currentChannelNum);
            m_currentChannel->emplace_back(move(tmpCircle));
            break;

            markDirtyGrabPoints();
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs &args)
{
    if (!bSelected)
        return;

    if (!m_currentChannel->empty())
        for (auto &grab : *m_currentChannel)
            if (grab->mouseDragged(args)) {
                markDirtyGrabPoints();
                break;
            }
}

void ofxLedController::mouseReleased(ofMouseEventArgs &args)
{
    int x = args.x, y = args.y;
    if (!m_currentChannel->empty()) {
        for (auto &grab : *m_currentChannel)
            grab->mouseReleased(args);
    }
}

void ofxLedController::keyPressed(ofKeyEventArgs &data)
{
    switch (data.key) {
        case '1':
            m_recordGrabType = ofxLedGrabObject::GRAB_TYPE::GRAB_LINE;
            break;
        case '2':
            m_recordGrabType = ofxLedGrabObject::GRAB_TYPE::GRAB_CIRCLE;
            break;
        case '3':
            m_recordGrabType = ofxLedGrabObject::GRAB_TYPE::GRAB_MATRIX;
            break;
        case OF_KEY_BACKSPACE:
            bDeletePoints = true;
            break;
        case OF_KEY_ESC:
            bSelected = false;
        default:
            break;
    }
}

void ofxLedController::keyReleased(ofKeyEventArgs &data)
{
    m_recordGrabType = ofxLedGrabObject::GRAB_TYPE::GRAB_EMPTY;
    bDeletePoints = false;
}

#ifndef LED_MAPPER_NO_GUI
void ofxLedController::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    if (e.target->getName() == LCGUIDropColorType) {
        ofLogVerbose("Drop Down " + ofToString(e.child));
        setColorType(getColorType(e.child));
    }
    if (e.target->getName() == LCGUIDropChannelNum) {
        setCurrentChannel(e.child);
    }
}

void ofxLedController::onButtonEvent(ofxDatGuiButtonEvent e)
{
    if (e.target->getName() == LCGUIButtonSend) {
        if (bUdpSend)
            setupUdp(m_udpIp, m_udpPort);
    }
    if (e.target->getName() == LCGUIButtonDmx) {
        if (bDmxSend)
            setupDmx("");
    }
}

void ofxLedController::onTextInputEvent(ofxDatGuiTextInputEvent e)
{
    if (e.target->getName() == LCGUITextIP) {
        std::smatch base_match;
        auto ip = e.target->getText();
        regex ip_addr("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})"); // ([^\\.]+)
        std::regex_match(ip, base_match, ip_addr);
        if (base_match.size() > 0) {
            m_udpIp = ip;
            setupUdp(m_udpIp, m_udpPort);
        }
        else {
            e.target->setText(m_udpIp);
        }
    }
    else if (e.target->getName() == LCGUITextPort) {
        m_udpPort = ofToInt(e.target->getText());
        setupUdp(m_udpIp, m_udpPort);
    }
}

void ofxLedController::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if (e.target->getName() == LCGUISliderPix) {
        for (auto &channelGrabs : m_channelGrabObjects)
            for (auto &grab : channelGrabs)
                grab->setPixelsInLed(m_pixelsInLed);

        /// TODO call only when change objects
        markDirtyGrabPoints();
    }
    if (e.target->getName() == LMGUISliderFps) {
        setFps(m_fps);
    }
}
#endif // LED_MAPPER_NO_GUI

ofxLedController::COLOR_TYPE ofxLedController::getColorType(int num) const
{
    return num < s_colorTypes.size() ? static_cast<COLOR_TYPE>(num) : COLOR_TYPE::RGB;
}

void ofxLedController::setColorType(COLOR_TYPE type)
{
    m_colorType = type;

    switch (m_colorType) {
        case RBG:
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.r);
                output.emplace_back(color.b);
                output.emplace_back(color.g);
            };
            break;
        case BRG:
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.b);
                output.emplace_back(color.r);
                output.emplace_back(color.g);
            };
            break;
        case BGR:
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.b);
                output.emplace_back(color.g);
                output.emplace_back(color.r);
            };
            break;
        case GRB:
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.g);
                output.emplace_back(color.r);
                output.emplace_back(color.b);
            };
            break;
        case GBR:
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.g);
                output.emplace_back(color.b);
                output.emplace_back(color.r);
            };
            break;
        case RGB:
        default: // RGB
            m_colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.r);
                output.emplace_back(color.g);
                output.emplace_back(color.b);
            };
            break;
    }
}

void ofxLedController::setCurrentChannel(int chan)
{
    m_currentChannelNum = chan % s_channelList.size();
    m_currentChannel = &m_channelGrabObjects[m_currentChannelNum];
}

/// ----------- DMX ------------
void ofxLedController::setupDmx(const string &serialName)
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

void ofxLedController::sendDmx(const ofPixels &grabbedImg)
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
