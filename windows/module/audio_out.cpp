#include "audio_out.h"
#include <glog/export.h>
#include <glog/logging.h>

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
    for (auto rate : sampleRate)
    {
        LOG(WARNING) << "rate:" << rate;
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
            wfx.nSamplesPerSec = rate; // ok
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
            LOG(WARNING) << "CreateSoundBuffer success."
                         << "  bits:" << bits << "  rate:" << rate;
            ;
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
#include <Mmdeviceapi.h>
#include <audioclient.h>
#include <devicetopology.h>
#include <propkey.h>
#include <functiondiscoverykeys_devpkey.h>
// <mmdeviceapi.h> 或 <functiondiscoverykeys_devpkey.h>。

// 辅助函数，用于检查支持的采样率
bool IsSampleRateSupported(IMMDevice *pDevice, UINT32 sampleRate)
{
    IAudioClient *pAudioClient = nullptr;
    WAVEFORMATEX *pwfx = nullptr;
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

    // //     std::cout << "Supported Sample Rates:";
    // //     for (int i = 0; i < count; ++i) {
    // //         std::cout << pVarArray[i] << " Hz";
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
    // WAVEFORMATEXTENSIBLE *pWaveFormat = NULL;
    // WAVEFORMATEXTENSIBLE* pWaveFormatExtensible = new WAVEFORMATEXTENSIBLE();
    WAVEFORMATEXTENSIBLE wfx = {0};  


    // Initialize COM
    CoInitialize(NULL);

    // Create device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);
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

    // 检查一系列采样率是否被支持，并输出支持的采样率
    // for (UINT32 sampleRate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000})
    // {
    //     if (IsSampleRateSupported(pDevice, sampleRate))
    //     {
    //         LOG(WARNING) << "Supported Sample Rate: " << sampleRate << " Hz";
    //     }
    //     else
    //     {
    //         LOG(WARNING) << "Not Supported Sample Rate: " << sampleRate << " Hz";
    //     }
    // }

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

    WAVEFORMATEX *pClosestMatch = nullptr;

    // // Get device supported sample rates
    // for (DWORD sampleRate = 8000; sampleRate <= 192000; sampleRate += 1000) {
    for (UINT32 sampleRate : {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000})
    {
        pWaveFormat->nSamplesPerSec = sampleRate;
        // pWaveFormat->nChannels = 1;
        // pWaveFormat->wBitsPerSample = 8;
        pWaveFormat->wFormatTag = WAVE_FORMAT_EXTENSIBLE; // WAVE_FORMAT_IEEE_FLOAT WAVE_FORMAT_PCM WAVE_FORMAT_EXTENSIBLE
        // pWaveFormat->nBlockAlign = pWaveFormat->wBitsPerSample / 8 * pWaveFormat->nChannels;
        // pWaveFormat->nAvgBytesPerSec = pWaveFormat->nSamplesPerSec * pWaveFormat->nBlockAlign;
        hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pWaveFormat, &pClosestMatch);
        if (hr == S_OK)
        {
            LOG(WARNING) << "IsFormatSupported Rate: " << sampleRate << " Hz";
            // LOG(WARNING) << "pWaveFormat->nSamplesPerSec: " << sampleRate;
            // LOG(WARNING) << "pWaveFormat->nChannels: " << pWaveFormat->nChannels;
            // LOG(WARNING) << "pWaveFormat->wBitsPerSample: " << pWaveFormat->wBitsPerSample;
            // LOG(WARNING) << "pWaveFormat->wFormatTag: " << pWaveFormat->wFormatTag;
            // LOG(WARNING) << "pWaveFormat->nBlockAlign: " << pWaveFormat->nBlockAlign;
            // LOG(WARNING) << "pWaveFormat->nAvgBytesPerSec: " << pWaveFormat->nAvgBytesPerSec;
        }
        else if (hr == S_FALSE)
        {
            // LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            // LOG(WARNING) << "IsFormatSupported Err: S_FALSE.";
        }
        else if (hr == E_INVALIDARG)
        {
            // LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            // LOG(WARNING) << "One or more arguments are invalid.";
        }
        else
        {
            // LOG(WARNING) << "IsFormatSupported sampleRate: " << sampleRate;
            // LOG(WARNING) << "IsFormatSupported Err: " << hr;
        }
    }
    // hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pWaveFormat, &pClosestMatch);
    // if (hr == S_OK)
    // {
    //     LOG(WARNING) << "IsFormatSupported Rate: " << pWaveFormat->nSamplesPerSec << " Hz";
    // }
    // else
    // {
    //     LOG(WARNING) << "IsFormatSupported Err: " << hr;
    // }

    // if (hr == E_POINTER)
    // {
    //     LOG(WARNING) << "IsFormatSupported E_POINTER ";
    // }
    // // Extract supported sample rate
    // DWORD dwSupportedSampleRate = pWaveFormat->nSamplesPerSec;
    // LOG(WARNING) << "Supported Sample Rate: " << dwSupportedSampleRate << " Hz";

    // Clean up
    CoTaskMemFree(pWaveFormat);
    if (pAudioClient)
        pAudioClient->Release();
    if (pDevice)
        pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    return false;
}

bool WASAPIOutput::isInit() const
{
    return init_flag;
    ;
}
