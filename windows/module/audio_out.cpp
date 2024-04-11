#include "audio_out.h"
#include <glog/export.h>
#include <glog/logging.h>
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>

#include <SDL.h>
#include <fstream>
#include <iostream>

#define SDL_SAMPLES 1024

SDL2Speaker::SDL2Speaker()
{
    tmp_pcmMem = std::make_unique<PcmMem>();
    tmp_pcmMem->mem_size = SDL_SAMPLES * 2;
    tmp_pcmMem->mem = (uint8_t *)malloc(sizeof(uint8_t) * tmp_pcmMem->mem_size); // 1通道, 16位（一个采样占2字节）
    tmp_pcmMem->data_size = 0;
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

    free(tmp_pcmMem->mem);
    tmp_pcmMem->mem = nullptr;
}

void SDL2Speaker::play()
{
    if (init_flag)
        // 开始播放音频（取消暂停）
        SDL_PauseAudioDevice(dev, 0);
    play_flag = true;
}

void SDL2Speaker::pause()
{
    if (init_flag)
        // 开始播放音频（取消暂停）
        SDL_PauseAudioDevice(dev, 1);
}

// void SDL2Speaker::add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info)
// {
//     // std::lock_guard<std::mutex> lock(outMutex);
//     std::unique_ptr<PcmObj> obj_ptr = nullptr;

//     if (!free_queue.empty())
//     {
//         obj_ptr = std::move(free_queue.front());
//         free_queue.pop();
//     }
//     else
//     {
//         obj_ptr = std::make_unique<PcmObj>();
//     }
//     obj_ptr->nb_samples = info.nb_samples;
//     // obj_ptr->channels = info.channels;
//     // obj_ptr->time_sec = info.time_sec;
//     if (obj_ptr->mem == nullptr || obj_ptr->mem_size < size)
//     {
//         if (obj_ptr->mem != nullptr)
//         {
//             free(obj_ptr->mem);
//             obj_ptr->mem = nullptr;
//             obj_ptr->mem_size = 0;
//         }
//         obj_ptr->mem = (uint8_t *)malloc(sizeof(uint8_t) * size);
//         obj_ptr->mem_size = size;
//     }
//     if (obj_ptr->mem)
//     {
//         memcpy(obj_ptr->mem, data, size);
//         obj_ptr->data_size = size;
//         total_samples += obj_ptr->nb_samples;
//     }

//     audio_queue.push_back(std::move(obj_ptr));

//     // LOG(ERROR) << "audio_queue :" << audio_queue.size();
//     // LOG(ERROR) << "total_samples :" << total_samples;

//     if(!play_flag && total_samples> 1024){
//         play();
//         LOG(ERROR) << "playing >>>>>>>>>> ";
//     }
// }

void SDL2Speaker::add_pcm(const uint8_t *data, int64_t size, PcmFormatInfo info)
{    
    std::lock_guard<std::mutex> lock(outMutex);
    uint8_t *data_pos = (uint8_t *)data;
    int64_t data_size = size;

    debug_outfile2->write((const char*)data, (int)size);
    while (data_size > 0)
    {
        std::unique_ptr<PcmMem> obj_ptr = nullptr;

        if ((tmp_pcmMem->data_size + data_size) >= SDL_SAMPLES * 2)
        {
            if (!new_free_queue.empty())
            {
                obj_ptr = std::move(new_free_queue.front());
                new_free_queue.pop();
            }
            else
            {
                obj_ptr = std::make_unique<PcmMem>();
                obj_ptr->mem_size = SDL_SAMPLES * 2;
                obj_ptr->mem = (uint8_t *)malloc(sizeof(uint8_t) * obj_ptr->mem_size); // 1通道, 16位（一个采样占2字节）
            }

            if (obj_ptr && obj_ptr->mem)
            {
                if (tmp_pcmMem->data_size > 0)
                {
                    memcpy(obj_ptr->mem, tmp_pcmMem->mem, tmp_pcmMem->data_size);
                    obj_ptr->data_size = tmp_pcmMem->data_size;
                    tmp_pcmMem->data_size = 0;
                    // LOG(INFO) << "tmp size [out]:" << tmp_pcmMem->data_size;
                    // LOG(INFO) << "obj size [in 1]:" << obj_ptr->data_size;
                }
                int cpy_size = obj_ptr->mem_size - obj_ptr->data_size;
                memcpy(obj_ptr->mem+obj_ptr->data_size, data_pos, cpy_size);
                obj_ptr->data_size += cpy_size;
                data_pos += cpy_size;
                data_size -= cpy_size;
                // LOG(INFO) << "obj size [in 2]:" << obj_ptr->data_size;

                new_audio_queue.push(std::move(obj_ptr));
                // LOG(INFO) << "new_audio_queue size:" << new_audio_queue.size();
            }
        }
        else
        {
            int cpy_size = data_size;
            memcpy(tmp_pcmMem->mem, data_pos, cpy_size);
            tmp_pcmMem->data_size += cpy_size;
            data_pos += cpy_size;
            data_size -= cpy_size;
            // LOG(INFO) << "tmp size [in]:" << tmp_pcmMem->data_size;
            break;
        }
    }
    if(!play_flag && (new_audio_queue.size() * SDL_SAMPLES) > SDL_SAMPLES*2){ // 缓存2个音频帧
        play();
        LOG(ERROR) << "playing >>>>>>>>>> ";
    }
}

bool SDL2Speaker::init(PcmFormatInfo info, const std::function<void(PcmFormatInfo)> &databack)
{

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        return false;
    }

    SDL_AudioSpec want;
    // SDL_AudioSpec have;
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
        if (speak_pcm_info.is_float == true)
        {
            want.format = AUDIO_F32;
        }
    }
    want.channels = speak_pcm_info.channels;
    want.samples = SDL_SAMPLES; // 每次回调的样本数
    // want.silence     = 0;                //静音值
    // want.size = 1024 * 2;

    pre_clock = std::chrono::high_resolution_clock::now();
    // want.callback = [](void *userdata, uint8_t *stream, int len)
    // {
    //     LOG(ERROR) << "len 1:" << len;
    //     SDL2Speaker *context = (SDL2Speaker *)userdata;
    //     // std::lock_guard<std::mutex> lock(context->outMutex);

    //     std::deque<std::unique_ptr<PcmObj>> &audio_queue = context->audio_queue;
    //     std::queue<std::unique_ptr<PcmObj>> &free_queue = context->free_queue;
    //     // PcmObj **tmp_obj = &context->tmp_obj;
    //     // 清除输出缓冲区
    //     memset(stream, 0, len);
    //     {
    //         // 调试
    //         // 1、每次回调得时间差
    //         // 2、回调得len
    //         // 3、计算样本数
    //         double duration_clock = 0;
    //         {
    //             // auto end = std::chrono::high_resolution_clock::now();
    //             // std::chrono::duration<double> elapsed = end - context->pre_clock;
    //             // duration_clock = elapsed.count();
    //         }
    //         // LOG(ERROR) << "duration_clock :" << duration_clock;
    //         // LOG(ERROR) << "len :" << len;
    //     }

    //     // LOG(ERROR) << "len :" << len;
    //     // 填充输出缓冲区，直到没有更多的数据或缓冲区已满
    //     while (len > 0 && !audio_queue.empty())
    //     {
    //         uint8_t *pcm_data = nullptr;
    //         size_t pcm_size = 0;

    //         std::unique_ptr<PcmObj> pcm_obj = std::move(audio_queue.front());
    //         audio_queue.pop_front();

    //         pcm_data = pcm_obj->mem;
    //         pcm_size = pcm_obj->data_size;

    //         size_t chunk_size = pcm_size * sizeof(uint8_t);
    //         if (chunk_size <= len)
    //         { // 复制整个obj的数据
    //             len -= chunk_size;

    //             // context->debug_outfile->write((const char*)pcm_data, chunk_size);
    //             context->total_samples -= pcm_obj->nb_samples;
    //             memcpy(stream, pcm_data, chunk_size);
    //             free_queue.push(std::move(pcm_obj));
    //         }
    //         else
    //         {
    //             size_t need_size = len;
    //             // context->debug_outfile->write((const char*)pcm_data, need_size);
    //             memcpy(stream, pcm_data, need_size);
    //             size_t last_size = chunk_size - need_size;
    //             len -= need_size;

    //             if (context->tmp_mem == nullptr || context->tmp_mem_size < last_size)
    //             {
    //                 if (context->tmp_mem != nullptr)
    //                 {
    //                     free(context->tmp_mem);
    //                     context->tmp_mem = nullptr;
    //                     context->tmp_mem_size = 0;
    //                 }
    //                 context->tmp_mem = (uint8_t *)malloc(sizeof(uint8_t) * last_size);
    //                 context->tmp_mem_size = last_size;
    //             }
    //             if (context->tmp_mem)
    //             {
    //                 memcpy(context->tmp_mem, pcm_data + need_size, last_size);
    //                 memcpy(pcm_obj->mem, context->tmp_mem, last_size);
    //                 pcm_obj->data_size = last_size;
    //                 audio_queue.push_front(std::move(pcm_obj));
    //             }
    //         }
    //     }
    //     LOG(ERROR) << "len 2:" << len;
    //     LOG(ERROR) << "";
    //     // context->pre_clock = std::chrono::high_resolution_clock::now();
    // };
    want.callback = [](void *userdata, uint8_t *stream, int len)
    {
        SDL2Speaker *context = (SDL2Speaker *)userdata;
        std::lock_guard<std::mutex> lock(context->outMutex);
        std::queue<std::unique_ptr<PcmMem>> &audio_queue = context->new_audio_queue;
        std::queue<std::unique_ptr<PcmMem>> &free_queue = context->new_free_queue;
        memset(stream, 0, len);
        while (len > 0 && !audio_queue.empty())
        {
            std::unique_ptr<PcmMem> pcm_obj = std::move(audio_queue.front());
            audio_queue.pop();
            context->debug_outfile->write((const char*)pcm_obj->mem, len);
            memcpy(stream, pcm_obj->mem, len);
            pcm_obj->data_size =0;
            free_queue.push(std::move(pcm_obj));

            len -= len;
        }
    };
    want.userdata = this;

    // if (SDL_OpenAudio(&want,&have) < 0)
    // {
    //     printf("fail to open audio\n");
    //     SDL_Quit();
    //     return false;
    // }
    // // 打开音频设备
    // if (!(dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE)))
    // {
    //     SDL_Quit();
    //     return false;
    // }
    if (!(dev = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0)))
    {
        SDL_Quit();
        return false;
    }
    // LOG(INFO) << "SDL freq: " << have.freq;
    // LOG(INFO) << "SDL format: " << have.format; // AUDIO_F32LSB 0x8120 33056 浮点小端格式
    // LOG(INFO) << "SDL channels: " << have.channels;
    // LOG(INFO) << "SDL size: " << have.size;
    // LOG(INFO) << "SDL samples: " << have.samples;

    // speak_pcm_info.sample_rate = have.freq;
    // speak_pcm_info.channels = have.channels;
    // switch (have.format)
    // {
    // case AUDIO_U8 | AUDIO_S8:
    //     speak_pcm_info.bitsSample = 8;
    //     speak_pcm_info.is_float = false;
    // case AUDIO_S16LSB:
    //     speak_pcm_info.bitsSample = 16;
    //     speak_pcm_info.is_float = false;
    //     break;
    // case AUDIO_S32LSB:
    //     speak_pcm_info.bitsSample = 32;
    //     speak_pcm_info.is_float = false;
    //     break;
    // case AUDIO_F32LSB:
    //     speak_pcm_info.bitsSample = 32;
    //     speak_pcm_info.is_float = true;
    //     LOG(INFO) << "  AUDIO_F32LSB ";
    //     break;
    // case AUDIO_F32MSB:
    //     speak_pcm_info.bitsSample = 32;
    //     speak_pcm_info.is_float = true;
    //     LOG(INFO) << "  AUDIO_F32MSB ";
    //     break;
    // default:
    //     break;
    // }
    // speak_pcm_info.nb_samples = have.samples;
    init_flag = true;
    databack(speak_pcm_info);

    debug_outfile = std::make_unique<std::ofstream>("out_data.pcm", std::ios::binary);
    if (!debug_outfile->is_open())
    { // 检查文件是否成功打开
        std::cerr << "Unable to open file for writing >>> out_data";
    }
    debug_outfile2 = std::make_unique<std::ofstream>("out_data2.pcm", std::ios::binary);
    if (!debug_outfile2->is_open())
    { // 检查文件是否成功打开
        std::cerr << "Unable to open file for writing >>> out_data";
    }
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
    info.channels = 1; // 写死1通道
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
        LOG(INFO) << "  AUDIO_F32LSB ";
        break;
    case AUDIO_F32MSB:
        info.bitsSample = 32;
        info.is_float = true;
        LOG(INFO) << "  AUDIO_F32MSB ";
        break;
    default:
        break;
    }

    // { // 写死进行调试
    //     info.bitsSample = 32;
    //     info.is_float = true;
    //     info.channels = 2;
    // }

    LOG(INFO) << "  info: " << info.toString();
    speak_pcm_info = info;
    databack(info);
}