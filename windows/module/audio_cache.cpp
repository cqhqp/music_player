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
    if(pcm_data!=nullptr){
        free(pcm_data);
        pcm_data = nullptr;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    cvNotEmpty_.notify_all();
}

bool PCMCacheManager::isEmpty() const
{
    return reada_pos == write_pos;
}

bool PCMCacheManager::isBusy() 
{
    int free_space = getFreeSpace();
    if(tmp_pcm_size != 0 && free_space > _mem.size()/2){ 
        // 如果有临时数据，等待缓存空间释放出1半的时候，在存储，减少处理次数
        add(tmp_pcm_data, tmp_pcm_size, tmp_info);
        tmp_pcm_size = 0;
    }
    return (tmp_pcm_size != 0);
}

void PCMCacheManager::clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!_pcmQueue.empty())
    {
        _pcmQueue.pop();
    }
    reada_pos = write_pos = 0; // 释放空间
    tmp_pcm_size = 0;          // 释放临时pcm空间
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


bool PCMCacheManager::add(const uint8_t *data, int64_t size, PcmFormatInfo info)
{
    // if(tmp_pcm_size != 0){ 
    //     // 如果有临时数据，等待缓存空间释放出1半的时候，在存储，减少处理次数
    //     add(tmp_pcm_data, tmp_pcm_size, tmp_info);
    //     tmp_pcm_size = 0;
    // }

    // LOG(INFO) << "add";
    std::unique_lock<std::mutex> lock(mutex_);

    int64_t free_space = getFreeSpace();
    // LOG(INFO) << "free:" << free / 1024 << " KB";

    size_t free_end_ = _mem.size() - write_pos;
    // LOG(INFO) << "free_end_:" << free_end_ << " B";

    // 尾部空间不足, 需要 去除尾部空间，重新计算可用空间
    if (free_end_ < size)
    {
        free_space = free_space - free_end_;
    }

    // 空余空间能存放数据
    if (free_space >= size)
    {
        if (free_end_ < size)
        {
            write_pos = 0;
            std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
            dataObj->size = free_end_;
            _pcmQueue.push(std::move(dataObj));
        }
        // LOG(INFO) << "free cpy...";

        uint8_t *data_ptr = &_mem[write_pos];        
        const uint8_t* data_src = data; 
        if (data_src != nullptr && data_ptr != nullptr)
        {
            memcpy(data_ptr, data_src, size);

            write_pos += size;
            // LOG(INFO) << "write_pos:" << write_pos;

            // _pcmQueue
            std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
            dataObj->data_ptr = data_ptr;
            dataObj->size = size;
            dataObj->info = info;
            _pcmQueue.push(std::move(dataObj));
        }
        else
        {
            LOG(ERROR) << "pcm data_src is null";
        }
        return true;
    }
    else
    {
        // 空间不够，需要把临时数据保存下来
        LOG(WARNING) << " no spase";
        LOG(WARNING) << " _pcmQueue size:" << _pcmQueue.size();
        LOG(WARNING) << " _pcmQueue time_sec:" << info.time_sec - _pcmQueue.front()->info.time_sec;

        if (tmp_pcm_maxsize < size)
        {                                
            // pcm_data->resize(all_data_size);
            // LOG(INFO) << "pcm_data size init.";
            if(tmp_pcm_data!=nullptr){
                free(tmp_pcm_data);
                tmp_pcm_data = nullptr;
            }
            tmp_pcm_data = (uint8_t* )malloc(sizeof(uint8_t) * size);
            tmp_pcm_maxsize = size;
        }   
        if (tmp_pcm_data != nullptr && data != nullptr){
            memcpy(tmp_pcm_data, data, size);
            tmp_pcm_size = size;
            tmp_info = info;
        }else{
            LOG(WARNING) << " 临时数据无法申请内存";
        }
        return false;
    }
}

void PCMCacheManager::get(const std::function<void(const uint8_t*, int64_t, PcmFormatInfo)> &databack)
{
    // LOG(INFO) << "get";
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
            // LOG(INFO) << "reada_pos:" << reada_pos;
            // int sample_rate, int nChannels,  int bitsPerSample          
            
            if (pcm_data_maxsize < dataObj->size)
            {                                
                // pcm_data->resize(all_data_size);
                // LOG(INFO) << "pcm_data size init.";
                if(pcm_data!=nullptr){
                    free(pcm_data);
                    pcm_data = nullptr;
                }
                pcm_data = (uint8_t* )malloc(sizeof(uint8_t) * dataObj->size);
                pcm_data_maxsize = dataObj->size;
            }            
            uint8_t* data_dst = pcm_data; 
            if (data_dst != nullptr && dataObj->data_ptr != nullptr)
            {
                memcpy(data_dst, dataObj->data_ptr, dataObj->size);
                databack(pcm_data, dataObj->size, dataObj->info);
            }
            else
            {
                LOG(ERROR) << "Either pcm_data or frame->data[0] is null..............................";  
            }
        }
    }
}