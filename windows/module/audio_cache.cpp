#include "audio_cache.h"
#include <glog/export.h>
#include <glog/logging.h>

// 在类外部定义 Singleton 的静态成员
std::mutex PCMCacheManager::Singleton::mtx;           // 静态成员 mtx 的定义
PCMCacheManager PCMCacheManager::Singleton::instance; // 静态成员 instance 的定义

PCMCacheManager::PCMCacheManager()
{
    _mem = std::vector<uint8_t>(1024 * 1024 * 10);
    reada_pos = write_pos = 0;
};

PCMCacheManager::~PCMCacheManager()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cvNotEmpty_.notify_all();
}

bool PCMCacheManager::isEmpty() const
{
    return reada_pos == write_pos;
}

void PCMCacheManager::clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!_pcmQueue.empty())
    {
        _pcmQueue.pop();
    }
    reada_pos = write_pos = 0;
}

int64_t PCMCacheManager::getFreeSpace() const
{
    int64_t used_space, free_space;
    // 计算已使用空间
    if (write_pos >= reada_pos)
        used_space = write_pos - reada_pos;
    else
        used_space = (_mem.size() - reada_pos) + write_pos;

    // 计算空闲空间
    free_space = _mem.size() - used_space;
    return free_space;
}

void PCMCacheManager::add(const uint8_t *data, int64_t size, PcmFormatInfo info)
{
    LOG(INFO) << "add";
    std::unique_lock<std::mutex> lock(mutex_);

    int64_t free = getFreeSpace();
    LOG(INFO) << "free:" << free / 1024 << " KB";

    size_t free_end_ = _mem.size() - write_pos;
    LOG(INFO) << "free_end_:" << free_end_ << " B";

    // 尾部空间不足
    if (free_end_ < size)
    {
        free = free - free_end_;
    }
    if (free >= size)
    {
        if (free_end_ < size)
        {
            write_pos = 0;
            std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
            dataObj->size = free_end_;
            _pcmQueue.push(std::move(dataObj));
        }
        LOG(INFO) << "free cpy...";
        uint8_t *data_ptr = &_mem[write_pos];
        memcpy(&_mem[write_pos], data, size);
        write_pos += size;
        LOG(INFO) << "write_pos:" << write_pos;

        // _pcmQueue
        std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
        dataObj->data_ptr = data_ptr;
        dataObj->size = size;
        dataObj->info = info;
        _pcmQueue.push(std::move(dataObj));
    }
    else
    {
        LOG(INFO) << " no spase";
    }
}

void PCMCacheManager::get(const std::function<void(uint8_t *, int64_t, PcmFormatInfo)> &databack)
{
    LOG(INFO) << "get";
    /* code */
    std::unique_lock<std::mutex> lock(mutex_);
    if (!_pcmQueue.empty())
    {

        if (_pcmQueue.front()->data_ptr == nullptr)
        { // 尾部无效数据
            _pcmQueue.pop();
            reada_pos = 0;
        }
        if (!_pcmQueue.empty())
        {
            std::unique_ptr<AudioDataObj> dataObj = std::move(_pcmQueue.front());
            _pcmQueue.pop();

            // 音频播放

            // 播放完后 返回内存
            reada_pos += dataObj->size;
            LOG(INFO) << "reada_pos:" << reada_pos;
            //int sample_rate, int nChannels,  int bitsPerSample
            databack(dataObj->data_ptr, dataObj->size, dataObj->info);
        }
    }
}