#include "audio_decoder.h"

#include <glog/export.h>
#include <glog/logging.h>

#define av_ts2str2(ts, buf) av_ts_make_string(buf, ts)
#define av_ts2timestr2(ts, tb, buf) av_ts_make_time_string(buf, ts, tb)

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    char ts_str[AV_TS_MAX_STRING_SIZE] = {0};

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str2(pkt->pts, ts_str), av_ts2timestr2(pkt->pts, time_base, ts_str),
           av_ts2str2(pkt->dts, ts_str), av_ts2timestr2(pkt->dts, time_base, ts_str),
           av_ts2str2(pkt->duration, ts_str), av_ts2timestr2(pkt->duration, time_base, ts_str),
           pkt->stream_index);
}

bool Mp3Decoder::initialize(const std::string &filePath, const std::function<void(double, double)> &callback)
{
    inFileName = filePath.c_str();
    this->bk = callback;

    fmtCtx = avformat_alloc_context();
    codecCtx = NULL;
    pkt = av_packet_alloc();
    frame = av_frame_alloc();
    aStreamIndex = -1;

    do
    {
        if (avformat_open_input(&fmtCtx, inFileName, NULL, NULL) < 0)
        {
            LOG(ERROR) << "Cannot open input file.";
            return false;
        }
        if (avformat_find_stream_info(fmtCtx, NULL) < 0)
        {
            LOG(ERROR) << "Cannot find any stream in file.";
            return false;
        }
        for (size_t i = 0; i < fmtCtx->nb_streams; i++)
        {
            if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                aStreamIndex = (int)i;
                break;
            }
        }
        if (aStreamIndex == -1)
        {
            LOG(ERROR) << "Cannot find audio stream.";
            return false;
        }

        AVCodecParameters *aCodecPara = fmtCtx->streams[aStreamIndex]->codecpar;
        const AVCodec *codec = avcodec_find_decoder(aCodecPara->codec_id);
        if (!codec)
        {
            LOG(ERROR) << "Cannot find any codec for audio.";
            return false;
        }

        codecCtx = avcodec_alloc_context3(codec);
        if (avcodec_parameters_to_context(codecCtx, aCodecPara) < 0)
        {
            LOG(ERROR) << "Cannot alloc codec context.";
            return false;
        }
        codecCtx->pkt_timebase = fmtCtx->streams[aStreamIndex]->time_base;

        if (avcodec_open2(codecCtx, codec, NULL) < 0)
        {
            LOG(ERROR) << "Cannot open audio codec.";
            return false;
        }
    } while (0);

    return true;
}

void Mp3Decoder::release()
{
    if (frame)
        av_frame_free(&frame);
    if (pkt)
        av_packet_free(&pkt);
    if (codecCtx)
    {
        avcodec_close(codecCtx);
        avcodec_free_context(&codecCtx);
    }
    if (fmtCtx)
        avformat_free_context(fmtCtx);
}

bool Mp3Decoder::decode()
{
    int i, ch, ret;

    bool dec_ret = false;
    do
    {

        ret = av_read_frame(fmtCtx, pkt);
        if (ret < 0)
        {
            // 检查是否是流的结尾
            if (ret == AVERROR_EOF)
            {
                LOG(ERROR) << "AVERROR_EOF.";
                break; // 没有更多的数据包了
            }
            else
            {
                // 处理其他错误
                if (ret < 0)
                {
                    LOG(ERROR) << "Error: av_readframe failed.";
                }
                av_packet_unref(pkt);
                break;
            }
        }

        // 在这里处理数据包
        // ...
        {
            // 这是一个音频数据包
            AVCodecParameters *codecpar = fmtCtx->streams[pkt->stream_index]->codecpar;
            int channels = codecpar->channels;
            // LOG(INFO) << "channels:" << channels;

            // char timeStr[AV_TS_MAX_STRING_SIZE] = {0};
            // av_ts_make_time_string(timeStr, (int64_t)pkt->pts, &fmtCtx->streams[pkt->stream_index]->time_base);
            // LOG(INFO) << "pkt->pts time1::" << timeStr;
            // log_packet(fmtCtx, pkt, "in");
            if (pkt->pts == AV_NOPTS_VALUE)
            {
                LOG(INFO) << "AV_NOPTS_VALUE";
            }
            else
            {
                double les = fmtCtx->streams[pkt->stream_index]->duration * av_q2d(fmtCtx->streams[pkt->stream_index]->time_base);
                double sec = pkt->pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                // LOG(INFO) << "les:" << les;
                // LOG(INFO) << "sec:" << sec;
                this->bk(les, sec);
            }
            dec_ret = true;
        }

        // 释放数据包
        av_packet_unref(pkt);
    } while (0);

    // if(pkt->stream_index== aStreamIndex){
    //     ret = avcodec_send_packet(codecCtx, pkt);
    //     if(ret <0){
    //         LOG(ERROR) << "Error: avcodec_send_packet failed.";
    //     }
    //     LOG(INFO) << "pkt->dts:" << pkt->dts;
    //     LOG(INFO) << "pkt->pts:" << pkt->pts;
    //     {
    //         char timeStr[AV_TS_MAX_STRING_SIZE] = {0};
    //         av_ts_make_time_string(timeStr, (int64_t)pkt->pts, &codecCtx->time_base);
    //         LOG(INFO) << "pkt->pts time1::" << timeStr;
    //     }
    //     /* read all the output frames (in general there may be any number of them */
    //     while (ret >= 0) {
    //         ret = avcodec_receive_frame(codecCtx, frame);
    //         if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
    //             LOG(INFO) << "End one frame.";
    //             break;
    //         }
    //         else if (ret < 0) {
    //             LOG(ERROR) << "Error: avcodec_receiveframe during decoding.";
    //             exit(1);
    //         }
    //         int linesize = 0;
    //         // int sample_size = av_get_bytes_per_sample(codecCtx->sample_fmt);
    //         int t_data_size = av_samples_get_buffer_size(
    //                 &linesize, 1, frame->nb_samples, (AVSampleFormat)frame->format, 0);
    //         if (t_data_size < 0) {
    //             /* This should not occur, checking just for paranoia */
    //             LOG(ERROR) << "Failed to calculate data size.";
    //             return;
    //         }else{
    //             LOG(INFO) << "data_size:" << t_data_size;
    //             LOG(INFO) << "frame->pts:" << frame->pts;
    //             double time1 = frame->pts * av_q2d(frame->time_base);
    //             double time2 = frame->pts * av_q2d(codecCtx->time_base);
    //             char timeStr[AV_TS_MAX_STRING_SIZE] = {0};
    //             av_ts_make_time_string(timeStr, (int64_t)frame->pts, &codecCtx->time_base);
    //             char *time3 = timeStr;
    //             LOG(INFO) << "time1::" << time1 <<"  time2:" << time3;
    //         }
    //         // for (i = 0; i < frame->nb_samples; i++)
    //         //     for (ch = 0; ch < codecCtx->channels; ch++)
    //         //         fwrite(frame->data[ch] + data_size*i, 1, data_size, outfile);
    //     }
    // }
    return dec_ret;
}

bool Mp3Decoder::isInitialized() const
{
    return false;
}

Mp3Decoder::~Mp3Decoder()
{
}
