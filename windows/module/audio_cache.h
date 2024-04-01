#ifndef AUDIOCACHE_H
#define AUDIOCACHE_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <cstdint>
#include <queue>
#include <functional>
#include "pcm_format.h"

// enum AudioCacheState {
//     AC_EMPTY    = 0,
//     AC_FULL     = 1, ///< Round toward zero.
//     AC_FREE     = 2,
// };

class IAudioCache
{
public:
    virtual ~IAudioCache() {}
    virtual bool isEmpty() const = 0;
    virtual bool isBusy() = 0;
    virtual void clear() = 0;
    virtual bool add(const uint8_t *data, int64_t size, PcmFormatInfo info) = 0;
    virtual void get(const std::function<void(const uint8_t*, int64_t, PcmFormatInfo)> &databack) = 0;
};

class AudioDataObj
{
public:
    // int64_t start;
    // int64_t end;
    uint8_t *data_ptr = nullptr;
    int64_t size;
    PcmFormatInfo info;
};

class PCMCacheManager : public IAudioCache
{
public:
    bool isEmpty() const;
    bool isBusy();
    void clear() override;
    bool add(const uint8_t *data, int64_t size, PcmFormatInfo info) override;
    void get(const std::function<void(const uint8_t*, int64_t, PcmFormatInfo)> &databack) override;

    // 提供一个公共的静态方法来获取Foo的单例对象
    static PCMCacheManager &getInstance()
    {
        return Singleton::getInstance();
    };

private:
    mutable std::mutex mutex_;           // 互斥锁，用于线程同步
    std::condition_variable cvNotEmpty_; // 条件变量，用于等待数据可读
    std::condition_variable cvFull_;     // 条件变量，用于等待空间可写
    std::queue<std::unique_ptr<AudioDataObj>> _pcmQueue;
    std::vector<uint8_t> _mem;
    int64_t reada_pos;
    int64_t write_pos;
    uint8_t *pcm_data = nullptr;
    int pcm_data_maxsize = 0;
    uint8_t *tmp_pcm_data = nullptr; // 当空间不足时，保持到临时空间中，等待下次空间充足后，在使用
    int tmp_pcm_size = 0; //  
    int tmp_pcm_maxsize = 0; // 
    PcmFormatInfo tmp_info;

    int64_t getFreeSpace() const;

    PCMCacheManager();
    ~PCMCacheManager();
    PCMCacheManager(const PCMCacheManager &) = delete;
    PCMCacheManager &operator=(const PCMCacheManager &) = delete;

    // 单例类，声明为PCMCacheManager的友元
    class Singleton
    {
    private:
        static PCMCacheManager instance;
        static std::mutex mtx;

        Singleton() = delete;
        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;

    public:
        static PCMCacheManager &getInstance()
        {
            std::lock_guard<std::mutex> lock(mtx);
            return instance;
        }

        // 友元声明，允许Singleton访问PCMCacheManager的私有构造函数
        friend class PCMCacheManager;
    };
};
#endif // AUDIOCACHE_H