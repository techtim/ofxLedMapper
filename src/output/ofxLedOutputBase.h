//
// Created by Timofey Tavlintsev on 13/01/2019.
//

#pragma once


namespace LedMapper {

class ofxLedOutput {
public:
    virtual ~ofxLedOutput();

    virtual void setup(const string ip);
    virtual bool resetup();
    virtual bool send(ChannelsToPix &&output);

    virtual void bindGui(ofxDatGui *gui);

    virtual vector<string> getChannels() noexcept;
    virtual size_t getMaxPixelsOut() noexcept;

    virtual void saveJson(ofJson &config) const;
    virtual void loadJson(const ofJson &config);
};

} // namespace LedMapper