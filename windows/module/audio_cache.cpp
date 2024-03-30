#include "audio_cache.h"
#include <glog/export.h>
#include <glog/logging.h>

// 在类外部定义 Singleton 的静态成员
std::mutex PCMCacheManager::Singleton::mtx;        // 静态成员 mtx 的定义
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
    return false;
}

int64_t PCMCacheManager::getFreeSpace() const
{  
    int64_t used_space,free_space;
    // 计算已使用空间  
    if(write_pos >= reada_pos) 
        used_space = write_pos - reada_pos;  
    else  
        used_space = (_mem.size() - reada_pos) + write_pos;  
      
    // 计算空闲空间  
    free_space = _mem.size() - used_space;
    return free_space;
}

void PCMCacheManager::add(const uint8_t *data, int64_t size, bool planar, int channels)
{
    LOG(INFO) << "add";
    int64_t free = getFreeSpace();
    LOG(INFO) << "free:"<< free/1024 << " KB";
    if(free >= size){
        LOG(INFO) << "free cpy...";
        memcpy(&_mem[write_pos], data, size);
        write_pos += size;
    }else{
        LOG(INFO) << " no spase";
    }
}