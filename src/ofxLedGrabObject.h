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

#include "Common.h"
#include "ofMain.h"
#include "ofxXmlSettings.h"

namespace LedMapper {

class ofxLedGrab;
class ofxLedGrabLine;
class ofxLedGrabCircle;
class ofxLedGrabMatrix;

enum LMGrabType { GRAB_EMPTY, GRAB_LINE, GRAB_CIRCLE, GRAB_MATRIX, GRAB_SELECT };
static const ofColor s_colorGreen = ofColor::fromHex(LM_COLOR_GREEN_LIGHT);

/// based on  glm::closestPointOnLine
inline float getPointDistanceToLine(const ofVec2f &point, const ofVec2f &lineFrom,
                                    const ofVec2f &lineTo)
{
    float LineLength = lineFrom.distance(lineTo);
    glm::fvec2 Vector = point - lineFrom;
    glm::fvec2 LineDirection = (lineTo - lineFrom) / LineLength;
    // Project Vector to LineDirection to get the distance of point from line
    float Distance = glm::dot(Vector, LineDirection);
    if (Distance < 0.f)
        return POINT_RAD * 10; // miss
    if (Distance > LineLength)
        return POINT_RAD * 10; // miss
    glm::fvec2 pointOnLine = lineFrom + LineDirection * Distance;
    return point.distance(pointOnLine);
}

class ofxLedGrab {
public:
    ofxLedGrab(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
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
    ofxLedGrab(const ofxLedGrab &line)
        : m_from(line.m_from)
        , m_to(line.m_to)
        , m_type(line.m_type)
        , m_bActive(false)
        , m_bSelected(false)
        , m_bSelectedFrom(false)
        , m_bSelectedTo(false)
        , m_pixelsInLed(line.m_pixelsInLed)
    {
    }
    virtual ~ofxLedGrab(){};
    virtual void updatePoints() = 0;
    virtual void draw(const ofColor &color = ofColor(200, 200, 200, 150)) = 0;
    virtual void drawGui() = 0;
    virtual void updateBounds() = 0;
    virtual bool mousePressed(ofMouseEventArgs &args) = 0;
    virtual bool mouseDragged(const ofMouseEventArgs &args) = 0;
    virtual bool mouseReleased(const ofMouseEventArgs &args) = 0;
    virtual bool pressedFromTo(const ofVec2f &args)
    {
        if (m_to.distance(args) <= POINT_RAD) {
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
            return true;
        }
        else if (m_from.distance(args) <= POINT_RAD) {
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
            return true;
        }

        return false;
    }
    void deselectFromTo() { m_bSelectedFrom = m_bSelectedTo = false; }

    virtual void load(ofxXmlSettings &xml, int tagNum = -1) = 0;
    virtual void save(ofxXmlSettings &xml, const int tagNum)
    {
        xml.setValue("LN:CHANNEL", m_channel, tagNum);
        xml.setValue("LN:fromX", m_from.x, tagNum);
        xml.setValue("LN:fromY", m_from.y, tagNum);
        xml.setValue("LN:toX", m_to.x, tagNum);
        xml.setValue("LN:toY", m_to.y, tagNum);
    }

    virtual ofJson toJson() const
    {
        return ofJson{ { "channel", m_channel },
                       { "fromX", m_from.x },
                       { "fromY", m_from.y },
                       { "toX", m_to.x },
                       { "toY", m_to.y } };
    }
    virtual void fromJson(const ofJson &j)
    {
        m_channel = j.count("channel") ? j.at("channel").get<int>() : 0;
        m_from = { j.count("fromX") ? j.at("fromX").get<float>() : 0.f,
                   j.count("fromY") ? j.at("fromY").get<float>() : 0.f };
        m_to = { j.count("toX") ? j.at("toX").get<float>() : 0.f,
                 j.count("toY") ? j.at("toY").get<float>() : 0.f };
    }

    void set(const ofVec2f &from, const ofVec2f &to)
    {
        if (from.x < 0 || from.y < 0 || to.x < 0 || to.y < 0)
            return;
        m_from = from;
        m_to = to;
        updatePoints();
    };

    void setFrom(const ofVec2f &from) { set(from, m_to); }
    void setTo(const ofVec2f &to) { set(m_from, to); }
    ofVec2f getFrom() const { return m_from; }
    ofVec2f getTo() const { return m_to; }

    void setStartAngle(float angle) { m_startAngle = angle; }

    void setClickedPos(const ofVec2f &pos) { m_clickedPos = pos; }
    const ofVec2f &getClickedPos() const { return m_clickedPos; }

    const ofRectangle &getBounds() { return m_bounds; }

    void setPixelsInLed(float _pixs)
    {
        m_pixelsInLed = _pixs;
        updatePoints();
    };
    float getPixelsInLed() const { return m_pixelsInLed; }
    void setObjectId(unsigned int _objID) { m_id = _objID; };
    unsigned int getObjectId() const { return m_id; };
    void setChannel(int _channel) { m_channel = _channel; }
    int getChannel() const { return m_channel; }
    void setActive(bool active) { m_bActive = active; }
    bool isActive() const { return m_bActive; }
    void setSelected(bool selected) { m_bSelected = selected; }
    bool isSelected() const { return m_bSelected; }

    int getType() const { return m_type; };

    const vector<ofVec2f> &points() const { return m_points; }

    vector<glm::vec3> getLedPoints()
    {
        vector<glm::vec3> ledPoints;
        ledPoints.reserve(m_points.size());
        for_each(m_points.begin(), m_points.end(), [&ledPoints](const ofVec2f &point) {
            ledPoints.push_back({ point.x, point.y, 0.f });
        });

        return ledPoints;
    }

    /// ------- Variables -------
    bool m_bActive, m_bSelected, m_bSelectedFrom, m_bSelectedTo;
    unsigned int m_id;
    int m_type = LMGrabType::GRAB_EMPTY;
    int m_channel, m_pixelsInObject;
    float m_pixelsInLed, m_startAngle;
    ofVec2f m_from, m_to, m_clickedPos;

    vector<ofVec2f> m_points;
    ofRectangle m_bounds;

#ifndef LED_MAPPER_NO_GUI
    void bindGui(ofxDatGui *gui)
    {
        gui->clear();

        gui->addHeader("Grab " + to_string(m_id));
        auto sInput = gui->addSlider("pixel step", 1, 20);
        sInput->bind(m_pixelsInLed);
        sInput->setValue(m_pixelsInLed);
        //        sInput = gui->addToggle("double line", m_isDoubleLine);
        //            ofxDatGuiToggle *tInput = gui->addToggle("double line", false);
        //            tInput->bind(m_isDoubleLine);

        sInput = gui->addSlider("from X", 0, 1000);
        sInput->bind(m_from.x);
        sInput->setValue(m_from.x);
        sInput = gui->addSlider("from Y", 0, 1000);
        sInput->bind(m_from.y);
        sInput->setValue(m_from.y);

        sInput = gui->addSlider("to X", 0, 2000);
        sInput->bind(m_to.x);
        sInput->setValue(m_to.x);
        sInput = gui->addSlider("to Y", 0, 2000);
        sInput->bind(m_to.y);
        sInput->setValue(m_to.y);
    }

#endif

    friend std::ostream &operator<<(std::ostream &os, const ofxLedGrab &obj);
}; // namespace LedMapper

inline std::ostream &operator<<(std::ostream &os, const ofxLedGrab &obj)
{
    os << "Grab obj with type=" << ofToString(obj.m_type) << " from=" << obj.m_from
       << " to=" << obj.m_to;
    return os;
}

class ofxLedGrabLine : public ofxLedGrab {
    bool m_isDoubleLine;

public:
    ofxLedGrabLine(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                   float pixInLed = 2.f, bool bDouble = false)
        : ofxLedGrab(from, to, pixInLed)
        , m_isDoubleLine(bDouble)
    {
        ofxLedGrab::m_type = LMGrabType::GRAB_LINE;
        updatePoints();
    }

    ofxLedGrabLine(const ofxLedGrabLine &line)
        : ofxLedGrab(line)
        , m_isDoubleLine(line.m_isDoubleLine)
    {
        updatePoints();
    }

    ~ofxLedGrabLine(){};

    bool mousePressed(ofMouseEventArgs &args) override
    {
        ofxLedGrab::setClickedPos(args);
        if (pressedFromTo(args)) {
            ofxLedGrab::setSelected(true);
            return true;
        }

        if (getPointDistanceToLine(args, m_from, m_to) < POINT_RAD * 2) {
            ofxLedGrab::setSelected(true);
            ofxLedGrab::pressedFromTo(args);
            return true;
        }
        ofxLedGrab::deselectFromTo();
        /// when holding shift don't deselect
        if (!args.hasModifier(OF_KEY_SHIFT))
            ofxLedGrab::setSelected(false);
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args) override
    {
        // set drag end on freshly created grab
        if (m_from == m_to) {
            m_bSelectedTo = true;
        }

        if (m_bSelectedFrom) {
            ofxLedGrab::setFrom(args);
            return true;
        }
        else if (m_bSelectedTo) {
            ofxLedGrab::setTo(args);
            return true;
        }
        else if (m_bSelected) {
            auto dist = args - ofxLedGrab::getClickedPos();
            ofLogVerbose() << "[mouseDragged] args=" << args
                           << "- prev pos=" << ofxLedGrab::getClickedPos() << " = " << dist;
            ofxLedGrab::setClickedPos(args);
            ofxLedGrab::set(m_from + dist, m_to + dist);
            ofLogVerbose() << "from=" << m_from << " to=" << m_to;
            return true;
        }
        return false;
    }

    bool mouseReleased(const ofMouseEventArgs &args) override
    {
        ofxLedGrab::deselectFromTo();
        return true;
    }

    void draw(const ofColor &color = ofColor(200, 200, 200, 150)) override
    {
        ofSetColor(color);
        ofDrawLine(m_from, m_to);
        if (isActive()) {
            ofFill();
            ofDrawCircle(m_from, 3);
            ofDrawCircle(m_to, 3);
            ofSetColor(s_colorGreen);
            ofDrawBitmapString("id" + ofToString(m_id), m_from + ofVec2f(0, 2));
            if (!m_bSelected)
                return;
            ofDrawBitmapString(ofToString(m_points.size()),
                               m_from.getInterpolated(m_to, .5) - ofVec2f(0, 2));
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

        if (m_isDoubleLine) {
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
        ofxLedGrab::save(xml, tagNum);
        xml.setValue("LN:TYPE", this->m_type, tagNum);
        xml.setValue("LN:bDouble", m_isDoubleLine, tagNum);
    }
    void load(ofxXmlSettings &xml, int tagNum) override
    {
        m_isDoubleLine = xml.getValue("LN:bDouble", false, tagNum);
        updatePoints();
    }

    ofJson toJson() const override
    {
        ofJson out = ofxLedGrab::toJson();
        out["type"] = m_type;
        out["isDouble"] = m_isDoubleLine;
        return out;
    }
    void fromJson(const ofJson &j) override
    {
        ofxLedGrab::fromJson(j);
        m_isDoubleLine = j.count("isDouble") ? j.at("isDouble").get<bool>() : false;
    }
};

class ofxLedGrabCircle : public ofxLedGrab {
    float m_radius;
    bool m_isClockwise;

public:
    ofxLedGrabCircle(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                     float pixInLed = 2.f)
        : ofxLedGrab(from, to, pixInLed)
        , m_isClockwise(true)
    {
        ofxLedGrab::m_type = LMGrabType::GRAB_CIRCLE;
        ofxLedGrab::m_startAngle = -90.f;
        updatePoints();
    }

    ofxLedGrabCircle(const ofxLedGrabCircle &circle)
        : ofxLedGrab(circle)
        , m_isClockwise(circle.m_isClockwise)
    {
        ofxLedGrab::m_startAngle = circle.m_startAngle;
        updatePoints();
    }

    ~ofxLedGrabCircle(){};

    bool mousePressed(ofMouseEventArgs &args) override
    {
        ofxLedGrab::setClickedPos(args);

        if (m_from.distance(args) <= POINT_RAD) {
            m_bSelectedFrom = true;
            m_bSelectedTo = false;
            m_bSelected = true;
            return true;
        }
        if (m_from.distance(args) <= m_radius + POINT_RAD) {
            m_bSelectedFrom = false;
            m_bSelectedTo = true;
            m_bSelected = true;
            return true;
        }

        if (!args.hasModifier(OF_KEY_SHIFT))
            ofxLedGrab::setSelected(false);

        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args) override
    {
        if (m_bSelectedFrom || m_bSelected) {
            m_bSelected = true;
            ofxLedGrab::set(args, m_to - m_from + args);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrab::setTo(args);
            return true;
        }
        return false;
    }

    bool mouseReleased(const ofMouseEventArgs &args) override
    {
        ofxLedGrab::deselectFromTo();
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
            ofFill();
            ofSetColor(150, 150, 150, 150); /// color for first point
            ofDrawCircle((*m_points.begin()), m_pixelsInLed / 1.5);
            ofSetColor(s_colorGreen);
            ofDrawBitmapString("id" + ofToString(m_id), m_from);
            if (!m_bSelected)
                return;
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               m_from.getInterpolated(m_to, .5));
        }
    }

    void drawGui() override { ; }

    void setClockwise(bool bClock) { m_isClockwise = bClock; }

    void updatePoints() override
    {
        updateBounds();
        m_radius = m_from.distance(m_to);
        float dist = m_radius * TWO_PI;
        int pixelsInLine = static_cast<int>(dist / m_pixelsInLed);
        float degreeStep = 360.f / static_cast<float>(pixelsInLine);
        float currStep = m_startAngle;
        m_points.clear();
        m_points.reserve(pixelsInLine);
        for (int i = 0; i < pixelsInLine; i++) {
            ofVec2f tmp = ofVec2f(cos(currStep * PI / 180), sin(currStep * PI / 180)) * m_radius;
            tmp += m_from;
            // check if point is on the screen
            if (tmp.x >= 0 && tmp.y >= 0)
                m_points.push_back(tmp);
            m_isClockwise ? currStep -= degreeStep : currStep += degreeStep;
        }
    }
    void updateBounds() override
    {
        m_bounds.set(ofVec2f(m_from - ofVec2f(m_radius + POINT_RAD)),
                     ofVec2f((m_radius + POINT_RAD) * 2.f));
    }

    void save(ofxXmlSettings &xml, const int tagNum) override
    {
        ofxLedGrab::save(xml, tagNum);
        xml.setValue("LN:TYPE", this->m_type, tagNum);
        xml.setValue("LN:startAngle", m_startAngle, tagNum);
        xml.setValue("LN:isClockwise", m_isClockwise, tagNum);
    }

    void load(ofxXmlSettings &xml, int tagNum) override
    {
        setStartAngle(xml.getValue("LN:startAngle", -90, tagNum));
        setClockwise(xml.getValue("LN:isClockwise", true, tagNum));
        updatePoints();
    }
    ofJson toJson() const override
    {
        ofJson out = ofxLedGrab::toJson();
        out["type"] = m_type;
        out["startAngle"] = m_startAngle;
        out["isClockwise"] = m_isClockwise;
        return out;
    }
    void fromJson(const ofJson &j) override
    {
        ofxLedGrab::fromJson(j);
        m_startAngle = j.count("startAngle") ? j.at("startAngle").get<float>() : 0;
        m_isClockwise = j.count("isClockwise") ? j.at("isClockwise").get<bool>() : true;
    }
};

class ofxLedGrabMatrix : public ofxLedGrab {
    int m_columns, m_rows;
    bool m_isVertical, m_isZigzag;

public:
    ofxLedGrabMatrix(const ofVec2f &from = ofVec2f(0), const ofVec2f &to = ofVec2f(0),
                     float pixInLed = 2.f, bool isVertical = true, bool isZigzag = true)
        : ofxLedGrab(from, to, pixInLed)
        , m_isVertical(isVertical)
        , m_isZigzag(isZigzag)
    {
        ofxLedGrab::m_type = LMGrabType::GRAB_MATRIX;
        updatePoints();
    }

    ofxLedGrabMatrix(const ofxLedGrabMatrix &grab)
        : ofxLedGrab(grab)
        , m_isVertical(grab.m_isVertical)
        , m_isZigzag(grab.m_isZigzag)
    {
        updatePoints();
    }

    ~ofxLedGrabMatrix() {}

    bool mousePressed(ofMouseEventArgs &args) override
    {
        ofxLedGrab::setClickedPos(args);

        if (pressedFromTo(args)) {
            ofxLedGrab::setSelected(true);
            return true;
        }
        if (m_bounds.inside(args)) {
            ofxLedGrab::setSelected(true);
            return true;
        }
        ofxLedGrab::deselectFromTo();
        if (!args.hasModifier(OF_KEY_SHIFT))
            ofxLedGrab::setSelected(false);
        return false;
    }

    bool mouseDragged(const ofMouseEventArgs &args) override
    {
        // set drag end on freshly created grab
        if (m_bSelected && m_from == m_to) {
            m_bSelectedTo = true;
        }

        if (m_bSelectedFrom) {
            m_bSelected = true;
            ofxLedGrab::setFrom(args);
            return true;
        }
        else if (m_bSelectedTo) {
            m_bSelected = true;
            ofxLedGrab::setTo(args);
            return true;
        }
        else if (m_bSelected) {
            ofVec2f dist(args - ofxLedGrab::getClickedPos());
            ofxLedGrab::setClickedPos(args);
            ofxLedGrab::set(m_from + dist, m_to + dist);
            return true;
        }

        return false;
    }

    bool mouseReleased(const ofMouseEventArgs &args) override
    {
        ofxLedGrab::deselectFromTo();
        return true;
    }

    void draw(const ofColor &color = ofColor(100, 100, 100, 150)) override
    {
        ofSetColor(color);
        ofNoFill();
        ofDrawRectangle(m_bounds);

        if (isActive()) {
            ofFill();
            ofDrawCircle(m_from, 3);
            ofDrawCircle(m_to, 3);

            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("id" + ofToString(m_id), m_from);

            if (!m_bSelected)
                return;

            ofSetColor(0, 250, 150, 250);
            ofDrawBitmapString("w=" + ofToString(m_isVertical ? m_rows : m_columns),
                               (m_from.x + m_to.x) * .5, m_from.y);
            ofDrawBitmapString("h=" + ofToString(m_isVertical ? m_columns : m_rows), m_from.x,
                               (m_from.y + m_to.y) * .5);

            ofSetColor(200, 200, 200, 150);
            ofDrawBitmapString(ofToString(static_cast<int>(m_points.size())),
                               m_from.getInterpolated(m_to, .5));
        }
    }

    void drawGui() override { ; }

    void setNumRows(int rows) { m_rows = rows; }
    void setNumColumns(int columns) { m_columns = columns; }

    void updatePoints() override
    {
        updateBounds();
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
        m_bounds.set(ofVec2f(MIN(m_from.x, m_to.x), MIN(m_from.y, m_to.y)),
                     ofVec2f(MAX(m_from.x, m_to.x), MAX(m_from.y, m_to.y)));
    }

    void save(ofxXmlSettings &xml, const int tagNum) override
    {
        ofxLedGrab::save(xml, tagNum);
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

    ofJson toJson() const override
    {
        ofJson out = ofxLedGrab::toJson();
        out["type"] = m_type;
        out["isVertical"] = m_isVertical;
        out["isZigzag"] = m_isZigzag;
        return out;
    }
    void fromJson(const ofJson &j) override
    {
        ofxLedGrab::fromJson(j);
        m_isVertical = j.count("isVertical") ? j.at("isVertical").get<bool>() : true;
        m_isZigzag = j.count("isZigzag") ? j.at("isZigzag").get<bool>() : true;
    }
};
} // namespace LedMapper

namespace nlohmann {
template <> struct adl_serializer<std::unique_ptr<LedMapper::ofxLedGrab>> {

    static void to_json(json &j, const std::unique_ptr<LedMapper::ofxLedGrab> &grab)
    {
        if (grab) {
            j = grab->toJson();
        }
        else {
            j = nullptr;
        }
    }

    // this is the overload needed for non-copyable types,
    static std::unique_ptr<LedMapper::ofxLedGrab> from_json(const json &j)
    {
        std::unique_ptr<LedMapper::ofxLedGrab> grab = nullptr;
        if (j.is_null()) {
            return grab;
        }
        else if (j.count("type") > 0 && j.at("type").is_number()) {
            auto type = j.at("type").get<LedMapper::LMGrabType>();
            if (type == LedMapper::LMGrabType::GRAB_LINE)
                grab = make_unique<LedMapper::ofxLedGrabLine>();
            else if (type == LedMapper::LMGrabType::GRAB_MATRIX)
                grab = make_unique<LedMapper::ofxLedGrabMatrix>();
            else if (type == LedMapper::LMGrabType::GRAB_CIRCLE)
                grab = make_unique<LedMapper::ofxLedGrabCircle>();

            try {
                grab->fromJson(j);
            }
            catch (std::exception &ex) {
                ofLogError() << "Parse Grab json failed with:" << ex.what();
            }
        }

        return grab;
    }
};
} // namespace nlohmann
