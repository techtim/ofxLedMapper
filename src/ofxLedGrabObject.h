//
//  ofxLedGrabObject.h
//  ledMapper
//
//  Created by Tim TVL on 01/09/15.
//
//

#ifndef __ledMapper__ofxLedGrabObject__
#define __ledMapper__ofxLedGrabObject__

#include "ofMain.h"

#define POINT_RAD 3

enum {
    GRAB_OBJ,
    GRAB_LINE,
    GRAB_CIRCLE
};

class ofxLedGrabObject {
    static const int type = GRAB_OBJ;
public:
//    virtual ~ofxLedGrabObject() = 0;
    virtual void updatePoints() = 0;
    virtual void draw() = 0;
    virtual bool mousePressed(ofMouseEventArgs & args) = 0;
    virtual bool mouseDragged(ofMouseEventArgs & args) = 0;
    virtual bool mouseReleased(ofMouseEventArgs & args) = 0;
    virtual void save(ofxXmlSettings & xml) = 0;
    virtual void load(ofxXmlSettings & xml, int tagNum) = 0;
    virtual void set(int _fromX, int _fromY, int _toX, int _toY) {
        fromX = _fromX; fromY = _fromY;
        toX = _toX; toY = _toY;
        updatePoints();
    };
    virtual void set(ofVec2f _from, ofVec2f _to){
        fromX = _from.x; fromY = _from.y;
        toX = _to.x; toY = _to.y;
        updatePoints();

    };
    
    virtual void setFrom(int _fromX, int _fromY) {
        fromX = _fromX; fromY = _fromY;
        updatePoints();
    }
    virtual void setTo(int _toX, int _toY) {
        toX = _toX; toY = _toY;
        updatePoints();
    }
    
    virtual ofVec2f getFrom() {
        return ofVec2f(fromX, fromY);
    }

    virtual ofVec2f getTo() {
        return ofVec2f(toX, toY);
    }
    
    void setPixelsInLed(float _pixs){
        pixelsInLed = _pixs;
        updatePoints();
        
    };
    
    void setObjectId(unsigned int _objID) { objID=_objID; };
    unsigned int getObjectId() const {return objID;};
    
    virtual void setStartAngle(float angle) {
        startAngle = angle;
    }
    
    int getType() {return type;};
    
    float pixelsInLed;
    float startAngle;
    vector<ofVec2f> points;
    
    int fromX;
    int fromY;
    int toX;
    int toY;
    int pixelsInObject;
    bool bSelected, bSelectedFrom, bSelectedTo, bDoubleLine;
    bool lineClicked;
    unsigned int objID;
};
//ofxLedGrabObject::~ofxLedGrabObject(){points.clear();};

class ofxLedGrabLine: public ofxLedGrabObject {
    static const int type = GRAB_LINE;
public:
    ofxLedGrabLine(int _fromX=0, int _fromY=0, int _toX=0, int _toY=0, float pixInLed = 2.f, bool _bDouble=false){
        fromX = _fromX; fromY = _fromY;
        toX = _toX; toY = _toY;
        pixelsInLed = pixInLed;
        bDoubleLine = _bDouble;
        lineClicked = bSelected =  bSelectedFrom = bSelectedTo = bDoubleLine= false;
        updatePoints();
    }
    ofxLedGrabLine(const ofxLedGrabObject & _line){
        fromX = _line.fromX; fromY = _line.fromY;
        toX = _line.toX; toY = _line.toY;
        pixelsInLed = _line.pixelsInLed;
        bDoubleLine = _line.bDoubleLine;
        lineClicked = bSelected =  bSelectedFrom = bSelectedTo = false;
        lineClicked = false;
        updatePoints();
    }

    ~ofxLedGrabLine() {
        points.clear();
    };
    
    bool mousePressed(ofMouseEventArgs & args){
        int x=args.x, y=args.y;
        if (x>fromX-POINT_RAD*2 && x<fromX+POINT_RAD*2 &&
            y>fromY-POINT_RAD*2 && y<fromY+POINT_RAD*2) {
            //                if (currentLine == -1 || currentLine == i) {
            lineClicked = true;
            bSelectedFrom = true;
            bSelectedTo = false;
            return true;
        } else if (x>toX-POINT_RAD*2 && x<toX+POINT_RAD*2 &&
                   y>toY-POINT_RAD*2 && y<toY+POINT_RAD*2) {
            lineClicked = true;
            bSelectedFrom = false;
            bSelectedTo = true;
            return true;
        }
        return false;
    }
    
    bool mouseDragged(ofMouseEventArgs & args) {
        int x=args.x, y=args.y;
        if (bSelectedFrom) {
            //                if (currentLine == -1 || currentLine == i) {
            lineClicked = true;
            setFrom(x, y);
            return true;
        } else if (bSelectedTo) {
            lineClicked = true;
            setTo(x, y);
            return true;
        }
        return false;
    }
    bool mouseReleased(ofMouseEventArgs & args) {
        bSelectedFrom = bSelectedTo = false;
        return true;
    }
    
    void draw() {
        ofSetColor(200, 200, 200,200);
        ofSetColor(200, 200, 200, 150);
        ofDrawLine(fromX, fromY, toX, toY);
        if (lineClicked) {
            for(vector<ofVec2f>::iterator  i = points.begin(); i != points.end(); i ++ ){
                ofDrawCircle((*i), pixelsInLed/2);
            }
        }
        ofDrawBitmapString(ofToString(static_cast<int>(points.size())), ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY),.5));
        ofSetColor(0, 250, 150, 250);
        ofDrawBitmapString("id"+ofToString(objID), fromX, fromY);
    }
    
    void updatePoints() {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
//        if (bSelected) {
            float dist = vert1.distance(vert2);
            pixelsInObject = static_cast<int>(dist/pixelsInLed);
            float lineOffset = (dist-pixelsInObject*static_cast<float>(pixelsInLed))/2.f; // for centering leds in line
            points.clear();
            for (int pix_num=0;pix_num<pixelsInObject; pix_num++) {
                float tmp_num = static_cast<float>(pix_num);
                float step = static_cast<float>(tmp_num/pixelsInObject);
                points.push_back(vert1.getInterpolated(vert2, step));
            }
//        if (bDoubleLine) {
//            for (int pix_num=0;pix_num<pixelsInObject; pix_num++) {
//                float tmp_num = static_cast<float>(pix_num);
//                float step = static_cast<float>(tmp_num/pixelsInObject);
//                points.push_back(vert2.getInterpolated(vert1, step));
//            }
//        }
//        }
    }
    void save(ofxXmlSettings & XML){

        //now we will add a pt tag - with two
        //children - X and Y
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", type, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.popTag();
    }
    void load(ofxXmlSettings & xml, int tagNum){
        
    }
    
};


class ofxLedGrabCircle: public ofxLedGrabObject {
    static const int type = GRAB_CIRCLE;
public:

    ofxLedGrabCircle(int _fromX=0, int _fromY=0, int _toX=0, int _toY=0){
        fromX = _fromX; fromY = _fromY; toX = _toX; toY = _toY;
        lineClicked = false;
        pixelsInLed = 5.f;
        startAngle = -90.f;
        lineClicked = bSelected =  bSelectedFrom = bSelectedTo = false;
        bClockwise = true;
        updatePoints();
    }
    ~ofxLedGrabCircle() { points.clear(); };

    bool mousePressed(ofMouseEventArgs & args){
        int x=args.x, y=args.y;
        if (x>fromX-POINT_RAD && x<fromX+POINT_RAD &&
            y>fromY-POINT_RAD && y<fromY+POINT_RAD) {
            //                if (currentLine == -1 || currentLine == i) {
            lineClicked = true;
            bSelectedFrom = true;
            bSelectedTo = false;
            return true;
        } else if (x>fromX-radius-POINT_RAD && x<fromX+radius+POINT_RAD &&
                   y>fromY-radius-POINT_RAD && y<fromY+radius+POINT_RAD) {
            bSelectedFrom = false;
            bSelectedTo = true;
            lineClicked = true;
            return true;
        }
        return false;
    }
    
    bool mouseDragged(ofMouseEventArgs & args) {
        int x=args.x, y=args.y;
        if (bSelectedFrom) {
            //                if (currentLine == -1 || currentLine == i) {
            lineClicked = true;
            set(x, y, toX+x-fromX, toY+y-fromY);
            return true;
        } else if (bSelectedTo) {
            lineClicked = true;
            setTo(x, y);
            return true;
        }
        return false;
    }
    
    bool mouseReleased(ofMouseEventArgs & args) {
        bSelectedFrom = bSelectedTo = false;
        return true;
    }
    
    void draw() {
        ofSetColor(200, 200, 200,200);
        ofFill();
        ofDrawCircle(fromX, fromY, 3);
        ofNoFill();
        ofDrawCircle(fromX, fromY, radius);
        ofSetColor(200, 200, 200, 250);
        ofFill();
        for(vector<ofVec2f>::iterator  i = points.begin(); i != points.end(); i ++ ){
            ofDrawCircle((*i), pixelsInLed/2);
            ofSetColor(200, 200, 200, 150);
        }
        ofDrawBitmapString(ofToString(static_cast<int>(points.size())), ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY),.5));
        ofSetColor(0, 250, 150, 250);
        ofDrawBitmapString("id"+ofToString(objID), fromX, fromY);

    }

    void setStartAngle(float angle){
        startAngle = angle;
    }
    void setClockwise(bool bClock) {
        bClockwise = bClock;
    }
    void updatePoints() {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        radius = vert1.distance(vert2);
        float dist = radius*TWO_PI;
        int pixelsInLine = static_cast<int>(dist/pixelsInLed);
        float degreeStep = 360.f/static_cast<float>(pixelsInLine);
        float currStep = startAngle;
        points.clear();
        for (int i=0; i<pixelsInLine; i++) {
//            float x = sin(currStep*PI/180);
//            float y = cos(currStep*PI/180);
            ofVec2f tmp = ofVec2f(cos(currStep*PI/180), sin(currStep*PI/180))*radius;
            tmp += vert1;
            points.push_back(tmp);
            bClockwise? currStep-=degreeStep : currStep+=degreeStep;
        }
    }

    void save(ofxXmlSettings & XML){
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", type, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.setValue("LN:startAngle", startAngle, tagNum);
        XML.setValue("LN:isClockwise", bClockwise, tagNum);
        XML.popTag();
    }
    
    void load(ofxXmlSettings & xml, int tagNum){
        setStartAngle(xml.getValue("LN:startAngle", -90, tagNum));
        setClockwise(xml.getValue("LN:isClockwise", true, tagNum));
        updatePoints();
    }

    
    float radius;
    float startAngle;
    bool bClockwise;

};

#endif /* defined(__ledMapper__ofxLedGrabObject__) */
