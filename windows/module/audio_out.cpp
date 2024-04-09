#include "audio_out.h"
#include <glog/export.h>
#include <glog/logging.h>
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>

#include <SDL.h>

SDL2Speaker::SDL2Speaker()
{
}

SDL2Speaker::~SDL2Speaker()
{
    if (dev)
    {
        // 停止播放并关闭音频设备
        SDL_PauseAudioDevice(dev, 1); // 暂停播放
        SDL_CloseAudioDevice(dev);
    }

    // 清理SDL
    SDL_Quit();

    // 内存释放
    if (tmp_mem)
    {
        free(tmp_mem);
        tmp_mem = nullptr;
    }
    while (!free_queue.empty())
    {
        free(free_queue.front()->mem);
        free_queue.front()->mem = nullptr;
        free_queue.pop();
    }
    while (!audio_queue.empty())
    {
        free(audio_queue.front()->mem);
        audio_queue.front()->mem = nullptr;
        audio_queue.pop_front();
    }
}

void SDL2Speaker::play()
{
    if(init_flag)
    // 开始播放音频（取消暂停）
    SDL_PauseAudioDevice(dev, 0);
}

void SDL2Speaker::pause()
{
    if(init_flag)
    // 开始播放音频（取消暂停）
    SDL_PauseAudioDevice(dev, 1);
}

void SDL2Speaker::add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info)
{
    std::unique_ptr<PcmObj> obj_ptr = nullptr;
    // LOG(ERROR) << "add_pcm size :" << size; 

    if (!free_queue.empty())
    {
        obj_ptr = std::move(free_queue.front());
        free_queue.pop();
    }
    else
    {
        obj_ptr = std::make_unique<PcmObj>();
    }
    obj_ptr->nb_samples = info.nb_samples;
    obj_ptr->channels = info.channels;
    obj_ptr->time_sec = info.time_sec;
    if (obj_ptr->mem == nullptr || obj_ptr->mem_size < size)
    {
        if (obj_ptr->mem != nullptr)
        {
            free(obj_ptr->mem);
            obj_ptr->mem = nullptr;
            obj_ptr->mem_size = 0;
        }
        obj_ptr->mem = (uint8_t *)malloc(sizeof(uint8_t) * size);
        obj_ptr->mem_size = size;
    }
    if (obj_ptr->mem)
    {
        memcpy(obj_ptr->mem, data, size);
        obj_ptr->data_size = size;
    }

    audio_queue.push_back(std::move(obj_ptr));
}

bool SDL2Speaker::init(PcmFormatInfo info)
{

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        return false;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want); // 初始化want结构体
    want.freq = speak_pcm_info.sample_rate;
    {

        if (speak_pcm_info.bitsSample == 8)
        {
            want.format = AUDIO_U8;
        }
        if (speak_pcm_info.bitsSample == 16)
        {
            want.format = AUDIO_S16LSB;
        }
        if (speak_pcm_info.bitsSample == 32)
        {
            want.format = AUDIO_S32LSB;
        }
        if (speak_pcm_info.is_float == 32)
        {
            want.format = AUDIO_F32LSB;
        }
    }
    want.channels = speak_pcm_info.channels;
    want.samples = 1024; // 每次回调的样本数
    // want.size = 2048*2*2;
    

    pre_clock = std::chrono::high_resolution_clock::now();  
    want.callback = [](void *userdata, Uint8 *stream, int len)
    {
        SDL2Speaker *context = (SDL2Speaker *)userdata;
        std::deque<std::unique_ptr<PcmObj>> &audio_queue = context->audio_queue;
        std::queue<std::unique_ptr<PcmObj>> &free_queue = context->free_queue;
        // PcmObj **tmp_obj = &context->tmp_obj;
        // 清除输出缓冲区
        memset(stream, 0, len);
        
        {
            // 调试 
            // 1、每次回调得时间差
            // 2、回调得len
            // 3、计算样本数
            double duration_clock = 0;
            {
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - context->pre_clock;
                duration_clock = elapsed.count();
            }
            // LOG(ERROR) << "duration_clock :" << duration_clock; 
            // LOG(ERROR) << "len :" << len; 
        }

        // 填充输出缓冲区，直到没有更多的数据或缓冲区已满
        while (len > 0 && !audio_queue.empty())
        {
            uint8_t *pcm_data = nullptr;
            size_t pcm_size = 0;

            std::unique_ptr<PcmObj> pcm_obj = std::move(audio_queue.front());
            audio_queue.pop_front();

            pcm_data = pcm_obj->mem;
            pcm_size = pcm_obj->data_size;

            size_t chunk_size = pcm_size * sizeof(uint8_t);
            if (chunk_size <= len)
            { // 复制整个obj的数据
                len -= chunk_size;
                memcpy(stream, pcm_data, chunk_size);
                free_queue.push(std::move(pcm_obj));
            }
            else
            {
                size_t need_size = len;
                memcpy(stream, pcm_data, need_size);
                size_t last_size = chunk_size - need_size;
                len -= need_size;

                if (context->tmp_mem == nullptr || context->tmp_mem_size < last_size)
                {
                    if (context->tmp_mem != nullptr)
                    {
                        free(context->tmp_mem);
                        context->tmp_mem = nullptr;
                        context->tmp_mem_size = 0;
                    }
                    context->tmp_mem = (uint8_t *)malloc(sizeof(uint8_t) * last_size);
                    context->tmp_mem_size = last_size;
                }
                if (context->tmp_mem)
                {
                    memcpy(context->tmp_mem, pcm_data + need_size, last_size);
                    memcpy(pcm_obj->mem, context->tmp_mem, last_size);
                    pcm_obj->data_size = last_size;
                    audio_queue.push_front(std::move(pcm_obj));
                }
            }
        }
        // LOG(ERROR) << "len 2:" << len; 
        context->pre_clock = std::chrono::high_resolution_clock::now();  
    };
    want.userdata = this;

    
    if (SDL_OpenAudio(&want,NULL) < 0)
    {
        printf("fail to open audio\n");
        return -1;
    }
    // 打开音频设备
    if (!(dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE)))
    {
        SDL_Quit();
        return false;
    }
    LOG(INFO) << "SDL freq: " << have.freq;
    LOG(INFO) << "SDL format: " << have.format; // AUDIO_F32LSB
    LOG(INFO) << "SDL channels: " << have.channels;
    LOG(INFO) << "SDL size: " << have.size;
    LOG(INFO) << "SDL samples: " << have.samples;
    init_flag = true;
    return true;
}

bool SDL2Speaker::isInit() const
{
    return init_flag;
}

void SDL2Speaker::getDefaultFormat(const std::function<void(PcmFormatInfo)> &databack)
{

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        LOG(ERROR) << "SDL could not initialize! SDL_Error: " << SDL_GetError();
        return;
    }

    // 查询音频设备的规格
    SDL_AudioSpec spec;
    if (SDL_GetAudioDeviceSpec(0, 0, &spec) != 0)
    {
        LOG(ERROR) << "Failed to get audio device spec! SDL_Error: " << SDL_GetError();
        SDL_Quit();
        return;
    }

    // // 输出音频设备的参数
    // LOG(INFO) << "Audio device supports:";
    // // LOG(INFO) << "  Name: " << spec.name;
    // LOG(INFO) << "  Format: " << spec.format; //    Signed 16-bit samples */ // AUDIO_S16LSB
    // LOG(INFO) << "  Frequency: " << spec.freq << " Hz";
    // LOG(INFO) << "  Channels: " << spec.channels;
    // LOG(INFO) << "  Samples: " << spec.samples;
    // LOG(INFO) << "  Silence: " << spec.silence;
    // LOG(INFO) << "  Padding: " << spec.padding;
    // LOG(INFO) << "  Size: " << spec.size << " bytes";

    // 清理SDL
    SDL_Quit();

    PcmFormatInfo info;
    info.sample_rate = spec.freq;
    info.channels = spec.channels;
    switch (spec.format)
    {
    case AUDIO_U8 | AUDIO_S8:
        info.bitsSample = 8;
        info.is_float = false;
    case AUDIO_S16LSB:
        info.bitsSample = 16;
        info.is_float = false;
        break;
    case AUDIO_S32LSB:
        info.bitsSample = 32;
        info.is_float = false;
        break;
    case AUDIO_F32LSB:
        info.bitsSample = 32;
        info.is_float = true;
        break;
    default:
        break;
    }

    { // 写死进行调试
    // info.bitsSample = 8;
    // info.channels = 1;
        info.bitsSample = 32;
        info.is_float = true;
        info.channels = 2;

    }

    LOG(INFO) << "  info: " << info.toString();
    speak_pcm_info = info;
    databack(info);
}