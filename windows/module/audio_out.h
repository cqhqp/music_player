#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <cstdint>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <windows.h>
#include <dsound.h>
#include <mmreg.h> 
#include <functional>
#include "pcm_format.h"

class IAudioOutput
{
public:
    virtual ~IAudioOutput() {}
    virtual void play() = 0;
    virtual bool init(PcmFormatInfo info) = 0;
    virtual bool isInit() const = 0;
};

class PCMOutput : public IAudioOutput
{
public:
    PCMOutput();
    ~PCMOutput();
    void play() override;
    bool init(PcmFormatInfo info) override;
    bool isInit() const;

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
};

class WASAPIOutput : public IAudioOutput
{
public:
    WASAPIOutput();
    ~WASAPIOutput();
    void play() override;
    bool init(PcmFormatInfo info) override;
    bool isInit() const;

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
};

#endif // AUDIOOUTPUT_H