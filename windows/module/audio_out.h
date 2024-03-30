#ifndef AUDIOOUTPUT_H  
#define AUDIOOUTPUT_H  
  
#include <cstdint>  
class IAudioOutput {  
public:  
    virtual ~IAudioOutput() {}  
    virtual void play() = 0;  
};  
  
#endif // AUDIOOUTPUT_H