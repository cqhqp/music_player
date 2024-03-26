
#include <iostream>  
#include <vector>  
  

class MP3Demuxer {  
public:  
    MP3Demuxer(const char* filePath) {  
    }  
  
    ~MP3Demuxer() {  
    }  
  
    bool ReadFrame() {  
        return false;  
    }  
  
private:  
    int audioStream = -1;  
};  
