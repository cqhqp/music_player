#ifndef PCM_FORMAT_H
#define PCM_FORMAT_H

#include <iomanip>  
#include <sstream>
#include <string>
  
class PcmFormatInfo
{
public:
    int nb_samples = 0; // 采样数
    int channels = 0;   // 通道
    int sample_rate =0; // 采样率
    bool is_float = false;  // u8 int float
    int bitsSample = 0; //采样位数
    bool planar = false;    // 平面还是交叉
    double time_sec = 0.0;
    double max_time_sec = 0.0; // 音频文件的最大时长
    std::string toString() {  
        std::ostringstream oss;  
        oss << "PcmFormatInfo{"  
            << "nb_samples=" << nb_samples << ", "  
            << "channels=" << channels << ", "  
            << "sample_rate=" << sample_rate << ", "  
            << "is_float=" << (is_float ? "true" : "false") << ", "  
            << "bitsSample=" << bitsSample << ", "  
            << "planar=" << (planar ? "true" : "false")  << ", "  
            << "time_sec=" << std::to_string(time_sec) << ", "  
            << "max_time_sec=" << std::to_string(max_time_sec) << ", "  
            << "}";  
        return oss.str();  
    }  
};

#endif