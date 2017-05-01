//
//  ofxLedGrabObject.h
//  ledMapper
//
//  Created by Tim TVL on 01/09/15.
//
//

#pragma once

#include "ofMain.h"

#define POINT_RAD 3

class ofxLedGrabLine;
class ofxLedGrabCircle;
class ofxLedGrabMatrix;

class ofxLedGrabObject {
public:

    enum GRAB_TYPE {
        GRAB_EMPTY,
        GRAB_LINE,
        GRAB_CIRCLE,
        GRAB_MATRIX
    };
    
    virtual ~ofxLedGrabObject() {;;};
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
        m_pixelsInLed = _pixs;
        updatePoints();
    };
    
    void setObjectId(unsigned int _objID) { objID=_objID; };
    unsigned int getObjectId() const {return objID;};
    
    virtual void setStartAngle(float angle) {
        startAngle = angle;
    }
    
    int getType() {return type;};
    
    vector<ofVec2f> m_points;
    
    unsigned int objID;
private:
    int fromX;
    int fromY;
    int toX;
    int toY;
    float m_pixelsInLed;
    float startAngle;
    int m_pixelsInObject;
    
    bool m_bSelected, m_bSelectedFrom, m_bSelectedTo, bDoubleLine;
    bool m_lineClicked;
    
    static const int type = GRAB_EMPTY;
    
    friend class ofxLedGrabLine;
    friend class ofxLedGrabCircle;
    friend class ofxLedGrabMatrix;
    
};


class ofxLedGrabLine: public ofxLedGrabObject {
    bool m_bSelected, m_bSelectedFrom, m_bSelectedTo, m_bDoubleLine;
    bool m_lineClicked;
    
    static const int type = GRAB_LINE;
public:
    ofxLedGrabLine(int _fromX=0, int _fromY=0, int _toX=0, int _toY=0, float pixInLed = 2.f, bool _bDouble=false)
    {
        fromX = _fromX;
        fromY = _fromY;
        toX = _toX;
        toY = _toY;
        m_pixelsInLed = pixInLed;
        m_bSelected = m_bSelectedFrom = m_bSelectedTo = m_bDoubleLine = m_lineClicked = false;
        updatePoints();
    }
    
    ofxLedGrabLine(const ofxLedGrabObject & _line){
        fromX = _line.fromX; fromY = _line.fromY;
        toX = _line.toX; toY = _line.toY;
        m_pixelsInLed = _line.m_pixelsInLed;
        bDoubleLine = _line.bDoubleLine;
        m_lineClicked = m_bSelected =  m_bSelectedFrom = m_bSelectedTo = false;
        m_lineClicked = false;
        updatePoints();
    }
    
    ~ofxLedGrabLine() {
        ofLogVerbose("[ofxLedGrabLine] Detor: clear m_points");
        m_points.clear();
    };
    
    bool mousePressed(ofMouseEventArgs & args){
        int x=args.x, y=args.y;
        if (x>fromX-POINT_RAD*2 && x<fromX+POINT_RAD*2 &&
            y>fromY-POINT_RAD*2 && y<fromY+POINT_RAD*2)
        {
            m_lineClicked = true;
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
            return true;
        } else if (x>toX-POINT_RAD*2 && x<toX+POINT_RAD*2 &&
                   y>toY-POINT_RAD*2 && y<toY+POINT_RAD*2)
        {
            m_lineClicked = true;
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
            return true;
        }
        return false;
    }
    
    bool mouseDragged(ofMouseEventArgs & args) {
        int x=args.x, y=args.y;
        if (m_bSelectedFrom) {
            m_lineClicked = true;
            setFrom(x, y);
            return true;
        } else if (m_bSelectedTo) {
            m_lineClicked = true;
            setTo(x, y);
            return true;
        }
        return false;
    }
    bool mouseReleased(ofMouseEventArgs & args) {
        m_bSelectedFrom = m_bSelectedTo = false;
        return true;
    }
    
    void draw() {
        ofSetColor(200, 200, 200,200);
        ofSetColor(200, 200, 200, 150);
        ofDrawLine(fromX, fromY, toX, toY);
        if (m_lineClicked) {
            for(vector<ofVec2f>::iterator  i = m_points.begin(); i != m_points.end(); i ++ ){
                ofDrawCircle((*i), m_pixelsInLed/2);
            }
        }
        ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())), ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY),.5));
        ofSetColor(0, 250, 150, 250);
        ofDrawBitmapString("id"+ofToString(objID), fromX, fromY);
    }
    
    void updatePoints() {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        
        float dist = vert1.distance(vert2);
        m_pixelsInObject = static_cast<int>(dist/m_pixelsInLed);
        float lineOffset = (dist-m_pixelsInObject*static_cast<float>(m_pixelsInLed))/2.f; // for centering leds in line
        m_points.clear();
        m_points.reserve(m_pixelsInObject);
        for (int pix_num=0;pix_num<m_pixelsInObject; pix_num++) {
            float tmp_num = static_cast<float>(pix_num);
            float step = static_cast<float>(tmp_num/m_pixelsInObject);
            m_points.push_back(vert1.getInterpolated(vert2, step));
        }
        //        TODO:
        //        if (bDoubleLine) {
        //            for (int pix_num=0;pix_num<m_pixelsInObject; pix_num++) {
        //                float tmp_num = static_cast<float>(pix_num);
        //                float step = static_cast<float>(tmp_num/m_pixelsInObject);
        //                m_points.push_back(vert2.getInterpolated(vert1, step));
        //            }
        //        }
        
    }
    void save(ofxXmlSettings & XML){
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
    
    ofxLedGrabCircle(int _fromX=0, int _fromY=0, int _toX=0, int _toY=0, float pixInLed = 2.f){
        fromX = _fromX; fromY = _fromY; toX = _toX; toY = _toY;
        m_lineClicked = false;
        m_pixelsInLed = pixInLed;
        startAngle = -90.f;
        m_lineClicked = m_bSelected =  m_bSelectedFrom = m_bSelectedTo = false;
        bClockwise = true;
        updatePoints();
    }
    ~ofxLedGrabCircle() {
        ofLogVerbose("[ofxLedGrabCircle] Detor: clear m_points");
        m_points.clear();
    };
    
    bool mousePressed(ofMouseEventArgs & args){
        int x=args.x, y=args.y;
        if (x>fromX-POINT_RAD && x<fromX+POINT_RAD &&
            y>fromY-POINT_RAD && y<fromY+POINT_RAD)
        {
            m_lineClicked = true;
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
            return true;
        } else if (x>fromX-radius-POINT_RAD && x<fromX+radius+POINT_RAD &&
                   y>fromY-radius-POINT_RAD && y<fromY+radius+POINT_RAD)
        {
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
            m_lineClicked = true;
            return true;
        }
        return false;
    }
    
    bool mouseDragged(ofMouseEventArgs & args) {
        int x=args.x, y=args.y;
        if (m_bSelectedFrom) {
            m_lineClicked = true;
            set(x, y, toX+x-fromX, toY+y-fromY);
            return true;
        } else if (m_bSelectedTo) {
            m_lineClicked = true;
            setTo(x, y);
            return true;
        }
        return false;
    }
    
    bool mouseReleased(ofMouseEventArgs & args) {
        m_bSelectedFrom = m_bSelectedTo = false;
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
        for(vector<ofVec2f>::iterator  i = m_points.begin(); i != m_points.end(); i ++ ){
            ofDrawCircle((*i), m_pixelsInLed/2);
            ofSetColor(200, 200, 200, 150);
        }
        ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())), ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY),.5));
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
        int pixelsInLine = static_cast<int>(dist/m_pixelsInLed);
        float degreeStep = 360.f/static_cast<float>(pixelsInLine);
        float currStep = startAngle;
        m_points.clear();
        m_points.reserve(pixelsInLine);
        for (int i=0; i<pixelsInLine; i++) {
            ofVec2f tmp = ofVec2f(cos(currStep*PI/180), sin(currStep*PI/180))*radius;
            tmp += vert1;
            // check if point is on the screen
            if (tmp.x >= 0 && tmp.y >= 0) m_points.push_back(tmp);
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

class ofxLedGrabMatrix: public ofxLedGrabObject {
    int m_columns, m_rows;
    bool m_isVertical;
    float startAngle;
    static const int type = GRAB_MATRIX;
public:
    ofxLedGrabMatrix(int _fromX=0, int _fromY=0, int _toX=0, int _toY=0, float pixInLed = 2.f, bool isVertical = true){
        fromX = _fromX; fromY = _fromY; toX = _toX; toY = _toY;
        m_lineClicked = false;
        m_pixelsInLed = pixInLed;
        m_isVertical = isVertical;
        startAngle = -90.f;
        m_lineClicked = m_bSelected =  m_bSelectedFrom = m_bSelectedTo = false;
        updatePoints();
    }
    
    ~ofxLedGrabMatrix() {
        ofLogVerbose("[ofxLedGrabCircle] Detor: clear m_points");
        m_points.clear();
    }
    
    bool mousePressed(ofMouseEventArgs & args){
        int x=args.x, y=args.y;
        if (x>fromX-POINT_RAD*2 && x<fromX+POINT_RAD*2 &&
            y>fromY-POINT_RAD*2 && y<fromY+POINT_RAD*2)
        {
            m_lineClicked = true;
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
            return true;
        } else if (x>toX-POINT_RAD*2 && x<toX+POINT_RAD*2 &&
                   y>toY-POINT_RAD*2 && y<toY+POINT_RAD*2)
        {
            m_lineClicked = true;
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
            return true;
        }
        return false;
    }
    
    bool mouseDragged(ofMouseEventArgs & args) {
        int x=args.x, y=args.y;
        if (m_bSelectedFrom) {
            m_lineClicked = true;
            setFrom(x, y);
            return true;
        } else if (m_bSelectedTo) {
            m_lineClicked = true;
            setTo(x, y);
            return true;
        }
        return false;
    }
    
    bool mouseReleased(ofMouseEventArgs & args) {
        m_bSelectedFrom = m_bSelectedTo = false;
        return true;
    }
    
    void draw() {
        ofSetColor(200, 200, 200,250);
        ofFill();
        ofDrawCircle(fromX, fromY, 3);
        ofDrawCircle(toX, toY, 3);

        ofSetColor(200, 200, 200, 200);
        ofFill();
        for(auto &it : m_points){
            ofDrawCircle((it), m_pixelsInLed/8);
            ofSetColor(200, 200, 200, 150);
        }
        ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())), ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY),.5));
        ofSetColor(0, 250, 150, 250);
        ofDrawBitmapString("id"+ofToString(objID), fromX, fromY);

        ofSetColor(0, 250, 150, 250);
        ofDrawBitmapString("w="+ofToString(m_columns), (fromX+toX)*.5, fromY);
        ofDrawBitmapString("h="+ofToString(m_rows), fromX, (fromY+toY)*.5);
    }
    
    void setNumRows(int rows){
        m_rows = rows;
    }
    void setNumColumns(int columns) {
        m_columns = columns;
    }
    
    void updatePoints() {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        
        float dist = vert1.distance(vert2);;
//        m_columns = static_cast<int>(m_isVertical? abs(vert1.y - vert2.y) / m_pixelsInLed : abs(vert1.x - vert2.x) / m_pixelsInLed);
//        m_rows = static_cast<int>(m_isVertical? abs(vert1.x - vert2.x) / m_pixelsInLed : abs(vert1.y - vert2.y) / m_pixelsInLed);
        m_columns = static_cast<int>(abs(vert1.x - vert2.x) / m_pixelsInLed);
        m_rows = static_cast<int>(abs(vert1.y - vert2.y) / m_pixelsInLed);
        
        int pixelsInLine = m_columns * m_rows;
        float degreeStep = 360.f/static_cast<float>(pixelsInLine);
        float currStep = startAngle;
        m_points.clear();
        m_points.reserve(pixelsInLine);
        
        if (m_isVertical) {
            for (int row = 0; row < m_rows; ++row) {
                for (int col = 0; col < m_columns; ++col) {
                    ofVec2f tmp = ofVec2f(row * m_pixelsInLed, col * m_pixelsInLed);
                    tmp += vert1;
                    if (tmp.x >= 0 && tmp.y >= 0)
                        m_points.emplace_back(std::move(tmp));
                }
            }
        } else {
            for (int col = 0; col < m_columns; ++col) {
                for (int row = 0; row < m_rows; ++row) {
                    ofVec2f tmp = ofVec2f(col * m_pixelsInLed, row * m_pixelsInLed);
                    tmp += vert1;
                    if (tmp.x >= 0 && tmp.y >= 0)
                        m_points.emplace_back(std::move(tmp));
                }
            }
        }
        
    }
    
    void save(ofxXmlSettings & XML){
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", type, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.setValue("LN:rows", m_rows, tagNum);
        XML.setValue("LN:columns", m_columns, tagNum);
        XML.setValue("LN:isVertical", m_isVertical, tagNum);
        XML.popTag();
    }
    
    void load(ofxXmlSettings & xml, int tagNum){
        setNumRows(xml.getValue("LN:numRows", 1, tagNum));
        setNumColumns(xml.getValue("LN:numColumns", 1, tagNum));
        m_isVertical = xml.getValue("LN:isVerical", true, tagNum);
        updatePoints();
    }
};


