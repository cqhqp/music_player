#ifndef MP3DECODER_H  
#define MP3DECODER_H  
#include "audio_decoder.h"
class MP3Decoder : public AudioDecoder {  
public:  
    bool initialize() override;  
    void release() override;  
    void decode(const std::uint8_t* data, size_t size) override;  
    bool isInitialized() const override;  
    // ... MP3特定的实现 ...  
};  
  
#endif // MP3DECODER_H