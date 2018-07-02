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

enum LMGrabType { GRAB_EMPTY, GRAB_LINE, GRAB_CIRCLE, GRAB_MATRIX, GRAB_SELECT };

class ofxLedGrabObject {
public:
    ofxLedGrabObject(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                     float pixInLed = 2.f)
        : m_from(from)
        , m_to(to)
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
    virtual bool pressedFromTo(const ofVec2f &args)
    {
        if (m_from.distance(args) <= POINT_RAD) {
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
        }
        else if (m_to.distance(args) <= POINT_RAD) {
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
        }
    }

    virtual void load(ofxXmlSettings &xml, int tagNum = -1) = 0;
    virtual void save(ofxXmlSettings &xml, const int tagNum) {
        xml.setValue("LN:CHANNEL", m_channel, tagNum);
        xml.setValue("LN:fromX", m_from.x, tagNum);
        xml.setValue("LN:fromY", m_from.y, tagNum);
        xml.setValue("LN:toX", m_to.x, tagNum);
        xml.setValue("LN:toY", m_to.y, tagNum);
    }

    virtual void set(int fromX, int fromY, int toX, int toY)
    {
        if (fromX < 0 || fromY < 0 || toX < 0 || toY < 0)
            return;
        m_from.x = fromX;
        m_from.y = fromY;
        m_to.x = toX;
        m_to.y = toY;
        updatePoints();
    };
    virtual void set(const ofVec2f &from, const ofVec2f &to) { set(from.x, from.y, to.x, to.y); };

    virtual void setFrom(const ofVec2f &from) { set(from, m_to); }
    virtual void setTo(const ofVec2f &to) { set(m_from, to); }
    virtual ofVec2f getFrom() const { return m_from; }
    virtual ofVec2f getTo() const { return m_to; }

    void setPixelsInLed(float _pixs)
    {
        m_pixelsInLed = _pixs;
        updatePoints();
    };

    void setObjectId(unsigned int _objID) { objID = _objID; };
    unsigned int getObjectId() const { return objID; };
    void setChannel(int _channel) { m_channel = _channel; }
    int getChannel() const { return m_channel; }
    void setActive(bool active) { m_bActive = active; }
    bool isActive() const { return m_bActive; }
    void setSelected(bool selected) { m_bSelected = selected; }
    bool isSelected() const { return m_bSelected; }

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
    const int m_type = LMGrabType::GRAB_EMPTY;
    unsigned int objID;

    ofVec2f m_from, m_to, m_clickedPos;
    bool m_bActive, m_bSelected, m_bSelectedFrom, m_bSelectedTo;

    float m_pixelsInLed;
    float startAngle;
    int m_channel, m_pixelsInObject;

    vector<ofVec2f> m_points;
    ofRectangle m_bounds;



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
    os << "Grab obj with type=" << ofToString(obj.m_type) << " from=" << obj.m_from
       << " to=" << obj.m_to;
    return os;
}

class ofxLedGrabLine : public ofxLedGrabObject {
    const int m_type = LMGrabType::GRAB_LINE;
    bool m_bDoubleLine;

public:
    ofxLedGrabLine(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                   float pixInLed = 2.f, bool bDouble = false)
        : ofxLedGrabObject(from, to, pixInLed)
        , m_bDoubleLine(bDouble)
    {
        updatePoints();
    }

    ofxLedGrabLine(const ofxLedGrabLine &line)
    {
        m_from = line.m_from;
        m_to = line.m_to;
        m_pixelsInLed = line.m_pixelsInLed;
        m_bDoubleLine = line.m_bDoubleLine;
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
        sInput->bind(m_from.x);
        sInput = gui->addSlider("from Y", 0, 1000);
        sInput->bind(m_from.y);

        sInput = gui->addSlider("to X", 0, 2000);
        sInput->bind(m_to.x);
        sInput = gui->addSlider("to Y", 0, 2000);
        sInput->bind(m_to.y);
#endif
    }

    ~ofxLedGrabLine(){};

    bool mousePressed(const ofMouseEventArgs &args) override
    {
        if (m_bounds.inside(args)) {
            m_bSelected = true;
            m_clickedPos = args;
            pressedFromTo(args);
            return true;
        }
        m_bSelected = false;
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args) override
    {
        if (m_bSelectedFrom) {
            ofxLedGrabObject::setFrom(args);
            return true;
        }
        else if (m_bSelectedTo) {
            ofxLedGrabObject::setTo(args);
            return true;
        }
        else if (m_bSelected) {
            ofVec2f dist(args - m_clickedPos);
            m_clickedPos = args;
            ofxLedGrabObject::set(m_from + dist, m_to + dist);
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
        ofDrawLine(m_from, m_to);
        if (m_bSelected || isActive()) {
            for (vector<ofVec2f>::iterator i = m_points.begin(); i != m_points.end(); i++) {
                ofDrawCircle((*i), m_pixelsInLed / 2);
            }
            ofNoFill();
            ofDrawRectangle(m_bounds);
            ofFill();
            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(objID), m_from);
            ofSetColor(107, 230, 180, 250);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               m_from.getInterpolated(m_to, .5));
        }
    }

    void drawGui() override { ; }

    void updatePoints() override
    {
        updateBounds();
        float dist = m_from.distance(m_to);
        m_pixelsInObject = static_cast<int>(dist / m_pixelsInLed);
        m_points.clear();
        m_points.reserve(m_pixelsInObject);
        for (int pix_num = 0; pix_num < m_pixelsInObject; ++pix_num) {
            float tmp_num = static_cast<float>(pix_num) + .5f;
            float step = static_cast<float>(tmp_num / m_pixelsInObject);
            m_points.push_back(m_from.getInterpolated(m_to, step));
        }

        if (m_bDoubleLine) {
            auto tmpPoints = m_points;
            std::reverse(tmpPoints.begin(), tmpPoints.end());
            m_points.insert(m_points.end(), tmpPoints.begin(), tmpPoints.end());
        }
    }
    void updateBounds() override
    {
        auto min = ofVec2f(MIN(m_from.x, m_to.x), MIN(m_from.y, m_to.y));
        auto max = ofVec2f(MAX(m_from.x, m_to.x), MAX(m_from.y, m_to.y));
        m_bounds.set(min - ofVec2f(POINT_RAD), max + ofVec2f(POINT_RAD));
    }

    void save(ofxXmlSettings &xml, const int tagNum) override
    {
        ofxLedGrabObject::save(xml, tagNum);
        xml.setValue("LN:TYPE", this->m_type, tagNum);
        xml.setValue("LN:bDouble", m_bDoubleLine, tagNum);
    }
    void load(ofxXmlSettings &xml, int tagNum) override
    {
        m_bDoubleLine = xml.getValue("LN:bDouble", false, tagNum);
    }
};

class ofxLedGrabCircle : public ofxLedGrabObject {
    const int m_type = LMGrabType::GRAB_CIRCLE;
    float m_radius;
    float startAngle;
    bool bClockwise;

public:
    ofxLedGrabCircle(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                     float pixInLed = 2.f)
        : ofxLedGrabObject(from, to, pixInLed)
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
        if (m_bounds.inside(args)) {
            m_bSelected = true;
            if (m_from.distance(args) <= POINT_RAD) {
                m_bSelectedFrom = true;
                m_bSelectedTo = false;
            }
            else if (m_from.distance(args) <= m_radius + POINT_RAD) {
                m_bSelectedFrom = false;
                m_bSelectedTo = true;
            }
            return true;
        }
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args)
    {
        if (m_bSelectedFrom) {
            m_bSelected = true;
            ofxLedGrabObject::set(args, m_to - m_from + args);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrabObject::setTo(args);
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
        ofDrawCircle(m_from, POINT_RAD);
        ofNoFill();
        ofDrawCircle(m_from, m_radius);

        if (isActive()) {
            m_bSelected ? ofFill() : ofNoFill();
            ofSetColor(150, 150, 150, 150); /// color for first point
            for (vector<ofVec2f>::iterator i = m_points.begin(); i != m_points.end(); i++) {
                ofDrawCircle((*i), m_pixelsInLed / 2);
                ofSetColor(color);
            }
            ofSetColor(100, 100, 100, 150);
            ofNoFill();
            ofDrawRectangle(m_bounds);
            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(objID), m_from);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               m_from.getInterpolated(m_to, .5));
        }
    }

    void drawGui() override { ; }

    void setStartAngle(float angle) override { startAngle = angle; }
    void setClockwise(bool bClock) { bClockwise = bClock; }

    void updatePoints() override
    {
        m_radius = m_from.distance(m_to);
        updateBounds();
        float dist = m_radius * TWO_PI;
        int pixelsInLine = static_cast<int>(dist / m_pixelsInLed);
        float degreeStep = 360.f / static_cast<float>(pixelsInLine);
        float currStep = startAngle;
        m_points.clear();
        m_points.reserve(pixelsInLine);
        for (int i = 0; i < pixelsInLine; i++) {
            ofVec2f tmp = ofVec2f(cos(currStep * PI / 180), sin(currStep * PI / 180)) * m_radius;
            tmp += m_from;
            // check if point is on the screen
            if (tmp.x >= 0 && tmp.y >= 0)
                m_points.push_back(tmp);
            bClockwise ? currStep -= degreeStep : currStep += degreeStep;
        }
    }
    void updateBounds() override
    {
        m_bounds.setPosition(m_from - ofVec2f(m_radius + POINT_RAD));
        m_bounds.setSize((m_radius + POINT_RAD) * 2.f, (m_radius + POINT_RAD) * 2.f);
    }

    void save(ofxXmlSettings &xml, const int tagNum) override
    {
        ofxLedGrabObject::save(xml, tagNum);
        xml.setValue("LN:TYPE", this->m_type, tagNum);
        xml.setValue("LN:startAngle", startAngle, tagNum);
        xml.setValue("LN:isClockwise", bClockwise, tagNum);
    }

    void load(ofxXmlSettings &xml, int tagNum) override
    {
        setStartAngle(xml.getValue("LN:startAngle", -90, tagNum));
        setClockwise(xml.getValue("LN:isClockwise", true, tagNum));
        updatePoints();
    }
};

class ofxLedGrabMatrix : public ofxLedGrabObject {
    const int m_type = LMGrabType::GRAB_MATRIX;
    int m_columns, m_rows;
    bool m_isVertical, m_isZigzag;
    float startAngle;

public:
    ofxLedGrabMatrix(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                     float pixInLed = 2.f, bool isVertical = true, bool isZigzag = true)
        : ofxLedGrabObject(from, to, pixInLed)
        , m_isVertical(isVertical)
        , m_isZigzag(isZigzag)
    {
        updatePoints();
    }

    ~ofxLedGrabMatrix()
    {
        ofLogVerbose("[ofxLedGrabMatrix] Detor: clear m_points");
        m_points.clear();
    }

    bool mousePressed(const ofMouseEventArgs &args)
    {
        if (m_bounds.inside(args)) {
            m_bSelected = true;
            m_clickedPos = args;
            pressedFromTo(args);
            return true;
        }
        m_bSelected = false;
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args)
    {
        if (m_bSelectedFrom) {
            m_bSelected = true;
            ofxLedGrabObject::setFrom(args);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrabObject::setTo(args);
            return true;
        }
        else if (m_bSelected) {
            ofVec2f dist(args - m_clickedPos);
            m_clickedPos = args;
            ofxLedGrabObject::set(m_from + dist, m_to + dist);
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
        ofDrawCircle(m_from, 3);
        ofDrawCircle(m_to, 3);

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
            ofDrawBitmapString("id" + ofToString(objID), m_from);

            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("w=" + ofToString(m_isVertical ? m_rows : m_columns),
                               (m_from.x + m_to.x) * .5, m_from.y);
            ofDrawBitmapString("h=" + ofToString(m_isVertical ? m_columns : m_rows), m_from.x,
                               (m_from.y + m_to.y) * .5);

            ofSetColor(200, 200, 200, 150);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               m_from.getInterpolated(m_to, .5));
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
        updateBounds();

        //        m_columns = static_cast<int>(m_isVertical? abs(m_from.y - m_to.y) / m_pixelsInLed
        //        : abs(m_from.x - m_to.x) / m_pixelsInLed);
        //        m_rows = static_cast<int>(m_isVertical? abs(m_from.x - m_to.x) / m_pixelsInLed :
        //        abs(m_from.y - m_to.y) / m_pixelsInLed);
        m_columns = static_cast<int>(m_isVertical ? abs(m_from.y - m_to.y) / m_pixelsInLed
                                                  : abs(m_from.x - m_to.x) / m_pixelsInLed);
        m_rows = static_cast<int>(m_isVertical ? abs(m_from.x - m_to.x) / m_pixelsInLed
                                               : abs(m_from.y - m_to.y) / m_pixelsInLed);

        m_pixelsInObject = m_columns * m_rows;

        m_points.clear();
        m_points.reserve(m_pixelsInObject);

        if (m_isVertical) {
            for (int row = 0; row < m_rows; ++row) {
                ofVec2f rowPos = m_from.getInterpolated(ofVec2f(m_to.x, m_from.y),
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
                        ofVec2f(rowPos.x, m_to.y), (static_cast<float>(cntr) + .5f) / m_columns);
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
                    tmp += m_from;
                    if (tmp.x >= 0 && tmp.y >= 0)
                        m_points.emplace_back(std::move(tmp));
                }
            }
        }
    }
    void updateBounds() override
    {
        auto min = ofVec2f(MIN(m_from.x, m_to.x), MIN(m_from.y, m_to.y));
        auto max = ofVec2f(MAX(m_from.x, m_to.x), MAX(m_from.y, m_to.y));
        m_bounds.set(min - ofVec2f(POINT_RAD), max + ofVec2f(POINT_RAD));
    }

    void save(ofxXmlSettings &xml, const int tagNum) override
    {
        ofxLedGrabObject::save(xml, tagNum);
        xml.setValue("LN:TYPE", this->m_type, tagNum);
        xml.setValue("LN:rows", m_rows, tagNum);
        xml.setValue("LN:columns", m_columns, tagNum);
        xml.setValue("LN:isVertical", m_isVertical, tagNum);
        xml.setValue("LN:isZigzag", m_isZigzag, tagNum);
    }

    void load(ofxXmlSettings &xml, int tagNum) override
    {
        setNumRows(xml.getValue("LN:numRows", 1, tagNum));
        setNumColumns(xml.getValue("LN:numColumns", 1, tagNum));
        m_isVertical = xml.getValue("LN:isVertical", true, tagNum);
        m_isZigzag = xml.getValue("LN:isZigzag", true, tagNum);
        updatePoints();
    }
};

} // namespace LedMapper