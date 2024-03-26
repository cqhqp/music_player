#ifndef AUDIOOUTPUT_H  
#define AUDIOOUTPUT_H  
  
#include <cstdint>  
class AudioOutput {  
public:  
    virtual ~AudioOutput() {}  
    virtual void play(const uint8_t* data, size_t size) = 0;  
};  
  
#endif // AUDIOOUTPUT_H