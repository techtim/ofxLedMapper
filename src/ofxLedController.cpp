//
//  ofxLedController.cpp
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#include "ofxLedController.h"
#include <regex>

ofxLedController::ofxLedController(const int& __id, const string & _path):
bSelected(false),
bSetuped(false),
bRecordCircles(false),
bRecordPoints(false),
bDeletePoints(false),
bUdpSend(true),
bUdpSetup(false),
bDmxSetup(false),
bShowGui(true),
offsetBegin(0),
offsetEnd(0),
pixelsInLed(1),
pointsCount(0),
totalLeds(0)
{
    output = new unsigned char [9];
    _id = __id;
    pixelsInLed = 5.f;
    ledType = LED_TYPE::LED_RGB;
    path = _path;
    cur_udpIpAddress = "";
    cur_udpPort = 0;
    udpPort = RPI_PORT;
    udpIpAddress = RPI_IP;
    
    lineColor = ofColor(ofRandom(50, 255), ofRandom(50, 255),ofRandom(50, 255));
    Lines.clear();
    load(path);

    gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
    gui->addHeader("Ctrl "+ofToString(_id));
    gui->setWidth(LC_GUI_WIDTH);

    auto toggle = gui->addToggle(LCGUIButtonSend, false);
    toggle->bind(bUdpSend);
    toggle->onButtonEvent(this, &ofxLedController::onButtonEvent);
    auto slider = gui->addSlider(LCGUISliderPix, 1, 50);
    slider->bind(pixelsInLed);
    slider->onSliderEvent(this, &ofxLedController::onSliderEvent);
    
    auto dropDown = gui->addDropdown(LCGUIDropLedType, ledTypes);
    dropDown->select(ledType);
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

    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);

    // LINES
    
    offsetBegin = static_cast<unsigned int>(offBeg);
    offsetEnd = static_cast<unsigned int>(offEnd);
    grabImg.allocate(50, 50, OF_IMAGE_COLOR);
    
    udpConnection.Create();

    bSetuped = true;
}

ofxLedController::~ofxLedController() {
    //    gui.saveToFile("ofxLedController-"+ofToString(_id));
    delete [] output;
    Lines.clear();
}

void ofxLedController::draw() {
    if (bSetuped && bShowGui) {

        ofPushMatrix();
        //    ofTranslate(ofGetWidth()-SCREEN_W, ofGetHeight()-SCREEN_H);
        
        if(bSelected) {
            ofSetColor(255);
            gui->update();
            gui->draw();
            grabImg.draw(gui->getPosition().x,gui->getPosition().y+gui->getHeight(), 50, 50);
        }

        for(auto &line : Lines){
            ofSetColor(lineColor,150);
            line->draw();
        }

        ofPopMatrix();

    } else {
        ;;
    }
    
}

void ofxLedController::updatePixels(const ofPixels &sidesGrabImg) {

    if (!bSetuped) return;

    totalLeds = 0;
    for(auto &line : Lines){
        ofSetColor(255, 255, 255);
        totalLeds += line->points.size();
        //        ofDrawBitmapString(ofToString(j)+": leds "+ofToString(totalLeds), vert1.getInterpolated(vert2,.5));
    }

    delete []output;
    output = new unsigned char [totalLeds*3]; // (unsigned char*)(sidesImageMat.data);
    
    unsigned int cntr = 0;
    int byte_count = 0;
    unsigned int loop_cntr = 0;
    for(auto &line : Lines){
        if (loop_cntr%2 == 1)
            for (int pix_num=0;pix_num<offsetBegin; pix_num++) {
                output[byte_count++] = 0;
                output[byte_count++] = 0;
                output[byte_count++] = 0;
            }
        
        for (int pix_num=0; pix_num<line->points.size(); pix_num++) {
            ofColor col = sidesGrabImg.getColor(line->points[pix_num].x, line->points[pix_num].y);
            
            switch (ledType) {
                case LED_RGB:
                    output[byte_count++] = col.r;
                    output[byte_count++] = col.g;
                    output[byte_count++] = col.b;
                    break;
                    
                case LED_RBG:
                    output[byte_count++] = col.r;
                    output[byte_count++] = col.b;
                    output[byte_count++] = col.g;
                    break;
                case LED_BRG:
                    output[byte_count++] = col.b;
                    output[byte_count++] = col.r;
                    output[byte_count++] = col.g;
                    break;
                case LED_GRB:
                    output[byte_count++] = col.g;
                    output[byte_count++] = col.r;
                    output[byte_count++] = col.b;
                    break;
                case LED_GBR:
                    output[byte_count++] = col.g;
                    output[byte_count++] = col.b;
                    output[byte_count++] = col.r;
                    break;
                default:
                    break;
            }
        }
        
        if (loop_cntr%2 == 0 || loop_cntr==0)
            for (int pix_num=0;pix_num<offsetBegin; pix_num++) {
                output[byte_count++] = 0;
                output[byte_count++] = 0;
                output[byte_count++] = 0;
            }
        
        loop_cntr++;
    };

    grabImg.setFromPixels(output, 50, totalLeds>50?static_cast<int>(totalLeds/50):1, OF_IMAGE_COLOR);
}

void ofxLedController::setupUdp(string host, unsigned int port) {
    if (bUdpSend || cur_udpIpAddress != host || cur_udpPort != port) {
        udpConnection.Close();
        udpConnection.Create();
        if (udpConnection.Connect(host.c_str(), port)) {
            ofLogVerbose("UDP connect to "+host);
        }
        udpConnection.SetSendBufferSize(4096*3);
        udpConnection.SetNonBlocking(true);
        //    }
        cur_udpIpAddress = host;
        cur_udpPort = port;
        bSetuped = bUdpSetup = true;
    }
}

void ofxLedController::sendUdp(const ofPixels &sidesGrabImg) {
    updatePixels(sidesGrabImg);
    sendUdp();
}

void ofxLedController::sendUdp() {
    if (!bUdpSend || !bUdpSetup) return;
    
    char to_leds [totalLeds*3];

    for (int i = 0; i<totalLeds*3;i++) {
        to_leds[i] = (char)output[i];
    }

    udpConnection.Send(to_leds, totalLeds*3);
}

void ofxLedController::setupDmx(string serialName){
#ifdef USE_DMX_FTDI
    else if (dmxFtdi.open()) {
        ofLog(OF_LOG_NOTICE, "******** DMX FDI SETUP ********");
    }
#elif USE_DMX
    if (serialName == "" && dmx.connect(0, 512)) {
        ofLog(OF_LOG_NOTICE, "******** DMX PRO Default SETUP ********");
    }
    else if (dmx.connect(serialName, 512)) { //DMX_MODULES * DMX_CHANNELS);
        dmx.update(true); // black on startup
        ofLog(OF_LOG_NOTICE, "******** DMX PRO "+serialName+" SETUP ********");
    }
#endif
}

void ofxLedController::sendDmx(const ofPixels &sidesGrabImg) {
    if (!bDmxSend) return;
    
    updatePixels(sidesGrabImg);
//    if(dmx.isConnected()) {
//        int cntr = getTotalLeds()*3;
//        for (int i=1; i<513; i++) {
//            if (i>512) return;
//            dmxFtdiVal[i] = i>cntr ? (char)0 : (char)output[i-1];
//            dmx.setLevel(i, dmxFtdiVal[i]);
//        }
//        dmx.update();
//    } else if (dmxFtdi.isOpen()) {
//        int cntr = getTotalLeds()*3;
//        for (int i=1; i<513; i++) {
//            if (i>512) return;
//            dmxFtdiVal[i] = i>cntr ? (char)0 : (char)output[i-1];
//            
//        }
//        dmxFtdi.writeDmx(dmxFtdiVal, 513);
//    }
//    if (!dmx.isConnected() && !dmxFtdi.isOpen()) {
////        ofSetColor(255,0,0);
//        ofLogVerbose("NO USB->DMX");
//    }
}

void ofxLedController::setSelected(bool state) {
    bSelected = state;
    if (bSelected) gui->focus();
}

void ofxLedController::setGuiPosition(int x, int y) {
    gui->setPosition(x, y);
}

void ofxLedController::setPixels(const ofPixels & _pix) {
    delete []output;
    grabImg = _pix;
    output = new unsigned char [totalLeds*3];
    
    for (int i=0; (i<_pix.size()*3 && i<totalLeds*3); i++) {
        output[i] = _pix[i];
    }
}

const ofPixels & ofxLedController::getPixels(){
    return grabImg.getPixels();
}

unsigned int ofxLedController::getTotalLeds() const {
    return totalLeds;
}

//
// --- Load & Save
//
void ofxLedController::save(string path) {
//    ipAddress = ipText.text;
//    gui.saveToFile(path+"/ofxLedController-"+ofToString(_id));
    XML.clear();
    
    int tagNum = XML.addTag("CONF");
    XML.setValue("CONF:LedType", ledType, tagNum);
    XML.setValue("CONF:PixInLed", pixelsInLed, tagNum);
    XML.setValue("CONF:UdpSend", bUdpSend, tagNum);
    XML.setValue("CONF:IpAddress", udpIpAddress, tagNum);
    XML.setValue("CONF:Port", udpPort, tagNum);
    XML.popTag();
    
    int lastStrokeNum = XML.addTag("STROKE");
    for(auto &line : Lines)
        if( XML.pushTag("STROKE", lastStrokeNum) )
            line->save(XML);
    XML.save(path+"/"+LCFileName+ofToString(_id)+".xml");
}

void ofxLedController::load(string path) {
    if(!XML.loadFile(path+"/"+LCFileName+ofToString(_id)+".xml") ) return;
    
    XML.pushTag("CONF",0);
    ledType = getLedType(XML.getValue("LedType", 0, 0));
    pixelsInLed = XML.getValue("PixInLed", 1.f, 0);
    bUdpSend = XML.getValue("UdpSend", false, 0);
    udpIpAddress = XML.getValue("IpAddress", RPI_IP, 0);
    udpPort = XML.getValue("Port", RPI_PORT, 0);
    XML.popTag();
    
    if (bUdpSend) setupUdp(udpIpAddress, udpPort);
    if (bDmxSend) setupDmx("");
    parseXml(XML);
    for(auto &line : Lines)
        line->setPixelsInLed(pixelsInLed);
}

void ofxLedController::parseXml(ofxXmlSettings & XML) {
    int numDragTags = XML.getNumTags("STROKE:LN");
    if(numDragTags > 0) {
        Lines.clear();
        //we push into the last STROKE tag
        //this temporarirly treats the tag as
        //the document root.
        XML.pushTag("STROKE", numDragTags-1);
        
        //we see how many points we have stored in <PT> tags
        int numPtTags = XML.getNumTags("LN");
        
        if(numPtTags > 0){
            //            int totalToRead = MIN(numPtTags, LINES_NUM);
            //            Lines = new grabLine[totalToRead];
            for(int i = 0; i < numPtTags; i++){
                //the last argument of getValue can be used to specify
                //which tag out of multiple tags you are refering to.
                unique_ptr<ofxLedGrabObject> tmpObj;
                if (XML.getValue("LN:TYPE", 2,i) == GRAB_LINE) {
                    tmpObj = make_unique<ofxLedGrabLine>(
                        XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                        XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)
                    );
                    tmpObj->load(XML, i);
                } else if (XML.getValue("LN:TYPE", 2,i) == GRAB_CIRCLE) {
                    tmpObj = make_unique<ofxLedGrabCircle>(
                                XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                                XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)
                    );
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
void ofxLedController::mousePressed(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    
    if (!bSelected) return;
    
    unsigned int linesCntr = 0;
    for(auto it = Lines.begin(); it != Lines.end();){
        if ((*it)->mousePressed(args)) {
            if (bDeletePoints) {
                it = Lines.erase(it);
                continue;
            }
        }
        if (bDeletePoints) (*it)->setObjectId(linesCntr);
        linesCntr++;
        it++;
    }
    //    bSelected = true;
    posClicked = ofVec2f(x,y);
    
    if (bRecordPoints) {
        if (pointsCount == 0) {
            pointsCount++;
            unique_ptr<ofxLedGrabLine> tmpLine = make_unique<ofxLedGrabLine>(x, y, x, y, pixelsInLed, bDoubleLine);
            tmpLine->setObjectId(Lines.size());
            Lines.push_back(move(tmpLine));
        } else {
            pointsCount=0;
            if (!Lines.empty()) Lines[Lines.size()-1]->setTo(x, y);
        }
    }
    
    if (bRecordCircles) {
        
        unique_ptr<ofxLedGrabCircle> tmpCircle = make_unique<ofxLedGrabCircle>(x, y, x+20, y+20);
        tmpCircle->setObjectId(Lines.size());
        Lines.push_back(move(tmpCircle));
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    if (!bSelected) return;
    bool lineClicked = false;
    if (!Lines.empty())
        for(auto &line : Lines)
            if (line->mouseDragged(args))
                break;
}

void ofxLedController::mouseReleased(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    if (!Lines.empty()) {
        for(auto &line : Lines)
            line->mouseReleased(args);
    }
}

void ofxLedController::keyPressed(ofKeyEventArgs& data){
    
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
        case OF_KEY_ESC :
            bSelected = false;
        default:
            break;
    }
}

void ofxLedController::keyReleased(ofKeyEventArgs& data){
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

void ofxLedController::onDropdownEvent(ofxDatGuiDropdownEvent e) {
    ofLogVerbose("Drop DOwn " +ofToString(e.child));
    ledType = getLedType(e.child);
    e.target->collapse();
}

void ofxLedController::onButtonEvent(ofxDatGuiButtonEvent e) {
    if (e.target->getName() == LCGUIButtonSend) {
        if (bUdpSend) setupUdp(udpIpAddress, udpPort);
    }
    if (e.target->getName() == LCGUIButtonDmx) {
        if (bDmxSend) setupDmx("");
    }
}

void ofxLedController::onTextInputEvent(ofxDatGuiTextInputEvent e) {
    if (e.target->getName() == LCGUITextIP) {
        std::smatch base_match;
        regex ip_addr("(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})"); // ([^\\.]+)
        std::regex_match(e.target->getText(), base_match, ip_addr);
        if (base_match.size() > 0) {
            udpIpAddress = e.target->getText();
        }
        setupUdp(udpIpAddress, udpPort);
    } else if (e.target->getName() == LCGUITextPort) {
        udpPort = ofToInt(e.target->getText());
        setupUdp(udpIpAddress, udpPort);
    }
}

void ofxLedController::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target->getName() == LCGUISliderPix) {
        for(auto &line : Lines)
            line->setPixelsInLed(pixelsInLed);
    }
}

LED_TYPE ofxLedController::getLedType(int num) {
    switch (num) {
        case LED_RGB:
            return LED_RGB;
        case LED_RBG:
            return LED_RBG;
        case LED_BRG:
            return LED_BRG;
        case LED_GRB:
            return LED_GRB;
        case LED_GBR:
            return LED_GBR;
        default:
            return LED_RGB;
    }
}

