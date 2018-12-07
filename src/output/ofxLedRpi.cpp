//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#include "ofxLedRpi.h"

namespace LedMapper {

static const vector<string> s_channelList = { "channel 1", "channel 2" };

/// s_ledTypeList elements must be the same as keys in s_ledTypeToEnum map in lmListener on RPI side
static const vector<string> s_ledTypeList = { "WS281X", "SK9822" };

constexpr size_t s_maxPixelsOut = 4000;

vector<string> ofxLedRpi::getChannels() noexcept { return s_channelList; }
size_t ofxLedRpi::getMaxPixelsOut() noexcept { return s_maxPixelsOut; }

ofxLedRpi::ofxLedRpi()
    : m_bSetup(false)
    , m_ip(RPI_IP)
    , m_port(RPI_PORT)
    , m_currentLedType(s_ledTypeList.front())
{
}
ofxLedRpi::~ofxLedRpi()
{
    m_frameConnection.Close();
    m_confConnection.Close();
}

bool ofxLedRpi::resetup()
{
    setup(m_ip, m_port);
    return false; /// for ternary operator in send
}

void ofxLedRpi::setup(const string ip, int port)
{
    m_ip = move(ip);
    m_port = port;

    m_frameConnection.Close();
    m_frameConnection.Create();

    m_confConnection.Close();
    m_confConnection.Create();

    if (m_frameConnection.Connect(m_ip.c_str(), m_port)) {
        ofLogVerbose() << "[ofxLedRpi] setup frame connect to " << m_ip << " : " << m_port;
    }
    if (m_confConnection.Connect(m_ip.c_str(), RPI_CONF_PORT))
        ofLogVerbose() << "[ofxLedRpi] setup config connect to conf port=" << RPI_CONF_PORT;

    m_frameConnection.SetSendBufferSize(MAX_SENDBUFFER_SIZE);
    m_frameConnection.SetNonBlocking(true);
    m_confConnection.SetNonBlocking(true);

    m_bSetup = true;

    sendLedType(m_currentLedType);
}

void ofxLedRpi::bindGui(ofxDatGui *gui)
{
    auto dropdown = gui->addDropdown(LCGUIDropLedType, s_ledTypeList);
    dropdown->select(find(s_ledTypeList.begin(), s_ledTypeList.end(), m_currentLedType)
                     - s_ledTypeList.begin());
    dropdown->onDropdownEvent(
        [this](ofxDatGuiDropdownEvent e) { this->sendLedType(s_ledTypeList[e.child]); });

    gui->addTextInput(LCGUITextIP, m_ip)->onTextInputEvent([this](ofxDatGuiTextInputEvent e) {
        if (ValidateIP(e.text)) {
            this->setup(e.text, m_port);
        }
    });
}

bool ofxLedRpi::send(ChannelsToPix &&output)
{
    if (!m_bSetup)
        return false;

    m_output.clear();

    size_t num_bytes = 0;
    std::for_each(output.begin(), output.end(), [&](const auto &vec) { num_bytes += vec.size(); });
    /// reserve space for header + end of header + num_bytes of pixels colors
    m_output.reserve(output.size() * 2 + 2 + num_bytes);

    /// don't send too much data
    if (m_output.capacity() >= MAX_SENDBUFFER_SIZE)
        return false;

    for (size_t i = 0; i < output.size(); ++i) {
        // setup header => uint16_t number of leds per chan for each chan
        uint16_t num_leds = output[i].size() / 3;
        m_output.push_back(num_leds & 0xff);
        m_output.push_back(num_leds >> 8);
    }
    /// mark end of header
    m_output.emplace_back(0xff);
    m_output.emplace_back(0xff);
    for (auto &vec : output)
        move(vec.begin(), vec.end(), back_inserter(m_output));

    return m_frameConnection.Send(m_output.data(), m_output.size()) != -1 ? true : resetup();
}

void ofxLedRpi::sendLedType(const string &ledType)
{
    if (!m_bSetup)
        return;

    if (find(s_ledTypeList.begin(), s_ledTypeList.end(), ledType) == s_ledTypeList.end())
        return;

    ofLogVerbose() << "Update LedType with " << ledType;
    m_currentLedType = ledType;
    /// send type 5 times hoping UDP packet won't lost
    for (size_t i = 0; i < 5; ++i)
        m_confConnection.Send(m_currentLedType.c_str(), m_currentLedType.size());
}

void ofxLedRpi::saveJson(ofJson &config) const
{
    config["ledType"] = getLedType();
    config["ipAddress"] = getIP();
    config["port"] = getPort();
}

void ofxLedRpi::loadJson(const ofJson &config)
{
    m_currentLedType
        = config.count("ledType") ? config.at("ledType").get<string>() : s_ledTypeList.front();

    setup(config.count("ipAddress") ? config.at("ipAddress").get<string>() : RPI_IP,
          config.count("port") ? config.at("port").get<int>() : RPI_PORT);
}

} // namespace LedMapper