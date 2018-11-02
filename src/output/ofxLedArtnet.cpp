//
// Created by Timofey Tavlintsev on 29/10/2018.
//

#include "ofxLedArtnet.h"
#include "Common.h"

namespace LedMapper {

static const vector<string> s_channelList = { "channel 1", "channel 2" };

size_t ofxLedArtnet::getNumChannels() const { return s_channelList.size(); }

// ref protocols
// https://art-net.org.uk/structure/streaming-packets/artdmx-packet-definition/

const string s_artnetHead = "Art-Net";
const short s_artnetOpOutput = 0x5000;
const size_t s_artnetSeviceDataSize = 11;

void ofxLedArtnet::bindGui(ofxDatGui *gui)
{
    gui->addTextInput(LCGUITextIP, m_ip)->onTextInputEvent([this](ofxDatGuiTextInputEvent e) {
        if (ValidateIP(e.text)) {
            setup(e.text);
        }
    });
}

/*
void ofxLedArtnet::send(const ofPixels &data)
{
    if (m_output.empty())
        return;

    /// update udp connection
    setupUdp(m_curUdpIp, 6454);

    bool prevStatus = m_statusOk;
    const size_t datasize = m_output.size();
    const size_t universesCount = datasize / 512 + 1;

    for (size_t universe = 0; universe < universesCount; ++universe) {

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
        size_t unisize = datasize - universe * 512 > 512 ? 512 : datasize - universe * 512;
        artnetBuff.emplace_back(datasize >> 8);
        artnetBuff.emplace_back(datasize & 0xff);

        artnetBuff.insert(artnetBuff.end(),
                          std::make_move_iterator(m_output.begin() + universe * 512),
                          std::make_move_iterator(m_output.begin() + universe * 512 + unisize));

        m_statusOk
                = (m_frameConnection.Send((const char *)artnetBuff.data(), m_output.size()) != -1);
        if (m_statusOk != prevStatus && m_statusChanged != nullptr)
            m_statusChanged();
    }

    m_output.erase(m_output.begin(), m_output.end());
}
*/

} // namespace LedMapper