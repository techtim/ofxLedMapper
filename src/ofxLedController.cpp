//
//  ofxLedController.cpp
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#include "ofxLedController.h"
#include <regex>

static const vector<string> s_colorTypes = { "RGB", "RBG", "BRG", "BGR", "GRB", "GBR" };
static const vector<string> s_channelList = { "channel 1", "channel 2" };

ofxLedController::ofxLedController(const int &__id, const string &_path)
: bSelected(false)
, bSetuped(false)
, bDeletePoints(false)
, m_recordGrabType(ofxLedGrabObject::GRAB_TYPE::GRAB_EMPTY)
, bSetupGui(false)
, bUdpSend(true)
, m_statusOk(false)
, bUdpSetup(false)
, bDmxSetup(false)
, bShowGui(true)
, pixelsInLed(5.f)
, pointsCount(0)
, totalLeds(0)
, m_channelGrabObjects(s_channelList.size())
, m_currentChannelNum(0)
, m_ledPoints(0)
{
    _id = __id;
    
    setColorType(COLOR_TYPE::RGB);
    
    path = _path;
    cur_udpIpAddress = "";
    cur_udpPort = 0;
    udpPort = RPI_PORT;
    udpIpAddress = RPI_IP;
    
    lineColor = ofColor(ofRandom(0, 100), ofRandom(50, 200), ofRandom(150, 255));
    
    load(path);
    
    setCurrentChannel(m_currentChannelNum);
    
    setupGui();
    
    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
    
    grabImg.allocate(50, 20, OF_IMAGE_COLOR);
    
    udpConnection.Create();
    
    bSetuped = true;
}

ofxLedController::~ofxLedController()
{
    ofLogVerbose("[ofxLedMapper] Detor: clear lines + remove event listeners + remove gui");
    m_channelGrabObjects.clear();
#ifndef LED_MAPPER_NO_GUI
    gui->clear();
#endif
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

void ofxLedController::setupGui()
{
    if (bSetupGui)
        return;
    
#ifndef LED_MAPPER_NO_GUI
    
    gui = make_shared<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    gui->setTheme(new LedMapper::ofxDatGuiThemeLedMapper());
    gui->addHeader("Ctrl " + ofToString(_id));
    gui->setWidth(LM_GUI_WIDTH);
    
    auto toggle = gui->addToggle(LCGUIButtonSend, false);
    toggle->bind(bUdpSend);
    toggle->onButtonEvent(this, &ofxLedController::onButtonEvent);
    auto slider = gui->addSlider(LCGUISliderPix, 1, 50);
    slider->bind(pixelsInLed);
    slider->onSliderEvent(this, &ofxLedController::onSliderEvent);
    
    auto dropDown = gui->addDropdown(LCGUIDropColorType, s_colorTypes);
    dropDown->select(colorType);
    dropDown->onDropdownEvent(this, &ofxLedController::onDropdownEvent);
    //    toggle = gui->addToggle(LCGUIButtonDoubleLine, false);
    //    toggle->bind(bDoubleLine);
    auto textInput = gui->addTextInput(LCGUITextIP, udpIpAddress);
    textInput->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    textInput = gui->addTextInput(LCGUITextPort, ofToString(udpPort));
    textInput->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    
    dropDown = gui->addDropdown(LCGUIDropChannelNum, s_channelList);
    dropDown->select(colorType);
    dropDown->onDropdownEvent(this, &ofxLedController::onDropdownEvent);
    
#if defined(USE_DMX_FTDI) && (USE_DMX)
    toggle = gui->addToggle(LCGUIButtonDmx, false);
    toggle->bind(bDmxSend);
    slider = gui->addSlider("DMX Chan", 1, 0, 512);
    slider->bind(dmxChannel);
#endif
    
#endif // LED_MAPPER_NO_GUI
    
    bSetupGui = true;
}

void ofxLedController::draw()
{
    if (bSetuped && bShowGui) {
        
        ofPushMatrix();
#ifndef LED_MAPPER_NO_GUI
        if (bSelected && bSetupGui) {
            ofSetColor(255);
            gui->update();
            gui->draw();
            
            
            
            
            
            
            
            
        }
#endif
        int chanNum = 0;
        for (auto &channelGrabs : m_channelGrabObjects) {
            for (auto &grab : channelGrabs) {
                if (chanNum == m_currentChannelNum)
                    grab->draw(ofColor(0, lineColor.g, lineColor.b, 200));
                else
                    grab->draw(ofColor(lineColor.r, lineColor.g, 0, 200));
            }
            ++chanNum;
        }
        
        ofPopMatrix();
    }
}

void ofxLedController::updatePixels(const ofPixels &grabbedImg)
{
    if (!bSetuped)
        return;
    
    vector<uint16_t> chanTotalLeds(m_channelGrabObjects.size(), 0);
    totalLeds = 0;
    m_output.clear();
    for (size_t i = 0; i < m_channelGrabObjects.size(); ++i) {
        for (auto &object : m_channelGrabObjects[i]) {
            chanTotalLeds[i] += object->points().size();
        }
        totalLeds += chanTotalLeds[i];
        // uint16_t number of leds per chan
        m_output.push_back(chanTotalLeds[i] & 0xff);
        m_output.push_back(chanTotalLeds[i] >> 8);
    }
    /// mark end of header (num leds per channel)
    m_output.emplace_back(0xff);
    m_output.emplace_back(0xff);
    m_outputHeaderOffset = 2 * m_channelGrabObjects.size() + 2;
    /// 2 * uint8 = uint16 leds number x 2 + grab points size
    m_output.reserve(m_outputHeaderOffset + totalLeds * 3);
    
    for (auto &channelGrabs : m_channelGrabObjects) {
        for (auto &grab : channelGrabs) {
            size_t total_pix = grab->points().size();
            for (int pix_num = 0; pix_num < total_pix; ++pix_num) {
                if (grab->points()[pix_num].x >= grabbedImg.getWidth()
                    || grab->points()[pix_num].y >= grabbedImg.getHeight())
                    ofLogWarning() << "add point outside of texture";
                
                ofColor color
                = grabbedImg.getColor(grab->points()[pix_num].x, grab->points()[pix_num].y);
                colorUpdator(m_output, color);
            }
        }
    }
    
}

void ofxLedController::setupUdp(string host, unsigned int port)
{
    if (bUdpSend || cur_udpIpAddress != host || cur_udpPort != port) {
        udpConnection.Close();
        udpConnection.Create();
        if (udpConnection.Connect(host.c_str(), port)) {
            ofLogVerbose("[ofxLedController] setupUdp connect to " + host);
        }
        udpConnection.SetSendBufferSize(4096 * 3);
        udpConnection.SetNonBlocking(true);
        //    }
        cur_udpIpAddress = host;
        cur_udpPort = port;
        bSetuped = bUdpSetup = true;
    }
}

void ofxLedController::sendUdp(const ofPixels &grabbedImg)
{
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
    if (m_statusOk != prevStatus)
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
#ifndef LED_MAPPER_NO_GUI
    if (bSelected && bSetupGui)
        gui->focus();
#endif
}

void ofxLedController::setGuiPosition(int x, int y)
{
#ifndef LED_MAPPER_NO_GUI
    if (bSetupGui)
        gui->setPosition(x, y);
#endif
}

unsigned int ofxLedController::getId() const { return _id; }

const ofPixels &ofxLedController::getPixels() { return grabImg.getPixels(); }

unsigned int ofxLedController::getTotalLeds() const { return totalLeds; }

//
// --- Load & Save ---
//
void ofxLedController::save(string path)
{
    m_ledPoints.clear();
    m_ledPoints.reserve(totalLeds);
    
    XML.clear();
    int tagNum = XML.addTag("CONF");
    XML.setValue("CONF:colorType", colorType, tagNum);
    XML.setValue("CONF:PixInLed", pixelsInLed, tagNum);
    XML.setValue("CONF:UdpSend", bUdpSend, tagNum);
    XML.setValue("CONF:IpAddress", udpIpAddress, tagNum);
    XML.setValue("CONF:Port", udpPort, tagNum);
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
    config["ipAddress"] = udpIpAddress;
    config["port"] = udpPort;
    config["colorType"] = colorType;
    config["points"] = m_ledPoints;
    ofstream jsonFile(path + ofToString(_id) + ".json");
    jsonFile << config.dump(4);
    jsonFile.close();
    
    XML.save(path + "/" + LCFileName + ofToString(_id) + ".xml");
}

void ofxLedController::load(string path)
{
    if (!XML.loadFile(path + LCFileName + ofToString(_id) + ".xml")) {
        ofLogError("[ofxLedController] No config with path: " + path);
        return;
    }
    
    ofLogVerbose("[ofxLedController] Load");
    
    XML.pushTag("CONF", 0);
    
    setColorType(getColorType(XML.getValue("colorType", 0, 0)));
    
    pixelsInLed = XML.getValue("PixInLed", 1.f, 0);
    bUdpSend = XML.getValue("UdpSend", false, 0) ? true : false;
    udpIpAddress = XML.getValue("IpAddress", RPI_IP, 0);
    udpPort = XML.getValue("Port", RPI_PORT, 0);
    XML.popTag();
    
    if (bUdpSend)
        setupUdp(udpIpAddress, udpPort);
    if (bDmxSend)
        setupDmx("");
    
    parseXml(XML);
    
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto &grab : channelGrabs)
            grab->setPixelsInLed(pixelsInLed);
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
    int x = args.x, y = args.y;
    
    if (!bSelected)
        return;
    
    unsigned int linesCntr = 0;
    
    for (auto &channelGrabs : m_channelGrabObjects)
        for (auto it = channelGrabs.begin(); it != channelGrabs.end();) {
            if ((*it) == nullptr)
                continue;
            if ((*it)->mousePressed(args)) {
                if (bDeletePoints) {
                    it = channelGrabs.erase(it);
                    continue;
                }
            }
            if (bDeletePoints)
                (*it)->setObjectId(linesCntr);
            linesCntr++;
            ++it;
        }
    
    posClicked = ofVec2f(x, y);
    
    switch (m_recordGrabType) {
        case ofxLedGrabObject::GRAB_TYPE::GRAB_EMPTY:
            break;
            
        case ofxLedGrabObject::GRAB_TYPE::GRAB_LINE:
            if (pointsCount == 0) {
                pointsCount++;
                unique_ptr<ofxLedGrabLine> tmpLine
                = make_unique<ofxLedGrabLine>(x, y, x, y, pixelsInLed, bDoubleLine);
                tmpLine->setObjectId(m_currentChannel->size());
                tmpLine->setChannel(m_currentChannelNum);
                m_currentChannel->emplace_back(move(tmpLine));
            }
            else {
                pointsCount = 0;
                if (!m_currentChannel->empty())
                    m_currentChannel->back()->setTo(x, y);
            }
            break;
            
        case ofxLedGrabObject::GRAB_TYPE::GRAB_MATRIX:
            if (pointsCount == 0) {
                pointsCount++;
                unique_ptr<ofxLedGrabMatrix> tmpLine
                = make_unique<ofxLedGrabMatrix>(x, y, x, y, pixelsInLed);
                tmpLine->setObjectId(m_currentChannel->size());
                tmpLine->setChannel(m_currentChannelNum);
                m_currentChannel->emplace_back(move(tmpLine));
            }
            else {
                pointsCount = 0;
                if (!m_currentChannel->empty())
                    m_currentChannel->back()->setTo(x, y);
            }
            break;
            
        case ofxLedGrabObject::GRAB_TYPE::GRAB_CIRCLE:
            unique_ptr<ofxLedGrabCircle> tmpCircle
            = make_unique<ofxLedGrabCircle>(x, y, x + 20, y + 20, pixelsInLed);
            tmpCircle->setObjectId(m_currentChannel->size());
            tmpCircle->setChannel(m_currentChannelNum);
            m_currentChannel->emplace_back(move(tmpCircle));
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs &args)
{
    if (!bSelected)
        return;
    
    if (!m_currentChannel->empty())
        for (auto &grab : *m_currentChannel)
            if (grab->mouseDragged(args))
                break;
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
            setupUdp(udpIpAddress, udpPort);
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
        regex ip_addr("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})"); // ([^\\.]+)
        std::regex_match(e.target->getText(), base_match, ip_addr);
        if (base_match.size() > 0) {
            udpIpAddress = e.target->getText();
            setupUdp(udpIpAddress, udpPort);
        }
        else {
            e.target->setText(udpIpAddress);
        }
    }
    else if (e.target->getName() == LCGUITextPort) {
        udpPort = ofToInt(e.target->getText());
        setupUdp(udpIpAddress, udpPort);
    }
}

void ofxLedController::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if (e.target->getName() == LCGUISliderPix) {
        for (auto &channelGrabs : m_channelGrabObjects)
            for (auto &grab : channelGrabs)
                grab->setPixelsInLed(pixelsInLed);
    }
}
#endif // LED_MAPPER_NO_GUI

ofxLedController::COLOR_TYPE ofxLedController::getColorType(int num) const
{
    return num < s_colorTypes.size() ? static_cast<COLOR_TYPE>(num) : COLOR_TYPE::RGB;
}

void ofxLedController::setColorType(COLOR_TYPE type)
{
    colorType = type;
    switch (colorType) {
        case RBG:
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.r);
                output.emplace_back(color.b);
                output.emplace_back(color.g);
            };
            break;
        case BRG:
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.b);
                output.emplace_back(color.r);
                output.emplace_back(color.g);
            };
            break;
        case BGR:
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.b);
                output.emplace_back(color.g);
                output.emplace_back(color.r);
            };
            break;
        case GRB:
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.g);
                output.emplace_back(color.r);
                output.emplace_back(color.b);
            };
            break;
        case GBR:
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.g);
                output.emplace_back(color.b);
                output.emplace_back(color.r);
            };
            break;
        case RGB:
        default: // RGB
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.r);
                output.emplace_back(color.g);
                output.emplace_back(color.b);
            };
            break;
    }
}

void ofxLedController::setCurrentChannel(int chan) {
    m_currentChannelNum = chan % s_channelList.size();
    m_currentChannel = &m_channelGrabObjects[m_currentChannelNum];
}

/// ----------- DMX ------------
void ofxLedController::setupDmx(string serialName)
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
