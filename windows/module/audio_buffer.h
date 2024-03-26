#ifndef AUDIOBUFFER_H  
#define AUDIOBUFFER_H  
  
#include <memory>  
  
class AudioBuffer {  
public:  
    virtual ~AudioBuffer() {}  
    virtual void enqueue(const std::uint8_t* data, size_t size) = 0;  
    virtual std::unique_ptr<std::uint8_t[]> dequeue(size_t& size) = 0;  
};  
  
#endif // AUDIOBUFFER_H