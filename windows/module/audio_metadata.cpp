#include "audio_metadata.h"
// #include "logger.h"

#include <iostream>
extern "C"
{
#include "libavcodec/avcodec.h"
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
};

#include <glog/export.h>
#include <glog/logging.h>

AudioMetadata::AudioMetadata(const std::string &filePath)
{
    this->filePath = filePath;
}

// 析构函数
AudioMetadata::~AudioMetadata()
{
}

void AudioMetadata::parse(const std::function<void(std::string, std::string)>& callback)
{
    do
    {
      AVFormatContext *fmt_ctx = NULL;
      const AVDictionaryEntry *tag = NULL;

      int ret;
      const char *inFileName = this->filePath.c_str();
      if ((ret = avformat_open_input(&fmt_ctx, inFileName, NULL, NULL)))
        break;

      if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
      {
        LOG(ERROR) << "Cannot find stream information";  
        break;
      }

      while ((tag = av_dict_iterate(fmt_ctx->metadata, tag)))
      {
        LOG(INFO) << tag->key << "=" << tag->value;  // INFO 级别的日志
        callback(tag->key,tag->value);
      }

      av_dump_format(fmt_ctx, 0, inFileName, 0);
      do
      {
        AVCodecContext *codecCtx = NULL;
        AVPacket *pkt = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

        LOG(INFO) << "stream_index="<<stream_index;   
        if (stream_index != -1)
        {
          AVStream *audioStream = fmt_ctx->streams[stream_index];
          AVCodecParameters *aCodecPara = fmt_ctx->streams[stream_index]->codecpar;
          const AVCodec *codec = avcodec_find_decoder(aCodecPara->codec_id);
          if (!codec)
          {
            LOG(ERROR) << "Cannot find any codec for audio.";   
            break;
          }
          codecCtx = avcodec_alloc_context3(codec);
          if (avcodec_parameters_to_context(codecCtx, aCodecPara) < 0)
          {
            LOG(ERROR) << "Cannot alloc codec context.";   
            break;
          }
          codecCtx->pkt_timebase = fmt_ctx->streams[stream_index]->time_base;

          if (avcodec_open2(codecCtx, codec, NULL) < 0)
          {
            LOG(ERROR) << "Cannot open audio codec.";   
            break;
          }
        }
      } while (false);

      avformat_close_input(&fmt_ctx);
    } while (false);
    LOG(INFO) << "parse success.";   
}


std::optional<std::string> AudioMetadata::getTitle() const
{
    return std::optional<std::string>();
}

std::optional<std::string> AudioMetadata::getArtist() const
{
    return std::optional<std::string>();
}

std::optional<double> AudioMetadata::getDuration() const
{
    return std::optional<double>();
}

std::optional<int> AudioMetadata::getBitrate() const
{
    return std::optional<int>();
}

std::optional<int> AudioMetadata::getSampleRate() const
{
    return std::optional<int>();
}