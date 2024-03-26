#ifndef AUDIOMANAGER_H  
#define AUDIOMANAGER_H  
  
#include <memory>  
#include "audio_decoder.h"  
#include "audio_out.h"  
#include "audio_buffer.h"  
  
class AudioManager {  
private:  
    std::unique_ptr<AudioDecoder> decoder_;  
    std::unique_ptr<AudioOutput> output_;  
    std::unique_ptr<AudioBuffer> buffer_;  
  
public:  
    AudioManager(std::unique_ptr<AudioDecoder> decoder,  
                  std::unique_ptr<AudioOutput> output,  
                  std::unique_ptr<AudioBuffer> buffer);  
  
    void processAudioData(const std::uint8_t* data, size_t size);  
};  
  
#endif // AUDIOMANAGER_H
