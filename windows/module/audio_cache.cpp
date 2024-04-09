#include "audio_cache.h"
#include <glog/export.h>
#include <glog/logging.h>

// 在类外部定义 Singleton 的静态成员
std::mutex PCMCacheManager::Singleton::mtx;           // 静态成员 mtx 的定义
PCMCacheManager PCMCacheManager::Singleton::instance; // 静态成员 instance 的定义

PCMCacheManager::PCMCacheManager()
{
    // _mem = std::vector<uint8_t>(1024 * 1024 * 1);
    _mem = std::vector<uint8_t>(128 * 1024 * 1);
    reada_pos = write_pos = 0;
    obj_idx = 0;
};

PCMCacheManager::~PCMCacheManager()
{
    if(pcm_data!=nullptr){
        free(pcm_data);
        pcm_data = nullptr;
    }
    cvNotEmpty_.notify_all();
}

bool PCMCacheManager::isEmpty() const
{
    return reada_pos == write_pos;
}

bool PCMCacheManager::isBusy() 
{
    if((audio_pcm_total_size + tmp_pcm_size) > _mem.size()){
        return true;
    }
    int free_space = getFreeSpace();
    if(tmp_pcm_size != 0 && free_space > _mem.size()/2){ 
        LOG(INFO) << " isBusy ..... free_space:" << free_space;
        
        // 如果有临时数据，等待缓存空间释放出1半的时候，在存储，减少处理次数
        add(tmp_pcm_data, tmp_pcm_size, tmp_info);
        tmp_pcm_size = 0;
    }
    return (tmp_pcm_size != 0);
}

void PCMCacheManager::clear()
{
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
    if (write_pos >= reada_pos){
        used_space = write_pos - reada_pos;
        if(write_pos == reada_pos){
            used_space = audio_pcm_total_size;
        }
    }
    else
        used_space = (_mem.size() - reada_pos) + write_pos;

    // 计算空闲空间
    free_space = _mem.size() - used_space;
    return free_space;
}


bool PCMCacheManager::add(const uint8_t *data, int64_t size, PcmFormatInfo info)
{
    int64_t free_space = getFreeSpace();
    size_t free_end_ = _mem.size() - write_pos;
    // LOG(INFO) << "free_space:" << free_space << "   free_end_:" << free_end_ ;

    // 尾部空间不足, 需要 去除尾部空间，重新计算可用空间
    if (free_space >= size && free_end_ < size)
    {
        // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();
        // LOG(INFO) << "free_space:" << free_space;

        // free_space = free_space - free_end_;
        // 尾部空间不够存入当前数据，用空对象占位
        write_pos = 0;
        // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();
        std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
        dataObj->size = free_end_;
        audio_pcm_total_size += free_end_;
        // LOG(INFO) << "push 0    audio_pcm_total_size: " << audio_pcm_total_size;
        _pcmQueue.push(std::move(dataObj));
        // LOG(INFO) << "free cpy...";
        free_space = getFreeSpace();
    }

    // 空余空间能存放数据
    if (free_space >= size)
    {
        // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();
        // LOG(INFO) << "free_space:" << free_space;
        uint8_t *data_ptr = &_mem[write_pos];        
        const uint8_t* data_src = data; 
        
        if (data_src != nullptr && data_ptr != nullptr)
        {
            memcpy(data_ptr, data_src, size);
            // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();

            // _pcmQueue
            std::unique_ptr<AudioDataObj> dataObj = std::make_unique<AudioDataObj>();
            dataObj->data_ptr = data_ptr;
            dataObj->size = size;
            dataObj->info = info;
            dataObj->idx = obj_idx;
            dataObj->start_post = write_pos;
            obj_idx++;
            audio_pcm_total_size += size;
            // LOG(INFO) << "push 1    audio_pcm_total_size: " << audio_pcm_total_size;
            // LOG(INFO) <<" idx:" <<dataObj->idx << "  dataObj->info time_sec:"<< dataObj->info.time_sec;
            _pcmQueue.push(std::move(dataObj));

            write_pos += size;
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
                tmp_pcm_size = 0;
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
    if (!_pcmQueue.empty())
    {

        if (_pcmQueue.front()->data_ptr == nullptr)
        { // 尾部无效数据
            reada_pos += _pcmQueue.front()->size;
            if(reada_pos >= _mem.size()){
                reada_pos = 0;
            }
            audio_pcm_total_size -= _pcmQueue.front()->size;
            // LOG(WARNING) << " pop 0 audio_pcm_total_size: "<<audio_pcm_total_size;
            // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();
            _pcmQueue.pop();
        }
        if (!_pcmQueue.empty())
        {
            std::unique_ptr<AudioDataObj> dataObj = std::move(_pcmQueue.front());
            _pcmQueue.pop();
            // 取走数据后 返回内存
            reada_pos += dataObj->size;
            audio_pcm_total_size -= dataObj->size;
            // LOG(WARNING) << " pop 1 audio_pcm_total_size: "<<audio_pcm_total_size  ;
            // LOG(WARNING) << " _mem.size: "<<_mem.size();
            // LOG(WARNING) << " _pcmQueue.size: "<<_pcmQueue.size();
            
            // int64_t free_space = getFreeSpace();
            // LOG(WARNING) << " free_space.size: "<<free_space;
            // LOG(WARNING) << " ";
            if(reada_pos >= _mem.size()){
                reada_pos = 0;
            }
            // LOG(INFO) << "reada_pos:" << reada_pos << "  write_pos:" << write_pos << "   _mem:"<<_mem.size();

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
                // LOG(INFO) <<" get idx:" <<dataObj->idx << "  dataObj->info time_sec:"<< dataObj->info.time_sec;
                databack(pcm_data, dataObj->size, dataObj->info);
            }
            else
            {
                LOG(ERROR) << "Either pcm_data or frame->data[0] is null..............................";  
            }
        }
    }else{
        LOG(WARNING) << " _pcmQueue.empty: "<< _pcmQueue.size();
    }
}