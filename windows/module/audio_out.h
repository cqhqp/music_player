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
    virtual bool init(PcmFormatInfo info, const std::function<void(PcmFormatInfo)> &databack) = 0;
    virtual bool isInit() const = 0;
    virtual void init_format(){};
    virtual void getDefaultFormat(const std::function<void(PcmFormatInfo)> &databack){};
    virtual void pause(){};
    virtual void add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info){};

};

struct PcmObj: PcmFormatInfo
{
    uint8_t *mem = nullptr;
    int64_t mem_size = 0;
    int64_t data_size = 0;
};

struct PcmMem
{
    uint8_t *mem = nullptr;
    int64_t mem_size = 0;
    int64_t data_size = 0;
};

#include <future>
#include <queue>
#include <mutex>
#include <variant>
class SDL2Speaker : public IAudioOutput
{
public:
    SDL2Speaker();
    ~SDL2Speaker();
    void play() override;
    bool init(PcmFormatInfo info, const std::function<void(PcmFormatInfo)> &databack) override;
    bool isInit() const;
    void getDefaultFormat(const std::function<void(PcmFormatInfo)> &databack) override;
    void pause() override;
    void add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info) override;

private:
    std::mutex outMutex;    // 缓存锁
    // LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
    bool init_flag = false;
    bool play_flag = false;
    size_t total_samples = 0;  
    int  frequency = 0;
    PcmFormatInfo speak_pcm_info;
    // 音频设备ID
    SDL_AudioDeviceID dev;
    std::queue<std::unique_ptr<PcmObj>> free_queue;
    std::deque<std::unique_ptr<PcmObj>> audio_queue;
    std::queue<std::unique_ptr<PcmMem>> new_audio_queue; 
    std::queue<std::unique_ptr<PcmMem>> new_free_queue;

    std::unique_ptr<PcmMem> tmp_pcmMem = nullptr;

    // PcmObj tmp_obj; //存放未用完的数据
    uint8_t *tmp_mem = nullptr;
    size_t   tmp_mem_size = 0;
    std::chrono::steady_clock::time_point pre_clock;

    std::unique_ptr<std::ofstream> debug_outfile; // 调试输出pcm
    std::unique_ptr<std::ofstream> debug_outfile2; // 调试输出pcm
};

#endif // AUDIOOUTPUT_H