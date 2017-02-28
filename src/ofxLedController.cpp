//
//  ofxLedController.cpp
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#include "ofxLedController.h"
#include <regex>

ofxLedController::ofxLedController(const int &__id, const string &_path)
: bSelected(false)
, bSetuped(false)
, bRecordCircles(false)
, bRecordPoints(false)
, bDeletePoints(false)
, bSetupGui(false)
, bUdpSend(true)
, bUdpSetup(false)
, bDmxSetup(false)
, bShowGui(true)
, offsetBegin(0)
, offsetEnd(0)
, pixelsInLed(5.f)
, pointsCount(0)
, totalLeds(0)
{
    _id = __id;
    
    setColorType(COLOR_TYPE::RGB);
    
    path = _path;
    cur_udpIpAddress = "";
    cur_udpPort = 0;
    udpPort = RPI_PORT;
    udpIpAddress = RPI_IP;
    
    lineColor = ofColor(ofRandom(50, 255), ofRandom(50, 255), ofRandom(50, 255));
    
    load(path);

    setupGui();
    
    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
    
    offsetBegin = static_cast<unsigned int>(offBeg);
    offsetEnd = static_cast<unsigned int>(offEnd);
    grabImg.allocate(50, 20, OF_IMAGE_COLOR);
    output.resize(1000);
    
    udpConnection.Create();
    
    bSetuped = true;
}

ofxLedController::~ofxLedController()
{
    ofLogVerbose("[ofxLedMapper] Detor: clear lines + remove event listeners + remove gui");
    Lines.clear();
    gui->clear();
    delete gui;
    udpConnection.Close();
    
    ofRemoveListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofRemoveListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);
}

void ofxLedController::setupGui()
{
    if (bSetupGui)
        return;
    
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
    gui->addHeader("Ctrl " + ofToString(_id));
    gui->setWidth(LC_GUI_WIDTH);
    
    auto toggle = gui->addToggle(LCGUIButtonSend, false);
    toggle->bind(bUdpSend);
    toggle->onButtonEvent(this, &ofxLedController::onButtonEvent);
    auto slider = gui->addSlider(LCGUISliderPix, 1, 50);
    slider->bind(pixelsInLed);
    slider->onSliderEvent(this, &ofxLedController::onSliderEvent);
    
    auto dropDown = gui->addDropdown(LCGUIDropColorType, colorTypes);
    dropDown->select(colorType);
    dropDown->onDropdownEvent(this, &ofxLedController::onDropdownEvent);
    //    toggle = gui->addToggle(LCGUIButtonDoubleLine, false);
    //    toggle->bind(bDoubleLine);
    auto textInput = gui->addTextInput(LCGUITextIP, udpIpAddress);
    textInput->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    textInput = gui->addTextInput(LCGUITextPort, ofToString(udpPort));
    textInput->onTextInputEvent(this, &ofxLedController::onTextInputEvent);
    
#if defined(USE_DMX_FTDI) && (USE_DMX)
    toggle = gui->addToggle(LCGUIButtonDmx, false);
    toggle->bind(bDmxSend);
    slider = gui->addSlider("DMX Chan", 1, 0, 512);
    slider->bind(dmxChannel);
#endif
    
    bSetupGui = true;
}

void ofxLedController::draw()
{
    if (bSetuped && bShowGui) {
        
        ofPushMatrix();
        
        if (bSelected && bSetupGui) {
            ofSetColor(255);
            gui->update();
            gui->draw();
            grabImg.draw(gui->getPosition().x, gui->getPosition().y + gui->getHeight(), 50, 20);
        }
        
        for (auto &line : Lines) {
            ofSetColor(lineColor, 150);
            line->draw();
        }
        
        ofPopMatrix();
    }
}

void ofxLedController::updatePixels(const ofPixels &sidesGrabImg)
{
    if (!bSetuped)
        return;
    
    totalLeds = 0;
    for (auto &line : Lines) {
        totalLeds += line->points.size();
    }
    
    if (output.size() < totalLeds * 3) {
        output.resize(totalLeds * 3);
    }
    
    output.clear();
    
    unsigned int loop_cntr = 0;
    for (auto &line : Lines) {
        if (loop_cntr % 2 == 1)
            for (int pix_num = 0; pix_num < offsetBegin; pix_num++) {
                output.emplace_back(0);
                output.emplace_back(0);
                output.emplace_back(0);
            }
        size_t total_pix = line->points.size();
        for (int pix_num = 0; pix_num < total_pix; ++pix_num) {
            ofColor color = sidesGrabImg.getColor(line->points[pix_num].x, line->points[pix_num].y);
            
            colorUpdator(output, color);
        }
        
        if (loop_cntr % 2 == 0 || loop_cntr == 0)
            for (int pix_num = 0; pix_num < offsetBegin; pix_num++) {
                output.emplace_back(0);
                output.emplace_back(0);
                output.emplace_back(0);
            }
        
        loop_cntr++;
    };
    
    //    ofPixels_<char> pixs;
    //    pixs.setFromExternalPixels(output.data(), 50,
    //    output.size()>50?static_cast<int>(output.size()*0.5):1, OF_IMAGE_COLOR);
    //    grabImg.setFromPixels(pixs);
    grabImg.setFromPixels(reinterpret_cast<unsigned char *>(output.data()), 50,
                          output.size() > 50 ? static_cast<int>(output.size() / 50) : 1,
                          OF_IMAGE_COLOR);
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

void ofxLedController::sendUdp(const ofPixels &sidesGrabImg)
{
    updatePixels(sidesGrabImg);
    sendUdp();
}

void ofxLedController::sendUdp()
{
    if (!bUdpSend || !bUdpSetup)
        return;
    
    udpConnection.Send(output.data(), output.size());
    
    //    char to_leds [totalLeds*3];
    //    for (int i = 0; i<totalLeds*3;i++) {
    //        to_leds[i] = static_cast<char>(output[i]);
    //    }
    //    udpConnection.Send(to_leds, totalLeds*3);
}

void ofxLedController::setSelected(bool state)
{
    bSelected = state;
    if (bSelected && bSetupGui)
        gui->focus();
}

void ofxLedController::setGuiPosition(int x, int y)
{
    if (bSetupGui)
        gui->setPosition(x, y);
}

void ofxLedController::setPixels(const ofPixels &_pix)
{
    grabImg = _pix;
    output.resize(_pix.size() * 3);
    
    for (int i = 0; i < _pix.size() * 3; ++i) {
        output[i] = _pix[i];
    }
}

unsigned int ofxLedController::getId() const { return _id; }

const ofPixels &ofxLedController::getPixels() { return grabImg.getPixels(); }

unsigned int ofxLedController::getTotalLeds() const { return totalLeds; }

//
// --- Load & Save ---
//
void ofxLedController::save(string path)
{
    XML.clear();
    
    int tagNum = XML.addTag("CONF");
    XML.setValue("CONF:colorType", colorType, tagNum);
    XML.setValue("CONF:PixInLed", pixelsInLed, tagNum);
    XML.setValue("CONF:UdpSend", bUdpSend, tagNum);
    XML.setValue("CONF:IpAddress", udpIpAddress, tagNum);
    XML.setValue("CONF:Port", udpPort, tagNum);
    XML.popTag();
    
    int lastStrokeNum = XML.addTag("STROKE");
    for (auto &line : Lines)
        if (XML.pushTag("STROKE", lastStrokeNum))
            line->save(XML);
    XML.save(path + "/" + LCFileName + ofToString(_id) + ".xml");
}

void ofxLedController::load(string path)
{
    if (!XML.loadFile(path + "/" + LCFileName + ofToString(_id) + ".xml"))
        return;
    
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
    
    for (auto &line : Lines)
        line->setPixelsInLed(pixelsInLed);
}

void ofxLedController::parseXml(ofxXmlSettings &XML)
{
    int numDragTags = XML.getNumTags("STROKE:LN");
    if (numDragTags > 0) {
        if (!Lines.empty()) {
            Lines.clear();
            ofLogVerbose("[ofxLedController] Clear lines");
        }
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
                if (XML.getValue("LN:TYPE", 2, i) == GRAB_LINE) {
                    tmpObj = make_unique<ofxLedGrabLine>(XML.getValue("LN:fromX", 0, i),
                                                         XML.getValue("LN:fromY", 0, i),
                                                         XML.getValue("LN:toX", 0, i),
                                                         XML.getValue("LN:toY", 0, i));
                    tmpObj->load(XML, i);
                }
                else if (XML.getValue("LN:TYPE", 2, i) == GRAB_CIRCLE) {
                    tmpObj = make_unique<ofxLedGrabCircle>(XML.getValue("LN:fromX", 0, i),
                                                           XML.getValue("LN:fromY", 0, i),
                                                           XML.getValue("LN:toX", 0, i),
                                                           XML.getValue("LN:toY", 0, i));
                    tmpObj->load(XML, i);
                }
                tmpObj->setObjectId(i);
                Lines.push_back(move(tmpObj));
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
    for (auto it = Lines.begin(); it != Lines.end();) {
        if ((*it) == nullptr)
            continue;
        if ((*it)->mousePressed(args)) {
            if (bDeletePoints) {
                it = Lines.erase(it);
                continue;
            }
        }
        if (bDeletePoints)
            (*it)->setObjectId(linesCntr);
        linesCntr++;
        it++;
    }
    
    posClicked = ofVec2f(x, y);
    
    if (bRecordPoints) {
        if (pointsCount == 0) {
            pointsCount++;
            unique_ptr<ofxLedGrabLine> tmpLine = make_unique<ofxLedGrabLine>(x, y, x, y, pixelsInLed, bDoubleLine);
            tmpLine->setObjectId(Lines.size());
            Lines.push_back(move(tmpLine));
        }
        else {
            pointsCount = 0;
            if (!Lines.empty())
                Lines[Lines.size() - 1]->setTo(x, y);
        }
    }
    else if (bRecordCircles) {
        unique_ptr<ofxLedGrabCircle> tmpCircle = make_unique<ofxLedGrabCircle>(x, y, x + 20, y + 20);
        tmpCircle->setObjectId(Lines.size());
        Lines.push_back(move(tmpCircle));
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs &args)
{
    int x = args.x, y = args.y;
    if (!bSelected)
        return;
    bool lineClicked = false;
    if (!Lines.empty())
        for (auto &line : Lines)
            if (line->mouseDragged(args))
                break;
}

void ofxLedController::mouseReleased(ofMouseEventArgs &args)
{
    int x = args.x, y = args.y;
    if (!Lines.empty()) {
        for (auto &line : Lines)
            line->mouseReleased(args);
    }
}

void ofxLedController::keyPressed(ofKeyEventArgs &data)
{
    
    switch (data.key) {
        case OF_KEY_COMMAND:
            bRecordPoints = true;
            break;
        case OF_KEY_BACKSPACE:
            bDeletePoints = true;
            break;
        case OF_KEY_SHIFT:
            bRecordCircles = true;
            break;
        case OF_KEY_ESC:
            bSelected = false;
        default:
            break;
    }
}

void ofxLedController::keyReleased(ofKeyEventArgs &data)
{
    switch (data.key) {
        case OF_KEY_COMMAND:
            bRecordPoints = false;
            break;
        case OF_KEY_SHIFT:
            bRecordCircles = false;
            break;
        case OF_KEY_BACKSPACE:
            bDeletePoints = false;
            break;
    }
}

void ofxLedController::onDropdownEvent(ofxDatGuiDropdownEvent e)
{
    ofLogVerbose("Drop DOwn " + ofToString(e.child));
    setColorType(getColorType(e.child));
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
        for (auto &line : Lines)
            line->setPixelsInLed(pixelsInLed);
    }
}

COLOR_TYPE ofxLedController::getColorType(int num) const
{
    return num < colorTypes.size() ? static_cast<COLOR_TYPE>(num) : COLOR_TYPE::RGB;
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
        default: // RGB
            colorUpdator = [](vector<char> &output, ofColor &color) {
                output.emplace_back(color.r);
                output.emplace_back(color.g);
                output.emplace_back(color.b);
            };
            break;
    }
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
#endif
}

void ofxLedController::sendDmx(const ofPixels &sidesGrabImg)
{
    if (!bDmxSend)
        return;
    
    updatePixels(sidesGrabImg);
    
#ifdef USE_DMX
    if (dmx.isConnected()) {
        int cntr = getTotalLeds() * 3;
        for (int i = 1; i < 513; i++) {
            if (i > 512)
                return;
            dmxFtdiVal[i] = i > cntr ? (char)0 : (char)output[i - 1];
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
            dmxFtdiVal[i] = i > cntr ? (char)0 : (char)output[i - 1];
        }
        dmxFtdi.writeDmx(dmxFtdiVal, 513);
    }
    else {
        ofLogVerbose("NO USB->DMX");
    }
#endif
}
