#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
extern "C"
{
#include "libavcodec/avcodec.h"
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include "libavutil/time.h"
#include "libavutil/channel_layout.h"
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/timestamp.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>


#define my_av_ts2str(ts) \
  do { \
    char* buf = new char[AV_TS_MAX_STRING_SIZE]; \
    av_ts_make_string((char[AV_TS_MAX_STRING_SIZE]){0}, ts) \
    /* 使用 buf 做一些操作 */ \
    delete[] buf; \
  } while (0)

// #define my_av_ts2timestr(ts, tb) \
//   av_ts_make_time_string((char[AV_TS_MAX_STRING_SIZE]){0}, ts, tb)
#define my_av_ts2timestr(ts, tb) \
  do { \
    char* buf = new char[AV_TS_MAX_STRING_SIZE]; \
    av_ts_make_time_string(buf, ts, tb); \
    /* 使用 buf 做一些操作 */ \
    delete[] buf; \
  } while (0)

};

class AudioDecoder
{
public:
    virtual ~AudioDecoder() {}
    virtual bool initialize(const std::string &filePath, const std::function<void(double, double)> &callback) = 0;       // 初始化解码器
    virtual void set_pcm_back(const std::function<void(const uint8_t*, int64_t, bool, int)> &callback) = 0;  //const std::unique_ptr<const std::vector<float>>, bool
    virtual void release() = 0;                                     // 初始化解码器
    virtual bool decode() = 0; // 解码数据
    virtual bool seek(double value) = 0;
    virtual bool isInitialized() const = 0;                         // 检查解码器是否已初始化
};

class Mp3Decoder : public AudioDecoder
{
public:
    bool initialize(const std::string &filePath, const std::function<void(double, double)> &callback) override;
    void set_pcm_back(const std::function<void(const uint8_t*, int64_t, bool, int)> &callback) override;  
    void release() override;
    bool decode() override;
    bool seek(double value) override;
    bool isInitialized() const override;
    ~Mp3Decoder();
private:
    const char *inFileName = nullptr;
    std::function<void(double, double)> bk;
    std::function<void(const std::unique_ptr<const std::vector<float>>, bool, int)> _data_back = nullptr;
    std::function<void(const uint8_t*, int64_t, bool, int)> _u8data_back = nullptr;

    AVFormatContext *fmtCtx =  nullptr;
    AVCodecContext *codecCtx =  nullptr;
    AVPacket *pkt= nullptr;
    AVFrame *frame = nullptr;
    int aStreamIndex = -1;
    int64_t seek_time = 0;
    double seek_dec = 0;
};

#endif // DECODER_H