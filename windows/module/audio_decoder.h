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
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/timestamp.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>  

#define my_av_ts2str(ts)                                                                     \
  do                                                                                         \
  {                                                                                          \
    char *buf = new char[AV_TS_MAX_STRING_SIZE];                                             \
    av_ts_make_string((char[AV_TS_MAX_STRING_SIZE]){0}, ts) /* 使用 buf 做一些操作 */ \
        delete[] buf;                                                                        \
  } while (0)

// #define my_av_ts2timestr(ts, tb) \
//   av_ts_make_time_string((char[AV_TS_MAX_STRING_SIZE]){0}, ts, tb)
#define my_av_ts2timestr(ts, tb)                 \
  do                                             \
  {                                              \
    char *buf = new char[AV_TS_MAX_STRING_SIZE]; \
    av_ts_make_time_string(buf, ts, tb);         \
    /* 使用 buf 做一些操作 */             \
    delete[] buf;                                \
  } while (0)
};
#include "pcm_format.h"

class IAudioDecoder
{
public:
  virtual ~IAudioDecoder() {}
  virtual bool initialize(const std::string &filePath, const std::function<void(double, double)> &callback) = 0; // 初始化解码器
  virtual void set_pcm_back(const std::function<void(const uint8_t *, int64_t, PcmFormatInfo)> &callback) = 0;   // const std::unique_ptr<const std::vector<float>>, bool
  virtual void release() = 0;                                                                                    // 初始化解码器
  virtual bool decode() = 0;                                                                                     // 解码数据
  virtual bool seek(double value) = 0;
  virtual bool isInitialized() const = 0; // 检查解码器是否已初始化
  virtual void setSpeakerInfo(PcmFormatInfo info) = 0; 
};

class Mp3Decoder : public IAudioDecoder
{
public:
  bool initialize(const std::string &filePath, const std::function<void(double, double)> &callback) override;
  void set_pcm_back(const std::function<void(const uint8_t *, int64_t, PcmFormatInfo)> &callback) override;
  void release() override;
  bool decode() override;
  bool seek(double value) override;
  bool isInitialized() const override;
  void setSpeakerInfo(PcmFormatInfo info) override;
  Mp3Decoder();
  ~Mp3Decoder();

private:
  const char *inFileName = nullptr;
  std::function<void(double, double)> bk;
  std::function<void(const std::unique_ptr<const std::vector<float>>, bool, int)> _data_back = nullptr;
  std::function<void(const uint8_t *, int64_t, PcmFormatInfo)> _u8data_back = nullptr;

  AVFormatContext *fmtCtx = nullptr;
  AVCodecContext *codecCtx = nullptr;
  AVPacket *pkt = nullptr;
  AVFrame *frame = nullptr;
  int aStreamIndex = -1;
  int64_t seek_time = 0;
  double seek_dec = 0;
   
  PcmFormatInfo speakerInfo;

  // 重采样
  SwrContext *swr_ctx = nullptr;  
  AVSampleFormat dst_sample_fmt;
  uint8_t **dst_data = nullptr;
  int max_dst_nb_samples = 0;
  int64_t out_pts = 0;
  double out_sec_one = 0;
  int64_t pre_pts = 0;
  int64_t pre_frame_pts = 0;

  // std::shared_ptr<std::vector<uint8_t>> pcm_data = std::make_shared<std::vector<uint8_t>>();
   uint8_t* data_dst= nullptr;
   int data_dst_size = 0;

   void init_swr();
};

#endif // DECODER_H