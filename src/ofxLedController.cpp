//
//  ofxLedController.cpp
//  ledGipsy
//
//  Created by Tim TVL on 05/06/14.
//
//

#include "ofxLedController.h"


ofxLedController::ofxLedController():
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
    ipText.setup();
    output = new unsigned char [9];
    path = "Ctrls";
}

ofxLedController::~ofxLedController() {
    gui.saveToFile("ofxLedController-"+ofToString(_id));
    delete [] output;
    Lines.clear();
}

void ofxLedController::setup(const int& __id, const string & _path, const ofRectangle& _region, int _maxWidth, int _maxHeight) {
    region = _region;
    _id = __id;
    xPos = curXPos =region.x;
    yPos = curYPos = region.y;
    width = region.width;
    height = region.height;
    pixelsInLed = curPixelsInLed = 5.f;
    maxWidth = _maxWidth ? _maxWidth : ofGetScreenWidth();
    maxHeight = _maxHeight? _maxHeight : ofGetScreenHeight();
    path = _path;
    
    lineColor = ofColor(ofRandom(50, 255), ofRandom(50, 255),ofRandom(50, 255));
    
    ofxGuiSetDefaultWidth(200);
    gui.setup("ledCtrl-"+ofToString(_id)); // most of the time you don't need a name but don't forget to call setup
//    gui.setName("control");
    gui.setSize(255, 400);

//    pInLed.setParent(&guiGroup);
//    gui.add(xPos.set("X", xPos, -100, static_cast<int>(ofGetWidth()/2)));
//    gui.add(yPos.set("Y", yPos, -100, static_cast<int>(ofGetHeight()/2)));
//    gui.add(width.set("W", width,0,ofGetWidth()));
//    gui.add(height.set("H", height,0,ofGetHeight()));
    guiGroup.add(pixelsInLed.set( "PixInLed", 5.f, 1.f, 100.f ));
    guiGroup.add(ledType.set("LedType", 0,0,4));
    guiGroup.add(bDoubleLine.set("Double Line", false));
    guiGroup.add(bUdpSend.set("UDP", false));
    guiGroup.add(ipAddress.set("IP", RPI_IP));
    guiGroup.add(port.set("Port", RPI_PORT, RPI_PORT, RPI_PORT+6));
    guiGroup.add(bDmxSend.set("DMX", false));
    guiGroup.add(dmxChannel.set("DMX Chan", 1, 0, 512));
    
    gui.add(guiGroup);
//
    if (_id<8) {
        gui.setPosition(ofGetWidth()-250,50+_id*100);
    } else {
        gui.setPosition(10+(_id-7)*250, 600+(_id%2==0?100:0));
    }
    gui.setWidthElements(255);
    gui.minimizeAll();
    load(path);

    ipText.bounds.x = gui.getPosition().x;
    ipText.bounds.y = gui.getPosition().y+gui.getHeight();
    ipText.bounds.height = 20;
    ipText.bounds.width = 200;

//    bDmxSend.addListener(this, &ofxLedController::notifyParameterChanged);
//    bUdpSend.addListener(this, &ofxLedController::notifyParameterChanged);
//    pixelsInLed.addListener(this, &ofxLedController::notifyParameterChanged);
//    gui.addListener(this, &ofxLedController::notifyParameterChanged);

    ofAddListener(ofEvents().mousePressed, this, &ofxLedController::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &ofxLedController::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &ofxLedController::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &ofxLedController::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxLedController::keyReleased);

    region.height = height;
    //    region.y = ofGetScreenHeight() - region.height - yPos;
    // LINES
    
    offsetBegin = static_cast<unsigned int>(offBeg);
    offsetEnd = static_cast<unsigned int>(offEnd);
    grabImg.allocate(region.width, region.height, OF_IMAGE_COLOR);
    
    udpConnection.Create();

    bSetuped = true;
}

void ofxLedController::setupUdp(string host, unsigned int port) {
    if (bUdpSend || udpHost != host || udpPort != port) {
        udpConnection.Close();
        udpConnection.Create();
        if (udpConnection.Connect(host.c_str(), port)) {
            ofLogVerbose("UDP connect to "+host);
        }
        udpConnection.SetSendBufferSize(4096*3);
        udpConnection.SetNonBlocking(true);
        //    }
        udpHost = host;
        udpPort = port;
        bSetuped = bUdpSetup = true;
    }
    //    ((ofxUITextInput*)gui->getWidget("host"))

}

void ofxLedController::setupDmx(string serialName){
    if (serialName == "" && dmx.connect(0, 512)) {
        ofLog(OF_LOG_NOTICE, "******** DMX PRO Default SETUP ********");
    }
    else if (dmx.connect(serialName, 512)) { //DMX_MODULES * DMX_CHANNELS);
        dmx.update(true); // black on startup
        ofLog(OF_LOG_NOTICE, "******** DMX PRO "+serialName+" SETUP ********");
    }
#ifdef USE_DMX_FTDI
    else if (dmxFtdi.open()) {
        ofLog(OF_LOG_NOTICE, "******** DMX FDI SETUP ********");
    }

        ((ofxUITextInput*)gui->getWidget("host"))
#endif
    
}

void ofxLedController::draw() {
    if (pixelsInLed != curPixelsInLed) {
        curPixelsInLed = pixelsInLed;
        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++)
            (*i)->setPixelsInLed(pixelsInLed);
    }
    if (bSetuped && bShowGui) {
        region.x = xPos;
        region.y = yPos;
        region.width = width;
        region.height = height;
        
        ofPushMatrix();
        //    ofTranslate(ofGetWidth()-SCREEN_W, ofGetHeight()-SCREEN_H);
        
        if(bSelected) {
            ofSetColor(255);
            ofRect(gui.getShape());
        }
        ofSetColor(200,200,200,200);
        ofNoFill();
        ofRect(region);
        ofFill();
        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
            ofSetColor(lineColor,150);
            (*i)->draw();
            ofSetColor(255, 255, 255,255);
//            ofDrawBitmapString(ofToString(static_cast<int>(vert1.distance(vert2)/pixelsInLed)), vert1.getInterpolated(vert2,.5));
            
        }
        ofSetColor(255, 255, 255,255);
        //            grabImg.draw(region.x,region.y-maxHeight/region.width, region.width, maxHeight/region.width);

        grabImg.draw(gui.getPosition().x,gui.getPosition().y+gui.getShape().height+20, 50, 50);
        ofPopMatrix();
//        
//        ipText.bounds.x = gui.getPosition().x+32;
//        ipText.bounds.y = gui.getPosition().y+gui.getShape().height-21;

        gui.draw();
        ipText.bounds.x = gui.getPosition().x;
        ipText.bounds.y = gui.getPosition().y+gui.getHeight();
        ipText.draw();
        if (ipText.getIsEditing()) {
            ipAddress = ipText.text;
        }
    } else {
        ;;
    }
    
}

void ofxLedController::mousePressed(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    
    if (gui.getShape().inside(x, y)) {
        bSelected = true;
        return;
    }
    
    if (!bSelected) return;
    
    unsigned int linesCntr = 0;
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
        if ((*i)->mousePressed(args)) {
            if (bDeletePoints) { Lines.erase(i); i--; continue;  }
        }
        if (bDeletePoints) (*i)->setObjectId(linesCntr);
        linesCntr++;
    }
    //    bSelected = true;
    posClicked = ofVec2f(x,y);
    
    if (bRecordPoints) {
        if (pointsCount == 0) {
            pointsCount++;
            ofxLedGrabLine *tmpLine = new ofxLedGrabLine(x, y, 0,0);
            tmpLine->setObjectId(Lines.size());
            Lines.push_back(tmpLine);
        } else {
            pointsCount=0;
            if (Lines.size()>0)Lines[Lines.size()-1]->setTo(x, y);
            if (bDoubleLine) {
                ofxLedGrabLine *tmpLine = new ofxLedGrabLine(*Lines[Lines.size()-1]);
                Lines.push_back(tmpLine);
            }
        }
    }
    
    if (bRecordCircles) {
        
        ofxLedGrabCircle *tmpCircle = new ofxLedGrabCircle(x, y, x+20, y+20);
        tmpCircle->setObjectId(Lines.size());
        Lines.push_back(tmpCircle);
    }
}

void ofxLedController::mouseDragged(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    if (bSelected) {
        
        bool lineClicked = false;
        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
            if ((*i)->mouseDragged(args)) {
                break;
            }
        }

    }
}

void ofxLedController::mouseReleased(ofMouseEventArgs & args){
    int x = args.x, y = args.y;
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
        (*i)->mouseReleased(args);
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
//        case 's':
//            save(path);
//            break;
//        case 'l':
//            load(path);
//            break;
        case OF_KEY_ESC :
            bSelected = false;
        default:
            break;
    }
}

void ofxLedController::keyReleased(ofKeyEventArgs& data){
    //    int key = data.key;
    
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


void ofxLedController::notifyParameterChanged(ofAbstractParameter & param){
    if (param.getName() == "PixInLed"){
        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++)
            (*i)->setPixelsInLed(pixelsInLed);
    }
    if (param.getName() == "DMX Chan"){
        for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++)
            (*i)->setPixelsInLed(pixelsInLed);
    }
    if (param.getName() == "UDP"){
        ipAddress = ipText.text;
        if (bUdpSend) setupUdp(ipAddress, port);
    }
    if (param.getName() == "DMX"){
        if (bDmxSend) setupDmx("");
    }
    ipAddress = ipText.text;
}

unsigned int ofxLedController::getTotalLeds() const {
    return totalLeds;
}

void ofxLedController::updatePixels(const ofPixels &sidesGrabImg) {

    if (!bSetuped) return;

    totalLeds = 0;
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
        ofSetColor(255, 255, 255);
        totalLeds += (*i)->points.size();
        +(offsetBegin*Lines.size());
        //        ofDrawBitmapString(ofToString(j)+": leds "+ofToString(totalLeds), vert1.getInterpolated(vert2,.5));
    }
    delete []output;
    output = new unsigned char [totalLeds*3]; // (unsigned char*)(sidesImageMat.data);
    
    unsigned int cntr = 0;
    int byte_count = 0;
    unsigned int loop_cntr = 0;
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++){
        //     //        ofVec2f vert1 = j%2 == 0 ? ofVec2f(Lines[j].fromX, Lines[j].fromY) : ofVec2f(Lines[j].toX, Lines[j].toY);
        //     //        ofVec2f vert2 = j%2 == 0 ? ofVec2f(Lines[j].toX, Lines[j].toY) : ofVec2f(Lines[j].fromX, Lines[j].fromY);
        
        //     ofVec2f vert1 = ofVec2f(Lines[j].fromX, Lines[j].fromY);
        //     ofVec2f vert2 = ofVec2f(Lines[j].toX, Lines[j].toY);
        //
        if (loop_cntr%2 == 1)
            for (int pix_num=0;pix_num<offsetBegin; pix_num++) {
                output[byte_count++] = 0;
                output[byte_count++] = 0;
                output[byte_count++] = 0;
            }
        
        for (int pix_num=0;pix_num<(*i)->points.size(); pix_num++) {
            ofColor col = sidesGrabImg.getColor((*i)->points[pix_num].x, (*i)->points[pix_num].y);
            
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

void ofxLedController::sendUdp(const ofPixels &sidesGrabImg) {
    updatePixels(sidesGrabImg);
    sendUdp();
}

void ofxLedController::sendUdp() {
    if (!bUdpSend || !bUdpSetup) return;
    
//    if (ofGetFrameNum()%2!=0) return;
    char to_leds [totalLeds*3];
//    int cntr = 0;
    for (int i = 0; i<totalLeds*3;i++) {
        to_leds[i] = (char)output[i];
//        to_leds[cntr++] = (char)output[i];
    }

    udpConnection.Send(to_leds, totalLeds*3);
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
//
//
//    if (!dmx.isConnected() && !dmxFtdi.isOpen()) {
////        ofSetColor(255,0,0);
//        ofLogVerbose("NO USB->DMX");
//    }
}

const ofPixels & ofxLedController::getPixels(){
    return grabImg.getPixels();
}

void ofxLedController::setPixels(const ofPixels & _pix) {
    delete []output;
    grabImg = _pix;
    output = new unsigned char [totalLeds*3];

    for (int i=0; (i<_pix.size()*3 && i<totalLeds*3); i++) {
        output[i] = _pix[i];
//        output[i+1] = _pix[i].g;
    }
}
    
void ofxLedController::addLine(ofxLedGrabObject * tmpLine) {
    Lines.push_back(tmpLine);
};

void ofxLedController::addLine(int x1, int y1, int x2, int y2) {
    ofxLedGrabLine *tmpLine = new ofxLedGrabLine(x1, y1, x2, y2);
    Lines.push_back(tmpLine);
};

void ofxLedController::save(string path) {
    ipAddress = ipText.text;
    gui.saveToFile(path+"/ofxLedController-"+ofToString(_id));
    XML.clear();
    int lastStrokeNum = XML.addTag("STROKE");
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++) {
        //        if ((*i)->getType() == GRAB_LINE){
        if( XML.pushTag("STROKE", lastStrokeNum) )
            (*i)->save(XML);
    }
    XML.save(path+"/ColLines-"+ofToString(_id)+".xml");
}

void ofxLedController::load(string path) {
    ofFile file = ofFile(path+"/ofxLedController-"+ofToString(_id));
    if (!file.exists()){
        gui.saveToFile(path+"/ofxLedController-"+ofToString(_id));
       return;
    }
    gui.loadFromFile(file.path());

    ipText.text = ipAddress;
    if (bUdpSend) setupUdp(ipText.text, port);
    if (bDmxSend) setupDmx("");

    region.x = xPos;
    region.y = yPos;
    if( XML.loadFile(path+"/ColLines-"+ofToString(_id)+".xml") ){
        parseXml(XML);
    }
    for(vector<ofxLedGrabObject *>::iterator  i = Lines.begin(); i != Lines.end(); i++)
        (*i)->setPixelsInLed(pixelsInLed);
}

void ofxLedController::parseXml (ofxXmlSettings & XML) {
    int numDragTags = XML.getNumTags("STROKE:LN");
    if(numDragTags > 0) {
        Lines.clear();
        //we push into the last STROKE tag
        //this temporarirly treats the tag as
        //the document root.
        XML.pushTag("STROKE", numDragTags-1);
//        XML.pushTag("STROKE", numDragTags-1);
        
        //we see how many points we have stored in <PT> tags
        int numPtTags = XML.getNumTags("LN");
        
        if(numPtTags > 0){
            
            //            int totalToRead = MIN(numPtTags, LINES_NUM);
            //            Lines = new grabLine[totalToRead];
            
            for(int i = 0; i < numPtTags; i++){
                //the last argument of getValue can be used to specify
                //which tag out of multiple tags you are refering to.
                ofxLedGrabObject * tmpObj;
                if (XML.getValue("LN:TYPE", 2,i) == GRAB_LINE) {
                    tmpObj = new ofxLedGrabLine(
                        XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                        XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)
                    );
                    tmpObj->load(XML, i);
                } else if (XML.getValue("LN:TYPE", 2,i) == GRAB_CIRCLE) {
                    tmpObj = new ofxLedGrabCircle(
                                XML.getValue("LN:fromX", 0, i), XML.getValue("LN:fromY", 0, i),
                                XML.getValue("LN:toX", 0, i), XML.getValue("LN:toY", 0, i)
                    );
                    tmpObj->load(XML, i);
                }
                tmpObj->setObjectId(i);
                Lines.push_back(tmpObj);
            }
        }
        XML.popTag();
    }

}

//void ofxLedController::guiEvent(ofParameterGroup &e){
//
//    if (e.widget->getName() == "h") {
//        region.height = ((ofxUINumberDialer*)e.widget)->getValue();
//    }
//    else if (e.widget->getName() == "x") {
//        float x_diff = region.x;
//        region.x = ((ofxUINumberDialer*)e.widget)->getValue();
//        x_diff -= region.x;
//        //        region.height = ((ofxUINumberDialer*)e.widget)->getValue();
//        for (int i=0;i<Lines.size(); i++) {
//            if (i % 2 == 0) {
//                Lines[i].fromX -= x_diff;
//                Lines[i].toX -= x_diff;
//            }
//            else {
//                Lines[i].toX -= x_diff;
//                Lines[i].fromX -= x_diff;
//            };
//        }
//    }
//    else if (e.widget->getName() == "y") {
//        float y_diff = region.y;
//        region.y = ((ofxUINumberDialer*)e.widget)->getValue();
//        y_diff -= region.y;
//        //        region.height = ((ofxUINumberDialer*)e.widget)->getValue();
//        for (int i=0;i<Lines.size(); i++) {
//            if (i % 2 == 0) {
//                Lines[i].fromY -= y_diff;
//                Lines[i].toY -= y_diff;
//            }
//            else {
//                Lines[i].toY -= y_diff;
//                Lines[i].fromY -= y_diff;
//            };
//        }
//    }
//    else if (e.widget->getName() == "w") {
//        region.width = ((ofxUINumberDialer*)e.widget)->getValue();
//        for (int i=0;i<Lines.size(); i++) {
//            //            Lines[i].fromX = region.x+(region.width/(LINES_NUM+1))*(i+1);
//            //            Lines[i].toX = region.x+(region.width/(LINES_NUM+1))*(i+1);
//        }
//    }
//    else if (e.widget->getName() =="offBeg") {
//        offsetBegin = ((ofxUINumberDialer*)e.widget)->getValue();
//    }
//    else if (e.widget->getName() =="offEnd") {
//        offsetEnd = ((ofxUINumberDialer*)e.widget)->getValue();
//    }
//    else if (e.widget->getName() =="send") {
//        if (((ofxUIToggle*)e.widget)->getValue())
//            setupUdp(udpHost, udpPort);
//    }
//    else if (e.widget->getName() =="LedType") {
//        ledType = static_cast<int>(((ofxUINumberDialer*)e.widget)->getValue());
//    }
//
//}

