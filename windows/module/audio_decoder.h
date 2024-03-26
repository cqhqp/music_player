#ifndef DECODER_H  
#define DECODER_H  
  
#include <cstdint>  
class AudioDecoder {  
public:  
    virtual ~AudioDecoder() {}  
    virtual bool initialize() = 0; // 初始化解码器  
    virtual void release() = 0; // 初始化解码器  
    virtual void decode(const std::uint8_t* data, size_t size) = 0; // 解码数据  
    virtual bool isInitialized() const = 0; // 检查解码器是否已初始化  
};  
  
#endif // DECODER_H