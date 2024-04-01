#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <cstdint>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <windows.h>
#include <dsound.h>
#include <mmreg.h> 
#include <functional>
#include "pcm_format.h"
#include <SDL.h>
#include <queue>

class IAudioOutput
{
public:
    virtual ~IAudioOutput() {}
    virtual void play() = 0;
    virtual bool init(PcmFormatInfo info) = 0;
    virtual bool isInit() const = 0;
    virtual void init_format(){};
    virtual void getDefaultFormat(const std::function<void(PcmFormatInfo)> &databack){};
    virtual void pause(){};
    virtual void add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info){};

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
    void init_format() override;

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
};

class CoreSpeaker : public IAudioOutput
{
public:
    CoreSpeaker();
    ~CoreSpeaker();
    void play() override;
    bool init(PcmFormatInfo info) override;
    bool isInit() const;
    void EnumerateAudioOutputDevices();
    void GetDefaultAudioOutputDeviceID();

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
};

class XAudio2Speaker : public IAudioOutput
{
public:
    XAudio2Speaker();
    ~XAudio2Speaker();
    void play() override;
    bool init(PcmFormatInfo info) override;
    bool isInit() const;

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
};

struct PcmObj: PcmFormatInfo
{
    uint8_t *mem = nullptr;
    int64_t mem_size = 0;
    int64_t data_size = 0;
};

class SDL2Speaker : public IAudioOutput
{
public:
    SDL2Speaker();
    ~SDL2Speaker();
    void play() override;
    bool init(PcmFormatInfo info) override;
    bool isInit() const;
    void getDefaultFormat(const std::function<void(PcmFormatInfo)> &databack) override;
    void pause() override;
    void add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info) override;

private:
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
    int  frequency = 0;
    PcmFormatInfo speak_pcm_info;
    // 音频设备ID
    SDL_AudioDeviceID dev;
    std::queue<std::unique_ptr<PcmObj>> free_queue;
    std::deque<std::unique_ptr<PcmObj>> audio_queue;
    // PcmObj tmp_obj; //存放未用完的数据
    uint8_t *tmp_mem = nullptr;
    size_t   tmp_mem_size = 0;

};

#endif // AUDIOOUTPUT_H