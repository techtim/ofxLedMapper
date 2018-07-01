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

#pragma once

#include "ofMain.h"

#include "Common.h"

namespace LedMapper {

class ofxLedGrabLine;
class ofxLedGrabCircle;
class ofxLedGrabMatrix;

enum LMGrabType {GRAB_EMPTY, GRAB_LINE, GRAB_CIRCLE, GRAB_MATRIX, GRAB_SELECT };

class ofxLedGrabObject {
public:

    ofxLedGrabObject(int _fromX = 0, int _fromY = 0, int _toX = 0, int _toY = 0,
                     float pixInLed = 2.f)
        : fromX(_fromX)
        , fromY(_fromY)
        , toX(_toX)
        , toY(_toY)
        , m_bActive(false)
        , m_bSelected(false)
        , m_bSelectedFrom(false)
        , m_bSelectedTo(false)
        , m_pixelsInLed(pixInLed)
    {
    }

    virtual ~ofxLedGrabObject(){};

    virtual void updatePoints() = 0;
    virtual void draw(const ofColor &color = ofColor(200, 200, 200, 150)) = 0;
    virtual void drawGui() = 0;
    virtual void updateBounds() = 0;
    virtual bool mousePressed(const ofMouseEventArgs &args) = 0;
    virtual bool mouseDragged(const ofMouseEventArgs &args) = 0;
    virtual bool mouseReleased(const ofMouseEventArgs &args) = 0;
    virtual void save(ofxXmlSettings &xml) = 0;
    virtual void load(ofxXmlSettings &xml, int tagNum) = 0;
    virtual void set(int _fromX, int _fromY, int _toX, int _toY)
    {
        if (_fromX >= 0 && _fromY >= 0 && _toX >= 0 && _toY >= 0) {
            fromX = _fromX;
            fromY = _fromY;
            toX = _toX;
            toY = _toY;
            updatePoints();
        }
    };
    virtual void set(ofVec2f _from, ofVec2f _to) { set(_from.x, _from.y, _to.x, _to.y); };

    virtual void setFrom(int _fromX, int _fromY)
    {
        fromX = _fromX;
        fromY = _fromY;
        updatePoints();
    }
    virtual void setTo(int _toX, int _toY)
    {
        toX = _toX;
        toY = _toY;
        updatePoints();
    }

    virtual ofVec2f getFrom() const { return ofVec2f(fromX, fromY); }

    virtual ofVec2f getTo() const { return ofVec2f(toX, toY); }

    void setPixelsInLed(float _pixs)
    {
        m_pixelsInLed = _pixs;
        updatePoints();
    };

    bool pressedFromTo(const ofVec2f &args)
    {
        if (abs(args.x - fromX) <= POINT_RAD && abs(args.y - fromY) <= POINT_RAD) {
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
        }
        else if (abs(args.x - toX) <= POINT_RAD && abs(args.y - toY) <= POINT_RAD) {
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
        }
    }
    void setChannel(int _channel) { m_channel = _channel; }
    int getChannel() const { return m_channel; }
    void setActive(bool active) { m_bActive = active; }
    bool isActive() const { return m_bActive; }
    void setSelected(bool selected) { m_bSelected = selected; }
    bool isSelected() const { return m_bSelected; }

    void setObjectId(unsigned int _objID) { objID = _objID; };
    unsigned int getObjectId() const { return objID; };

    virtual void setStartAngle(float angle) { startAngle = angle; }

    int getType() const { return m_type; };
    const ofRectangle &getBounds() { return m_bounds; }
    const vector<ofVec2f> &points() const { return m_points; }

    vector<LedMapper::Point> getLedPoints()
    {
        vector<LedMapper::Point> ledPoints;
        ledPoints.reserve(m_points.size());
        for_each(m_points.begin(), m_points.end(), [&ledPoints](const ofVec2f &point) {
            ledPoints.push_back({ static_cast<uint16_t>(point.x), static_cast<uint16_t>(point.y) });
        });

        return ledPoints;
    }

private:
    unsigned int objID;

    int fromX, fromY, toX, toY;
    bool m_bActive, m_bSelected, m_bSelectedFrom, m_bSelectedTo;

    float m_pixelsInLed;
    ofVec2f m_clickedPos;
    float startAngle;
    int m_channel, m_pixelsInObject;

    vector<ofVec2f> m_points;
    ofRectangle m_bounds;

    static const int m_type = LMGrabType::GRAB_EMPTY;

#ifndef LED_MAPPER_NO_GUI
    unique_ptr<ofxDatGui> gui;
#endif

    friend class ofxLedGrabLine;
    friend class ofxLedGrabCircle;
    friend class ofxLedGrabMatrix;
    friend std::ostream &operator<<(std::ostream &os, const ofxLedGrabObject &obj);
};

inline std::ostream &operator<<(std::ostream &os, const ofxLedGrabObject &obj)
{
    os << "Grab obj with type=" << ofToString(obj.m_type) << " from=" << ofVec2f(obj.fromX, obj.fromY)
       << " to=" << ofVec2f(obj.toX, obj.toY);
    return os;
}

class ofxLedGrabLine : public ofxLedGrabObject {
    static const int m_type = LMGrabType::GRAB_LINE;
    bool m_bDoubleLine;

public:
    ofxLedGrabLine(int _fromX = 0, int _fromY = 0, int _toX = 0, int _toY = 0, float pixInLed = 2.f,
                   bool _bDouble = false)
        : ofxLedGrabObject(_fromX, _fromY, _toX, _toY, pixInLed)
        , m_bDoubleLine(_bDouble)
    {
        updatePoints();
    }

    ofxLedGrabLine(const ofxLedGrabLine &_line)
    {
        fromX = _line.fromX;
        fromY = _line.fromY;
        toX = _line.toX;
        toY = _line.toY;
        m_pixelsInLed = _line.m_pixelsInLed;
        m_bDoubleLine = _line.m_bDoubleLine;
        m_bSelectedFrom = false;
        m_bSelectedTo = false;
        m_bSelected = false;
        updatePoints();
    }

    void setupGui()
    {
#ifndef LED_MAPPER_NO_GUI
        gui = make_unique<ofxDatGui>(ofxDatGuiAnchor::TOP_RIGHT);
        gui->setAssetPath("");
        gui->setWidth(LM_GUI_WIDTH);

        //    ofxDatGuiFolder* folder = gui->addFolder("parameters", ofColor::white);
        ofxDatGuiSlider *sInput;
        sInput = gui->addSlider("pixel step", 1, 20);
        sInput->bind(m_pixelsInLed);

        //        sInput = gui->addToggle("double line", m_bDoubleLine);
        ofxDatGuiToggle *tInput = gui->addToggle("double line", false);
        tInput->bind(m_bDoubleLine);

        sInput = gui->addSlider("from X", 0, 1000);
        sInput->bind(fromX);

        sInput = gui->addSlider("from Y", 0, 1000);
        sInput->bind(fromY);

        sInput = gui->addSlider("to X", 0, 2000);
        sInput->bind(toX);
        sInput = gui->addSlider("to Y", 0, 2000);
        sInput->bind(toY);
#endif
    }

    ~ofxLedGrabLine(){};

    bool mousePressed(const ofMouseEventArgs &args) override
    {
        int x = args.x, y = args.y;
        if (m_bounds.inside(x, y)) {
            m_bSelected = true;
            m_clickedPos = ofVec2f(x, y);
            pressedFromTo(args);
            return true;
        }
        m_bSelected = false;
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args) override
    {
        int x = args.x, y = args.y;
        if (m_bSelectedFrom) {
            //            m_bSelected = true;
            ofxLedGrabObject::setFrom(x, y);
            return true;
        }
        else if (m_bSelectedTo) {
            //            m_bSelected = true;
            ofxLedGrabObject::setTo(x, y);
        }
        else if (m_bSelected) {
            ofVec2f dist(ofVec2f(x, y) - m_clickedPos);
            m_clickedPos = ofVec2f(x, y);
            ofxLedGrabObject::setFrom(fromX + dist.x, fromY + dist.y);
            ofxLedGrabObject::setTo(toX + dist.x, toY + dist.y);
            return true;
        }
        return false;
    }
    bool mouseReleased(const ofMouseEventArgs &args) override
    {
        m_bSelectedFrom = m_bSelectedTo = false;
        return true;
    }

    void draw(const ofColor &color = ofColor(200, 200, 200, 150)) override
    {
        ofSetColor(color);
        ofDrawLine(fromX, fromY, toX, toY);
        if (m_bSelected || isActive()) {
            for (vector<ofVec2f>::iterator i = m_points.begin(); i != m_points.end(); i++) {
                ofDrawCircle((*i), m_pixelsInLed / 2);
            }
            ofNoFill();
            ofDrawRectangle(m_bounds);
            ofFill();
            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(objID), fromX, fromY);
            ofSetColor(107, 230, 180, 250);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY), .5));
        }
    }

    void drawGui() override { ; }

    void updatePoints() override
    {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        updateBounds();
        float dist = vert1.distance(vert2);
        m_pixelsInObject = static_cast<int>(dist / m_pixelsInLed);
        m_points.clear();
        m_points.reserve(m_pixelsInObject);
        for (int pix_num = 0; pix_num < m_pixelsInObject; pix_num++) {
            float tmp_num = static_cast<float>(pix_num) + .5f;
            float step = static_cast<float>(tmp_num / m_pixelsInObject);
            m_points.push_back(vert1.getInterpolated(vert2, step));
        }

        if (m_bDoubleLine) {
            auto tmpPoints = m_points;
            std::reverse(tmpPoints.begin(), tmpPoints.end());
            m_points.insert(m_points.end(), tmpPoints.begin(), tmpPoints.end());
        }
    }
    void updateBounds() override
    {
        auto min = ofVec2f(MIN(fromX, toX), MIN(fromY, toY));
        auto max = ofVec2f(MAX(fromX, toX), MAX(fromY, toY));
        m_bounds.set(min - ofVec2f(POINT_RAD), max + ofVec2f(POINT_RAD));
    }

    void save(ofxXmlSettings &XML) override
    {
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", m_type, tagNum);
        XML.setValue("LN:CHANNEL", m_channel, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.setValue("LN:bDouble", m_bDoubleLine, tagNum);
        XML.popTag();
    }
    void load(ofxXmlSettings &xml, int tagNum) override
    {
        m_bDoubleLine = xml.getValue("LN:bDouble", false, tagNum);
    }
};

class ofxLedGrabCircle : public ofxLedGrabObject {
    static const int m_type = LMGrabType::GRAB_CIRCLE;
    float m_radius;
    float startAngle;
    bool bClockwise;

public:
    ofxLedGrabCircle(int _fromX = 0, int _fromY = 0, int _toX = 0, int _toY = 0,
                     float pixInLed = 2.f)
        : ofxLedGrabObject(_fromX, _fromY, _toX, _toY, pixInLed)
    {
        startAngle = -90.f;
        bClockwise = true;
        updatePoints();
    }
    ~ofxLedGrabCircle()
    {
        ofLogVerbose("[ofxLedGrabCircle] Detor: clear m_points");
        m_points.clear();
    };

    bool mousePressed(const ofMouseEventArgs &args)
    {
        int x = args.x, y = args.y;
        if (m_bounds.inside(x, y)) {
            m_bSelected = true;
            if (x > fromX - POINT_RAD && x < fromX + POINT_RAD && y > fromY - POINT_RAD
                && y < fromY + POINT_RAD) {
                m_bSelectedFrom = true;
                m_bSelectedTo = false;
            }
            else if (x > fromX - m_radius - POINT_RAD && x < fromX + m_radius + POINT_RAD
                     && y > fromY - m_radius - POINT_RAD && y < fromY + m_radius + POINT_RAD) {
                m_bSelectedFrom = false;
                m_bSelectedTo = true;
            }
            return true;
        }
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args)
    {
        int x = args.x, y = args.y;
        if (m_bSelectedFrom) {
            m_bSelected = true;
            ofxLedGrabObject::set(x, y, toX + x - fromX, toY + y - fromY);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrabObject::setTo(x, y);
            return true;
        }
        return false;
    }

    bool mouseReleased(const ofMouseEventArgs &args) override
    {
        m_bSelectedFrom = m_bSelectedTo = false;
        return true;
    }

    void draw(const ofColor &color = ofColor(200, 200, 200, 150)) override
    {
        ofSetColor(color);
        ofFill();
        ofDrawCircle(fromX, fromY, POINT_RAD);
        ofNoFill();
        ofDrawCircle(fromX, fromY, m_radius);
        ofSetColor(200, 200, 200, 250);
        ofFill();
        if (m_bSelected || isActive()) {
            for (vector<ofVec2f>::iterator i = m_points.begin(); i != m_points.end(); i++) {
                ofDrawCircle((*i), m_pixelsInLed / 2);
                ofSetColor(200, 200, 200, 150);
            }
            ofSetColor(100, 100, 100, 150);
            ofDrawRectangle(m_bounds);
            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(objID), fromX, fromY);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY), .5));
        }
    }

    void drawGui() override
    {
        ;
    }

    void setStartAngle(float angle) override { startAngle = angle; }
    void setClockwise(bool bClock) { bClockwise = bClock; }
    void updatePoints() override
    {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        m_radius = vert1.distance(vert2);
        updateBounds();
        float dist = m_radius * TWO_PI;
        int pixelsInLine = static_cast<int>(dist / m_pixelsInLed);
        float degreeStep = 360.f / static_cast<float>(pixelsInLine);
        float currStep = startAngle;
        m_points.clear();
        m_points.reserve(pixelsInLine);
        for (int i = 0; i < pixelsInLine; i++) {
            ofVec2f tmp = ofVec2f(cos(currStep * PI / 180), sin(currStep * PI / 180)) * m_radius;
            tmp += vert1;
            // check if point is on the screen
            if (tmp.x >= 0 && tmp.y >= 0)
                m_points.push_back(tmp);
            bClockwise ? currStep -= degreeStep : currStep += degreeStep;
        }
    }
    void updateBounds() override
    {
        m_bounds.setPosition(fromX - m_radius - POINT_RAD, fromY - m_radius - POINT_RAD);
        m_bounds.setSize((m_radius + POINT_RAD) * 2.f, (m_radius + POINT_RAD) * 2.f);
    }

    void save(ofxXmlSettings &XML)
    {
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", m_type, tagNum);
        XML.setValue("LN:CHANNEL", m_channel, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.setValue("LN:startAngle", startAngle, tagNum);
        XML.setValue("LN:isClockwise", bClockwise, tagNum);
        XML.popTag();
    }

    void load(ofxXmlSettings &xml, int tagNum)
    {
        setStartAngle(xml.getValue("LN:startAngle", -90, tagNum));
        setClockwise(xml.getValue("LN:isClockwise", true, tagNum));
        updatePoints();
    }
};

class ofxLedGrabMatrix : public ofxLedGrabObject {
    static const int m_type = LMGrabType::GRAB_MATRIX;
    int m_columns, m_rows;
    bool m_isVertical, m_isZigzag;
    float startAngle;

public:
    ofxLedGrabMatrix(int _fromX = 0, int _fromY = 0, int _toX = 0, int _toY = 0,
                     float pixInLed = 2.f, bool isVertical = true, bool isZigzag = true)
        : ofxLedGrabObject(_fromX, _fromY, _toX, _toY, pixInLed)
    {
        m_isVertical = isVertical;
        m_isZigzag = isZigzag;
        updatePoints();
    }

    ~ofxLedGrabMatrix()
    {
        ofLogVerbose("[ofxLedGrabCircle] Detor: clear m_points");
        m_points.clear();
    }

    bool mousePressed(const ofMouseEventArgs &args)
    {
        int x = args.x, y = args.y;
        if (m_bounds.inside(x, y)) {
            m_bSelected = true;
            m_clickedPos = ofVec2f(x, y);
            pressedFromTo(args);
            return true;
        }
        m_bSelected = false;
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args)
    {
        int x = args.x, y = args.y;
        if (m_bSelectedFrom) {
            m_bSelected = true;
            ofxLedGrabObject::setFrom(x, y);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrabObject::setTo(x, y);
            return true;
        }
        else if (m_bSelected) {
            ofVec2f dist(ofVec2f(x, y) - m_clickedPos);
            m_clickedPos = ofVec2f(x, y);
            ofxLedGrabObject::setFrom(fromX + dist.x, fromY + dist.y);
            ofxLedGrabObject::setTo(toX + dist.x, toY + dist.y);
            return true;
        }

        return false;
    }

    bool mouseReleased(const ofMouseEventArgs &args)
    {
        m_bSelectedFrom = m_bSelectedTo = false;
        return true;
    }

    void draw(const ofColor &color = ofColor(100, 100, 100, 150)) override
    {
        ofSetColor(0, 191, 165, 200);
        ofFill();
        ofDrawCircle(fromX, fromY, 3);
        ofDrawCircle(toX, toY, 3);

        ofNoFill();
        if (m_bSelected || isActive()) {
            ofSetColor(color);
            ofFill();
            //            unsigned ctr=0;
            for (auto &it : m_points) {
                ofDrawCircle(it, m_pixelsInLed / 8);
                //                ofDrawBitmapString(ofToString(ctr), it);
                //                ++ctr;
            }
            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(objID), fromX, fromY);

            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("w=" + ofToString(m_isVertical ? m_rows : m_columns),
                               (fromX + toX) * .5, fromY);
            ofDrawBitmapString("h=" + ofToString(m_isVertical ? m_columns : m_rows), fromX,
                               (fromY + toY) * .5);

            ofSetColor(200, 200, 200, 150);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               ofVec2f(fromX, fromY).getInterpolated(ofVec2f(toX, toY), .5));
        }
        ofSetColor(100, 100, 100, 100);
        ofDrawRectangle(m_bounds);
    }

    void drawGui() override
    {
        ;
        ;
    }

    void setNumRows(int rows) { m_rows = rows; }
    void setNumColumns(int columns) { m_columns = columns; }

    void updatePoints()
    {
        ofVec2f vert1(fromX, fromY);
        ofVec2f vert2(toX, toY);
        updateBounds();
        //        m_columns = static_cast<int>(m_isVertical? abs(vert1.y - vert2.y) / m_pixelsInLed
        //        : abs(vert1.x - vert2.x) / m_pixelsInLed);
        //        m_rows = static_cast<int>(m_isVertical? abs(vert1.x - vert2.x) / m_pixelsInLed :
        //        abs(vert1.y - vert2.y) / m_pixelsInLed);
        m_columns = static_cast<int>(m_isVertical ? abs(vert1.y - vert2.y) / m_pixelsInLed
                                                  : abs(vert1.x - vert2.x) / m_pixelsInLed);
        m_rows = static_cast<int>(m_isVertical ? abs(vert1.x - vert2.x) / m_pixelsInLed
                                               : abs(vert1.y - vert2.y) / m_pixelsInLed);

        m_pixelsInObject = m_columns * m_rows;

        m_points.clear();
        m_points.reserve(m_pixelsInObject);

        if (m_isVertical) {
            for (int row = 0; row < m_rows; ++row) {
                ofVec2f rowPos = vert1.getInterpolated(ofVec2f(vert2.x, vert1.y),
                                                       (static_cast<float>(row) + .5f) / m_rows);
                // ofLogVerbose("row/m_rows =" + ofToString(static_cast<float>(row + 1)/m_rows ) + "
                // -> " + ofToString(rowPos));
                int cntr, maxCntr, increment;
                // on zigzag start not %2 from opposite side
                if (m_isZigzag && row % 2 == 1) {
                    cntr = m_columns - 1;
                    maxCntr = -1;
                    increment = -1;
                }
                else {
                    cntr = 0;
                    maxCntr = m_columns;
                    increment = 1;
                }

                while (cntr != maxCntr) {
                    ofVec2f tmp = rowPos.getInterpolated(
                        ofVec2f(rowPos.x, vert2.y), (static_cast<float>(cntr) + .5f) / m_columns);
                    if (tmp.x >= 0 && tmp.y >= 0)
                        m_points.emplace_back(std::move(tmp));
                    cntr += increment;
                }
            }
        }
        else {
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
    void updateBounds() override
    {
        auto min = ofVec2f(MIN(fromX, toX), MIN(fromY, toY));
        auto max = ofVec2f(MAX(fromX, toX), MAX(fromY, toY));
        m_bounds.set(min - ofVec2f(POINT_RAD), max + ofVec2f(POINT_RAD));
    }

    void save(ofxXmlSettings &XML)
    {
        int tagNum = XML.addTag("LN");
        XML.setValue("LN:TYPE", m_type, tagNum);
        XML.setValue("LN:CHANNEL", m_channel, tagNum);
        XML.setValue("LN:fromX", fromX, tagNum);
        XML.setValue("LN:fromY", fromY, tagNum);
        XML.setValue("LN:toX", toX, tagNum);
        XML.setValue("LN:toY", toY, tagNum);
        XML.setValue("LN:rows", m_rows, tagNum);
        XML.setValue("LN:columns", m_columns, tagNum);
        XML.setValue("LN:isVertical", m_isVertical, tagNum);
        XML.setValue("LN:isZigzag", m_isZigzag, tagNum);
        XML.popTag();
    }

    void load(ofxXmlSettings &xml, int tagNum)
    {
        setNumRows(xml.getValue("LN:numRows", 1, tagNum));
        setNumColumns(xml.getValue("LN:numColumns", 1, tagNum));
        m_isVertical = xml.getValue("LN:isVertical", true, tagNum);
        m_isZigzag = xml.getValue("LN:isZigzag", true, tagNum);
        updatePoints();
    }
};

} // namespace LedMapper