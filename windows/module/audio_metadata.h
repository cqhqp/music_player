#ifndef AUDIOMETADATA_H  
#define AUDIOMETADATA_H  

#include <string>  
#include <optional>  
#include <functional>

class AudioMetadata {  
public:  
    // 构造函数，可能需要传入音频文件路径  
    AudioMetadata(const std::string& filePath);  
  
    // 析构函数  
    ~AudioMetadata();  
  
    // 获取标题  
    std::optional<std::string> getTitle() const;  
  
    // 获取艺术家  
    std::optional<std::string> getArtist() const;  
  
    // 获取专辑  
    std::optional<std::string> getAlbum() const;  
  
    // 获取持续时间（以秒为单位）  
    std::optional<double> getDuration() const;  
  
    // 获取比特率（以比特每秒为单位）  
    std::optional<int> getBitrate() const;  
  
    // 获取采样率（以赫兹为单位）  
    std::optional<int> getSampleRate() const;  
  
    // 其他可能的元数据获取方法...  
    void parse(const std::function<void(std::string, std::string)>& callback);
  
private:  
    // 私有成员变量，用于存储音频文件路径和元数据  
    std::string filePath;  
    // 这里可以添加其他私有成员变量来存储解析后的元数据  

    // 禁止拷贝构造和赋值操作  
    AudioMetadata(const AudioMetadata&) = delete;  
    AudioMetadata& operator=(const AudioMetadata&) = delete;  
};

#endif