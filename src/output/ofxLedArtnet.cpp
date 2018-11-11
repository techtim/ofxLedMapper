//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#include "ofxLedArtnet.h"
#include "Common.h"

namespace LedMapper {

static const vector<string> s_channelList = { "channel 1", "channel 2", "channel 3", "channel 4",
                                              "channel 5", "channel 6", "channel 7", "channel 8" };

static const int s_artnetPort = 6454;
static const size_t s_maxPixelsOut = 16320; /// compatible with PixLite 16 MkI
static const string s_defaultIp = "192.168.0.50";

vector<string> ofxLedArtnet::getChannels() noexcept { return s_channelList; }
size_t ofxLedArtnet::getMaxPixelsOut() noexcept { return s_maxPixelsOut; }

ofxLedArtnet::ofxLedArtnet()
    : m_bSetup(false)
    , m_ip(s_defaultIp)
    , m_universesInChannel(4)
{
}

void ofxLedArtnet::setup(const string ip)
{
    m_ip = move(ip);

    m_frameConnection.Close();
    m_frameConnection.Create();

    if (m_frameConnection.Connect(m_ip.c_str(), s_artnetPort)) {
        ofLogVerbose() << "[ofxLedArtnet] setup frame connect to " << m_ip << " : " << s_artnetPort;
    }

    m_frameConnection.SetSendBufferSize(MAX_SENDBUFFER_SIZE);
    m_frameConnection.SetNonBlocking(true);

    m_bSetup = true;
}

void ofxLedArtnet::bindGui(ofxDatGui *gui)
{
    auto slider = gui->addSlider(LCGUISliderUniInChan, 1, 6); // up to 1,020 RGB pixels per chan
    slider->setPrecision(0);
    slider->onSliderEvent([this](ofxDatGuiSliderEvent e) { m_universesInChannel = e.value; });

    gui->addTextInput(LCGUITextIP, m_ip)->onTextInputEvent([this](ofxDatGuiTextInputEvent e) {
        if (ValidateIP(e.text)) {
            setup(e.text);
        }
    });
}

// ref protocols
// https://art-net.org.uk/structure/streaming-packets/artdmx-packet-definition/
const string s_artnetHead = "Art-Net";
const short s_artnetOpOutput = 0x5000;
const size_t s_artnetSeviceDataSize = 11;

bool ofxLedArtnet::send(ChannelsToPix &&output)
{
    if (!m_bSetup)
        return false;

    size_t universe = 0;
    for (auto &pixels : output) {
        for (size_t offset = 0; offset < pixels.size(); offset += 512)
            sendUniverse(pixels, offset, universe++);
    }
    return true;
}

bool ofxLedArtnet::sendUniverse(vector<char> &pixels, size_t offset, size_t universe)
{

    vector<unsigned char> artnetBuff;
    artnetBuff.reserve(s_artnetHead.size() + s_artnetSeviceDataSize);
    std::copy(s_artnetHead.begin(), s_artnetHead.end(), std::back_inserter(artnetBuff));

    artnetBuff.emplace_back(0); // end string
    artnetBuff.emplace_back(s_artnetOpOutput & 0xff);
    artnetBuff.emplace_back(s_artnetOpOutput >> 8);

    artnetBuff.emplace_back(0); // protocol version high byte
    artnetBuff.emplace_back(14); // protocol version low byte

    artnetBuff.emplace_back(0); // sequence no - disable sequence(0)
    artnetBuff.emplace_back(0); // The physical input port from which DMX512

    // universe
    artnetBuff.emplace_back(universe & 0xff);
    artnetBuff.emplace_back(universe >> 8);

    // universe datasize
    size_t unisize = pixels.size() - offset > 512 ? 512 : pixels.size() - offset;
    artnetBuff.emplace_back(unisize >> 8);
    artnetBuff.emplace_back(unisize & 0xff);

    artnetBuff.insert(artnetBuff.end(), std::make_move_iterator(pixels.begin() + offset),
                      std::make_move_iterator(pixels.begin() + offset + unisize));

    return (m_frameConnection.Send((const char *)artnetBuff.data(), artnetBuff.size()) != -1);
}

void ofxLedArtnet::saveJson(ofJson &config) const
{
    config["ipAddress"] = m_ip;
    config["universesInChannel"] = m_universesInChannel;
}

void ofxLedArtnet::loadJson(const ofJson &config)
{
    setup(config.count("ipAddress") ? config.at("ipAddress").get<string>() : s_defaultIp);
    m_universesInChannel
        = config.count("universesInChannel") ? config.at("universesInChannel").get<size_t>() : 4;
}

} // namespace LedMapper