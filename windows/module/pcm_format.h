#ifndef PCM_FORMAT_H
#define PCM_FORMAT_H

#include <iomanip>  
#include <sstream>
  
class PcmFormatInfo
{
public:
    int nb_samples; // 采样数
    int channels;   // 通道
    int sample_rate; // 采样率
    bool is_float;  // u8 int float
    int bitsSample; //采样位数
    bool planar;    // 平面还是交叉
    std::string toString() {  
        std::ostringstream oss;  
        oss << "PcmFormatInfo{"  
            << "nb_samples=" << nb_samples << ", "  
            << "channels=" << channels << ", "  
            << "sample_rate=" << sample_rate << ", "  
            << "is_float=" << (is_float ? "true" : "false") << ", "  
            << "bitsSample=" << bitsSample << ", "  
            << "planar=" << (planar ? "true" : "false")  
            << "}";  
        return oss.str();  
    }  
};

#endif