#include "audio_out.h"
#include <glog/export.h>
#include <glog/logging.h>
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>

PCMOutput::PCMOutput()
{
}

PCMOutput::~PCMOutput()
{
}

bool PCMOutput::isInit() const
{
    return init_flag;
}

void PCMOutput::play()
{
    // // 播放PCM数据
    // pDSBuffer->Lock(0, out_samples * 2 * 2, (void **)&out_buffer, nullptr, nullptr, 0, nullptr);
    // pDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
    // pDSBuffer->Unlock(out_buffer, out_samples * 2 * 2, nullptr, 0);
}

bool PCMOutput::init(PcmFormatInfo info)
{
    // 设置音频格式
    // if (isfloat)
    // {
    //     wfx.wBitsPerSample = info.bitsSample;
    //     wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    // }
    // else
    // {
    //     wfx.wFormatTag = WAVE_FORMAT_PCM;
    //     wfx.wBitsPerSample = 16;
    // }
    // wfx.nChannels = info.channels;
    // wfx.nSamplesPerSec = info.sample_rate; // 采样率
    // // wfx.nSamplesPerSec = 44100; // 采样率
    // // wfx.nAvgBytesPerSec = wfx.nChannels * wfx.nSamplesPerSec * 4;
    // wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
    // wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    // wfx.cbSize = 0;

    int sampleRate[] = {8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    int bitsPer[] = {8, 16, 32};
    // for (auto rate : sampleRate)
    for (UINT32 sampleRate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000})
    {
        LOG(WARNING) << "rate:" << sampleRate;
        for (auto bits : bitsPer)
        {
            LOG(WARNING) << "bits:" << bits;

            LPDIRECTSOUND pDSound = nullptr;
            DSBUFFERDESC dsbd;
            WAVEFORMATEX wfx;
            HRESULT hr;

            bool isfloat = true;
            LPDIRECTSOUNDBUFFER pDSBuffer = nullptr;
            // 初始化DirectSound
            hr = DirectSoundCreate(nullptr, &pDSound, nullptr);
            if (FAILED(hr))
            {
                // 处理错误
                LOG(ERROR) << "pDSound FAILED.";
                return false;
            }

            ZeroMemory(&wfx, sizeof(wfx));

            wfx.wFormatTag = WAVE_FORMAT_PCM;
            wfx.nChannels = info.channels;
            wfx.nSamplesPerSec = sampleRate; // ok
            wfx.wBitsPerSample = bits;
            wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
            wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

            // 设置DirectSound缓冲区描述
            ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
            // dsbd.dwSize = sizeof(DSBUFFERDESC);
            // dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCHARDWARE;
            // dsbd.dwBufferBytes = 4096; // 可以根据实际需要调整
            // dsbd.lpwfxFormat = &wfx;

            dsbd.dwSize = sizeof(DSBUFFERDESC);
            dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
            dsbd.dwBufferBytes = 4096;
            dsbd.lpwfxFormat = &wfx;

            // 创建DirectSound缓冲区
            hr = pDSound->CreateSoundBuffer(&dsbd, &pDSBuffer, nullptr);
            if (FAILED(hr))
            {
                // 处理错误
                pDSound->Release();
                LOG(ERROR) << info.toString();
                LOG(ERROR) << "CreateSoundBuffer FAILED.";
                return false;
            }
            else
            {
                LOG(WARNING) << "CreateSoundBuffer success."
                             << "  bits:" << bits << "  rate:" << sampleRate;
            }
        }
    }

    return true;
}

WASAPIOutput::WASAPIOutput()
{
}

WASAPIOutput::~WASAPIOutput()
{
}

void WASAPIOutput::play()
{
}

// #include <mmdeviceapi.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <devicetopology.h>
#include <propkey.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>

// <mmdeviceapi.h> 或 <functiondiscoverykeys_devpkey.h>。

// 辅助函数，用于检查支持的采样率
bool IsSampleRateSupported(IMMDevice *pDevice, UINT32 sampleRate)
{
    IAudioClient *pAudioClient = nullptr;
    // WAVEFORMATEX *pwfx = nullptr;
    HRESULT hr;

    // 定义要测试的音频格式
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(wfx));
    // wfx.wFormatTag = WAVE_FORMAT_PCM;
    // wfx.nChannels = 2; // 假设立体声
    // wfx.nSamplesPerSec = sampleRate;
    // wfx.wBitsPerSample = 16; // 假设16位
    // wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    // wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;               // 假设立体声
    wfx.nSamplesPerSec = sampleRate; // ok
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    // 尝试使用此格式创建音频客户端
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&pAudioClient);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to activate audio client.";
        goto done;
    }

    hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &wfx, NULL);
    if (SUCCEEDED(hr))
    {

        // 格式被支持
        return true;
    }
    else
    {
        LOG(ERROR) << "Failed to IsFormatSupported." << hr;
    }

done:
    if (pAudioClient)
    {
        pAudioClient->Release();
        pAudioClient = nullptr;
    }
    return false;
}

bool WASAPIOutput::init(PcmFormatInfo info)
{
    // HRESULT hr;
    // IMMDeviceEnumerator *pEnumerator = NULL;
    // IMMDevice *pDevice = NULL;
    // IPropertyStore *pProps = NULL;
    // PROPVARIANT propVar;
    // WAVEFORMATEX *pWaveFormat = NULL;

    // LOG(WARNING) << "WASAPIOutput::init.";

    // // Initialize COM
    // CoInitialize(NULL);
    // LOG(WARNING) << "WASAPIOutput::init. 2";

    // // Create device enumerator
    // hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    // LOG(WARNING) << "WASAPIOutput::init. 3";
    // if (FAILED(hr)) {
    //     LOG(ERROR) << "Failed to create device enumerator.";
    //     CoUninitialize();
    //     return false;
    // }
    // LOG(WARNING) << "WASAPIOutput::init. 4";

    // // Get default audio endpoint (speaker)
    // hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    // if (FAILED(hr)) {
    //     LOG(ERROR) << "Failed to get default audio endpoint.";
    //     pEnumerator->Release();
    //     CoUninitialize();
    //     return false;
    // }

    // LOG(WARNING) << "WASAPIOutput::init. 5";
    // // Open property store
    // hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    // if (FAILED(hr)) {
    //     LOG(ERROR) << "Failed to open property store.";
    //     pDevice->Release();
    //     pEnumerator->Release();
    //     CoUninitialize();
    //     return false;
    // }

    // // Get device format
    // hr = pProps->GetValue(PKEY_AudioEngine_DeviceFormat, &propVar);
    // if (FAILED(hr)) {
    //     LOG(ERROR) << "Failed to get device format.";
    //     pProps->Release();
    //     pDevice->Release();
    //     pEnumerator->Release();
    //     CoUninitialize();
    //     return false;
    // }

    // // Cast PROPVARIANT to WAVEFORMATEX
    // pWaveFormat = reinterpret_cast<WAVEFORMATEX*>(propVar.blob.pBlobData);

    // // Extract supported sample rates
    // DWORD dwSupportedSampleRates = pWaveFormat->nSamplesPerSec;

    // LOG(ERROR) << "Supported Sample Rate: " << dwSupportedSampleRates << " Hz";

    // // // Get supported formats
    // // hr = pProps->GetValue(PKEY_AudioEngine_SupportedFormats, &pWaveFormat);
    // // if (FAILED(hr)) {
    // //     LOG(ERROR) << "Failed to get supported formats.";
    // //     pProps->Release();
    // //     pDevice->Release();
    // //     pEnumerator->Release();
    // //     CoUninitialize();
    // //     return hr;
    // // }

    // // if (propVar.vt == VT_VECTOR | VT_UI4) {
    // //     PROPVARIANT *pVarArray = propVar.caub.pElems;
    // //     int count = propVar.caub.cElems / sizeof(UINT);

    // //     LOG(INFO) << "Supported Sample Rates:";
    // //     for (int i = 0; i < count; ++i) {
    // //         LOG(INFO) << pVarArray[i] << " Hz";
    // //     }
    // // }

    // // Clean up
    // // PropVariantClear(&propVar);

    // CoTaskMemFree(pWaveFormat);
    // pProps->Release();
    // pDevice->Release();
    // pEnumerator->Release();
    // CoUninitialize();

    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    WAVEFORMATEX *pWaveFormat = NULL;

    // Initialize COM
    CoInitialize(NULL);

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
    // hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&g_pGraphBuilder);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to create device enumerator.";
        CoUninitialize();
        return false;
    }

    // Get default audio endpoint (speaker)
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to get default audio endpoint.";
        pEnumerator->Release();
        CoUninitialize();
        return false;
    }

    // Activate audio client
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&pAudioClient);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to activate audio client.";
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return false;
    }

    // Get mix format
    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to get mix format.";
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return false;
    }

    if (pWaveFormat == NULL)
    {
        LOG(WARNING) << "Error: pWaveFormat pointer is NULL. ";
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return false;
    }

    // // 定义音频参数
    // WAVEFORMATEX wfx = {0};
    // wfx.wFormatTag = WAVE_FORMAT_PCM;                           // 使用PCM格式
    // wfx.nChannels = 2;                                          // 通道数，例如立体声为2
    // wfx.nSamplesPerSec = 48000;                                 // 采样率，例如44.1kHz
    // wfx.wBitsPerSample = 16;                                    // 每个样本的位数，例如16位PCM
    // wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels; // 块对齐
    // wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; // 平均每秒字节数
    // wfx.cbSize = 0;                                             // 对于PCM格式，cbSize为0
    // // 初始化音频客户端
    // hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,          // 共享模式
    //                               AUDCLNT_STREAMFLAGS_EVENTCALLBACK, // 流标志
    //                               0,                                 // 缓冲区持续时间（以100纳秒为单位），0表示使用默认值
    //                               0,                                 // 缓冲区大小（以帧为单位），0表示使用默认值
    //                               &wfx,                              // 音频格式
    //                               nullptr);                          // 默认的音频会话标识符

    WAVEFORMATEX *pClosestMatch = nullptr;

    LOG(WARNING) << "GetMixFormat nSamplesPerSec: " << pWaveFormat->nSamplesPerSec;
    LOG(WARNING) << "GetMixFormat wBitsPerSample: " << pWaveFormat->wBitsPerSample;
    LOG(WARNING) << "GetMixFormat wFormatTag: " << pWaveFormat->wFormatTag;
    LOG(WARNING) << "GetMixFormat nChannels: " << pWaveFormat->nChannels;
    // Get device supported sample rates
    for (UINT32 sampleRate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000})
    {
        pWaveFormat->nSamplesPerSec = sampleRate;
        pWaveFormat->nChannels = 1;
        pWaveFormat->wBitsPerSample = 64;
        if (pWaveFormat->wBitsPerSample < 32)
        {
            pWaveFormat->wFormatTag = WAVE_FORMAT_PCM;
            pWaveFormat->cbSize = 0;
            pWaveFormat->nBlockAlign = (pWaveFormat->wBitsPerSample / 8) * pWaveFormat->nChannels; // 块对齐
            pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign; // 平均每秒字节数
        }
        if (pWaveFormat->wBitsPerSample >= 32)
        {
            pWaveFormat->wFormatTag = WAVE_FORMAT_EXTENSIBLE;                                      // WAVE_FORMAT_IEEE_FLOAT
            pWaveFormat->nBlockAlign = (pWaveFormat->wBitsPerSample / 8) * pWaveFormat->nChannels; // 块对齐
            pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign; // 平均每秒字节数
        }
        // wfx.wFormatTag = WAVE_FORMAT_EXTENSIBLE; // WAVE_FORMAT_IEEE_FLOAT WAVE_FORMAT_PCM WAVE_FORMAT_EXTENSIBLE
        // pWaveFormat->nBlockAlign = pWaveFormat->wBitsPerSample / 8 * pWaveFormat->nChannels;
        // pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign;
        hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pWaveFormat, &pClosestMatch);
        if (hr == S_OK)
        {
            LOG(WARNING) << "IsFormatSupported Rate: " << sampleRate << " Hz ............................................";
            // LOG(WARNING) << "pWaveFormat->nSamplesPerSec: " << sampleRate;
            // LOG(WARNING) << "pWaveFormat->nChannels: " << pWaveFormat->nChannels;
            // LOG(WARNING) << "pWaveFormat->wBitsPerSample: " << pWaveFormat->wBitsPerSample;
            // LOG(WARNING) << "pWaveFormat->wFormatTag: " << pWaveFormat->wFormatTag;
            // LOG(WARNING) << "pWaveFormat->nBlockAlign: " << pWaveFormat->nBlockAlign;
            // LOG(WARNING) << "pWaveFormat->nAvgBytesPerSec: " << pWaveFormat->nAvgBytesPerSec;
        }
        else if (hr == S_FALSE) // 同格式有支持的参数
        {
            LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            LOG(WARNING) << "IsFormatSupported Err: S_FALSE.";
        }
        else if (hr == E_INVALIDARG) // 参数有错
        {
            // AUDCLNT_E_UNSUPPORTED_FORMAT
            LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            LOG(WARNING) << "One or more arguments are invalid.";
        }
        else if (hr = 0x88890008L) // 参数有错
        {
            LOG(WARNING) << "IsFormatSupported Err: " << hr;
            LOG(WARNING) << "设备配置异常";
        }
        else
        {
            LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            LOG(WARNING) << "IsFormatSupported Err: " << hr;
        }

        if (hr != S_OK && pClosestMatch)
        {
            LOG(WARNING) << "pClosestMatch->nSamplesPerSec: " << pClosestMatch->nSamplesPerSec;
        }
    }

    // Clean up
    if (pAudioClient)
        pAudioClient->Release();
    if (pDevice)
        pDevice->Release();
    if (pEnumerator)
        pEnumerator->Release();
    CoUninitialize();

    return false;
}

bool WASAPIOutput::isInit() const
{
    return init_flag;
    ;
}

void WASAPIOutput::init_format()
{

    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pDevice = NULL;
    IAudioClient *pAudioClient = NULL;
    WAVEFORMATEX *pWaveFormat = NULL;

    // Initialize COM
    CoInitialize(NULL);

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
    // hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&g_pGraphBuilder);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to create device enumerator.";
        CoUninitialize();
        return;
    }

    // Get default audio endpoint (speaker)
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to get default audio endpoint.";
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    // Activate audio client
    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&pAudioClient);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to activate audio client.";
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    // Get mix format
    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr))
    {
        LOG(ERROR) << "Failed to get mix format.";
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    if (pWaveFormat == NULL)
    {
        LOG(WARNING) << "Error: pWaveFormat pointer is NULL. ";
        pAudioClient->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return;
    }
    else
    {
        LOG(WARNING) << "GetMixFormat nSamplesPerSec: " << pWaveFormat->nSamplesPerSec; // 采样率
        LOG(WARNING) << "GetMixFormat wBitsPerSample: " << pWaveFormat->wBitsPerSample; // 采样位
        LOG(WARNING) << "GetMixFormat wFormatTag: " << pWaveFormat->wFormatTag;         // 格式
        LOG(WARNING) << "GetMixFormat nChannels: " << pWaveFormat->nChannels;           // 通道数
        if (pWaveFormat->wFormatTag == WAVE_FORMAT_PCM)                                 // WAVE_FORMAT_IEEE_FLOAT WAVE_FORMAT_PCM WAVE_FORMAT_EXTENSIBLE
        {
            // int
        }

        if (pWaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE || pWaveFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            // float
        }
    }

    // Clean up
    if (pAudioClient)
        pAudioClient->Release();
    if (pDevice)
        pDevice->Release();
    if (pEnumerator)
        pEnumerator->Release();
    CoUninitialize();
}

// **************************************************************************
CoreSpeaker::CoreSpeaker()
{
    EnumerateAudioOutputDevices(); // 枚举音频输出设备
    GetDefaultAudioOutputDeviceID();
}

CoreSpeaker::~CoreSpeaker()
{
}

void CoreSpeaker::play()
{
}

bool CoreSpeaker::init(PcmFormatInfo info)
{
    return false;
}

bool CoreSpeaker::isInit() const
{
    return false;
}

void CoreSpeaker::EnumerateAudioOutputDevices()
{
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDeviceCollection *pDevices = nullptr;
    UINT deviceCount = 0;
    HRESULT hr = S_OK;

    // 初始化COM库
    CoInitialize(nullptr);

    // 创建IMMDeviceEnumerator实例
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (SUCCEEDED(hr))
    {
        // 枚举音频输出设备
        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
        if (SUCCEEDED(hr))
        {
            // 获取设备数量
            hr = pDevices->GetCount(&deviceCount);
            if (SUCCEEDED(hr))
            {
                for (UINT i = 0; i < deviceCount; ++i)
                {
                    IMMDevice *pDevice = nullptr;
                    // 获取设备实例
                    hr = pDevices->Item(i, &pDevice);
                    if (SUCCEEDED(hr))
                    {
                        IPropertyStore *pProps = nullptr;
                        // 获取设备属性存储
                        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
                        if (SUCCEEDED(hr))
                        {
                            PROPVARIANT varName;
                            PropVariantInit(&varName);
                            // 获取设备友好名称
                            hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                            if (SUCCEEDED(hr))
                            {
                                // wprintf(L"Device Name: %s\n", varName.pwszVal);

                                int utf8Length = WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                                if (utf8Length > 0)
                                {
                                    std::vector<char> utf8Buffer(utf8Length);
                                    WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, &utf8Buffer[0], utf8Length, nullptr, nullptr);
                                    std::string deviceName = std::string(utf8Buffer.begin(), utf8Buffer.end() - 1); // 减1是为了去掉结尾的null字符
                                    LOG(ERROR) << "deviceName 1:" << deviceName;
                                }

                                PropVariantClear(&varName);
                            }
                            pProps->Release();
                        }
                        pDevice->Release();
                    }
                }
            }
            pDevices->Release();
        }
        pEnumerator->Release();
    }

    // 卸载COM库
    CoUninitialize();
}

void CoreSpeaker::GetDefaultAudioOutputDeviceID()
{
    IMMDeviceEnumerator *pEnumerator = nullptr;
    IMMDevice *pDefaultDevice = nullptr;
    IPropertyStore *pProps = nullptr;
    PROPVARIANT varName;
    HRESULT hr = S_OK;

    // 初始化COM库
    CoInitialize(nullptr);

    // 创建IMMDeviceEnumerator实例
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (SUCCEEDED(hr))
    {
        // 获取默认音频播放设备
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultDevice);
        if (SUCCEEDED(hr))
        {
            // 打开设备的属性存储
            hr = pDefaultDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (SUCCEEDED(hr))
            {
                // 获取设备的ID（字符串形式）
                // hr = pProps->GetValue(PKEY_Device_DeviceDesc, &varName);
                // PKEY_Device_ConfigFlags
                // PKEY_Device_DeviceDesc      耳机
                // PKEY_Device_FriendlyName    耳机 (OpenFit by Shokz Stereo)
                hr = pProps->GetValue(PKEY_Device_ConfigFlags, &varName); // PKEY_Device_DeviceDesc
                if (SUCCEEDED(hr) && varName.pwszVal != nullptr)
                {
                    // wprintf(L"Device Name: %s\n", varName.pwszVal);

                    // int size_needed = WideCharToMultiByte(CP_ACP, 0, varName.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                    // std::vector<char> buffer(size_needed);
                    // WideCharToMultiByte(CP_ACP, 0, varName.pwszVal, -1, &buffer[0], size_needed, nullptr, nullptr);
                    // std::string deviceName = std::string(buffer.begin(), buffer.end() - 1); // 减1是为了去掉结尾的null字符
                    // LOG(ERROR) << "deviceName:" << deviceName;

                    wprintf(L"Device Name: %s\n", varName.pwszVal);
                    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, nullptr, 0, nullptr, nullptr);
                    if (utf8Length > 0)
                    {
                        std::vector<char> utf8Buffer(utf8Length);
                        WideCharToMultiByte(CP_UTF8, 0, varName.pwszVal, -1, &utf8Buffer[0], utf8Length, nullptr, nullptr);
                        std::string deviceName = std::string(utf8Buffer.begin(), utf8Buffer.end() - 1); // 减1是为了去掉结尾的null字符
                        LOG(ERROR) << "deviceName:" << deviceName;
                    }
                }
                PropVariantClear(&varName);
                pProps->Release();
            }
            pDefaultDevice->Release();
        }
        pEnumerator->Release();
    }

    // 卸载COM库
    CoUninitialize();
}

#include <xaudio2.h>

XAudio2Speaker::XAudio2Speaker()
{
    IXAudio2 *pXAudio2 = nullptr;
    if (FAILED(XAudio2Create(&pXAudio2, 0)))
    {
        return;
    }

    // 释放XAudio2实例
    pXAudio2->Release();
}

XAudio2Speaker::~XAudio2Speaker()
{
}

void XAudio2Speaker::play()
{
}

bool XAudio2Speaker::init(PcmFormatInfo info)
{
    return false;
}

bool XAudio2Speaker::isInit() const
{
    return false;
}

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