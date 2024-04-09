#include "audio_decoder.h"

#include <glog/export.h>
#include <glog/logging.h>
#include <string>
#include <algorithm> // for std::transform
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
    // std::ofstream outfile("binary_data.pcm", std::ios::binary);
    outfile = std::make_unique<std::ofstream>("binary_data.pcm", std::ios::binary);

    if (!outfile->is_open())
    { // 检查文件是否成功打开
        std::cerr << "Unable to open file for writing";
        return false;
    }

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

void Mp3Decoder::set_pcm_back(const std::function<void(const uint8_t *, int64_t, PcmFormatInfo)> &callback)
{
    this->_u8data_back = callback;
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

    outfile->close(); // 关闭文件
}

bool Mp3Decoder::decode()
{
    int i, ch, ret;

    bool dec_ret = false;
    bool dowhile = false;
    bool showdebugseek = false;

    bool out_data = true;
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
                // break;
            }
        }

        if (seek_dec != 0 && pkt->stream_index == aStreamIndex)
        {
            double sec = pkt->pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
            LOG(WARNING) << " decode pkt:" << sec;
            if (sec < seek_dec)
            {
                av_packet_unref(pkt);
                LOG(WARNING) << "sec < seek_dec.";
                dowhile = true;
                continue;
            }
            else
            {
                showdebugseek = true;
                dowhile = false;
                seek_dec = 0;
                seek_time = 0;
            }
        }

        out_data = true;
        // 在这里处理数据包
        // ...
        if (pkt->stream_index == aStreamIndex)
        {
            AVCodecParameters *codecpar = fmtCtx->streams[pkt->stream_index]->codecpar;
            int channels = codecpar->channels;
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
                if (seek_time == 0)
                {
                    if (showdebugseek)
                    {
                        LOG(WARNING) << "bk sec:" << sec;
                    }
                    // this->bk(les, sec);
                }

                ret = avcodec_send_packet(codecCtx, pkt);
                if (ret < 0)
                {
                    LOG(ERROR) << "Error: avcodec_send_packet failed.";
                }

                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(codecCtx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0)
                    {
                        LOG(ERROR) << "Error during decoding";
                        dec_ret = false;
                        av_packet_unref(pkt);
                        return dec_ret;
                    }
                    else if (ret > 0)
                    {
                        LOG(ERROR) << "Error avcodec_receive_frame ret:" << ret;
                    }

                    // 检查样本格式
                    enum AVSampleFormat sample_fmt = (enum AVSampleFormat)frame->format;
                    std::string fmt_str;
                    int linesize;
                    std::vector<int> t_data_size(frame->channels);
                    // const std::unique_ptr<const std::vector<float>> vecPtr = std::make_unique<const std::vector<float>>(frame->nb_samples*frame->channels);

                    switch (sample_fmt)
                    {
                    case AV_SAMPLE_FMT_U8:
                        fmt_str = "8-bit unsigned integer";
                        break;
                    case AV_SAMPLE_FMT_S16:
                        fmt_str = "16-bit signed integer";
                        break;
                    case AV_SAMPLE_FMT_S32:
                        fmt_str = "32-bit signed integer";
                        break;
                    case AV_SAMPLE_FMT_FLT:
                        fmt_str = "32-bit float";
                        break;
                    case AV_SAMPLE_FMT_DBL:
                        fmt_str = "64-bit float";
                        break;
                    case AV_SAMPLE_FMT_FLTP:
                        fmt_str = "32-bit float planar";
                        {
                            std::vector<uint8_t> u8_vec_pcm;
                            size_t all_data_size = 0;
                            linesize = frame->linesize[0];
                            all_data_size = av_samples_get_buffer_size(
                                &linesize, frame->channels, frame->nb_samples, (AVSampleFormat)frame->format, 0);

                            int sample_rate = codecCtx->sample_rate;

                            if (debug_src_out_idx < 5)
                            {
                                LOG(INFO) << " ";
                                LOG(INFO) << " sample_rate:" << sample_rate;
                                LOG(INFO) << " channels:" << frame->channels;
                                LOG(INFO) << " nb_samples:" << frame->nb_samples;
                                LOG(INFO) << " all_data_size:" << all_data_size;
                                LOG(INFO) << " linesize[0]:" << frame->linesize[0];
                                LOG(INFO) << " linesize[1]:" << frame->linesize[1];
                                LOG(INFO) << " 32-bit float planar";
                                LOG(INFO) << " ";
                                debug_src_out_idx++;
                            }
                            // // debug 解码的pcm保存到文件
                            // outfile->write(reinterpret_cast<char *>(frame->data[1]), frame->linesize[0]);

                            // 重采样和通道无关  扬声器不支持平面格式
                            // speakerInfo.channels != frame->channels ||

                            if (speakerInfo.sample_rate != sample_rate ||
                                speakerInfo.bitsSample != 32 ||
                                speakerInfo.is_float != true)
                            {
                                // LOG(INFO) << "需要重采样.";
                                if (swr_ctx == nullptr)
                                    init_swr();

                                int src_rate = codecCtx->sample_rate;    // 源采样率
                                int src_nb_samples = frame->nb_samples;  // 源 样本数
                                int dst_rate = speakerInfo.sample_rate;  // 目标采样率
                                int dst_channels = speakerInfo.channels; // 目标通道数
                                // int dst_nb_samples = speakerInfo.channels;  // 目标 样本数

                                { // 重采样的采样数是变化的，所以要多次计算重采样数的最大值
                                    // 第一次初始化
                                    
                                    if (!dst_data)
                                    {
                                        // 根据src_nb_samples*dst_rate/src_rate公式初步估算重采样后音频的nb_samples大小
                                        
                                        LOG(INFO) << " xxx";
                                        max_dst_nb_samples = dst_nb_samples =
                                            av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
                                        ret = av_samples_alloc_array_and_samples(&dst_data , &dst_linesize, dst_channels,
                                                                                 dst_nb_samples, dst_sample_fmt, 0);

                                        LOG(INFO) << " 将 dst_data 指针和linesize复制到frame的相应字段";
                                        // 将 dst_data 指针和linesize复制到frame的相应字段  
                                        for (int i = 0; i < dst_channels; i++) {  
                                            output_frame->data[i] = dst_data[i];  
                                            output_frame->linesize[i] = dst_linesize;  
                                        } 
                                        dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_channels, dst_nb_samples, dst_sample_fmt, 0);
                                        LOG(INFO) << " 1 ret:" << ret;
                                        LOG(INFO) << " 1 dst_nb_samples:" << dst_nb_samples;
                                        LOG(INFO) << " 1 max_dst_nb_samples:" << max_dst_nb_samples;
                                        LOG(INFO) << " 1 dst_linesize:" << dst_linesize;
                                        LOG(INFO) << " 1 dst_bufsize:" << dst_bufsize;
                                    }

                                    // 第二次判断
                                    int64_t delay = swr_get_delay(swr_ctx, src_rate);
                                    dst_nb_samples = av_rescale_rnd(delay + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);

                                    if (dst_nb_samples > max_dst_nb_samples)
                                    {
                                        if (dst_data)
                                        {
                                            av_freep(&dst_data[0]);
                                        }
                                        ret = av_samples_alloc(dst_data, &dst_linesize, dst_channels,
                                                               dst_nb_samples, dst_sample_fmt, 0);
                                        if (ret < 0)
                                        {
                                            out_data = false;
                                        }
                                        else
                                        {
                                            max_dst_nb_samples = dst_nb_samples;
                                        }
                                        // // 将 dst_data 指针和linesize复制到frame的相应字段  
                                        for (int i = 0; i < dst_channels; i++) {  
                                            output_frame->data[i] = dst_data[i];  
                                            output_frame->linesize[i] = linesize;  
                                        } 
                                        dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_channels, dst_nb_samples, dst_sample_fmt, 0);
                                        LOG(INFO) << " 2 ret:" << ret;
                                        LOG(INFO) << " 2 dst_nb_samples:" << dst_nb_samples;
                                        LOG(INFO) << " 2 max_dst_nb_samples:" << max_dst_nb_samples;
                                        LOG(INFO) << " 2 dst_linesize:" << dst_linesize;
                                        LOG(INFO) << " 2 dst_bufsize:" << dst_bufsize;
                                    }
                                }

                                if (out_pts == 0) // 读取第一次的pts，后面根据计算，动态增加pts
                                {
                                    out_pts = frame->pts;
                                }

                                // // 调试，输出源样本 时长所需要的pts，与前一个进行对比，判断是否计算正确
                                // {
                                //     int in_sample_rate = codecCtx->sample_rate;                                // 输入的采样率，例如 44.1kHz
                                //     int in_num_samples = frame->nb_samples;                                    // 输入的样本数，例如一个音频帧的样本数
                                //     double duration_seconds = (double)in_num_samples / (double)in_sample_rate; // 计算(输入的)时长（秒）
                                //     int64_t nanosecoend = lrint(duration_seconds * 1000 * 1000 * 1000.0);      // 转换为纳秒，并四舍五入到最近的整数
                                //     AVRational time_base = fmtCtx->streams[pkt->stream_index]->time_base;
                                //     int64_t pts = av_rescale_rnd(nanosecoend, time_base.den, time_base.num, AV_ROUND_ZERO); // AV_ROUND_NEAR_INF AV_ROUND_DOWN AV_ROUND_INF
                                //     pts = pts / 1000 / 1000 / 1000.0;                                          // 前面为了精度放大了值，这里要还原pts的值

                                //     double sec_my = pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);           // 这里输出 通过计算得出的 pts对应的时钟（秒）
                                //     double sec_src2 = frame->pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);  // 这里输出pts对应的 时钟（秒）
                                //     // double sec_src = (frame->pts - pre_frame_pts) * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                                //     LOG(INFO) << " pts-sec(my):" << sec_my;
                                //     LOG(INFO) << " pts-sec(src):" << sec_src2;

                                //     pre_frame_pts = frame->pts;
                                //     if (out_sec_one == 0)
                                //     {
                                //         out_sec_one = sec_src2 - sec_my;
                                //     }
                                // }

#if 0
                                // 调试，输出目标样本 时长所需要的pts
                                {
                                    int sample_rate = dst_rate;                                  // 采样率，例如 44.1kHz
                                    int num_samples = dst_nb_samples;                            // 样本数，例如一个音频帧的样本数
                                    double duration_seconds = (double)num_samples / sample_rate; // 计算时长（秒）
                                    duration_seconds = duration_seconds * 1000 * 1000 * 1000;    // 当时长小于毫秒后，下面的pts无法计算，会损失精度
                                    AVRational time_base = fmtCtx->streams[pkt->stream_index]->time_base;
                                    // int64_t pts = av_rescale_rnd(duration_seconds * time_base.den, sample_rate, 1, AV_ROUND_NEAR_INF); // 将时长转换为以 AV_TIME_BASE 为单位的 PTS
                                    int64_t pts = av_rescale_rnd(duration_seconds, time_base.den, time_base.num, AV_ROUND_NEAR_INF);
                                    pts = pts / 1000 / 1000 / 1000;
                                    out_pts = out_pts + pts;
                                    LOG(INFO) << "num_samples:" << num_samples;
                                    LOG(INFO) << "PTS:" << pts;
                                    LOG(INFO) << "out_pts:" << out_pts;
                                    double sec_src = out_pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                                    LOG(INFO) << "sec_src:" << sec_src;
                                    LOG(INFO) << "";
                                }
#endif
                                // double sec = frame->pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                                // LOG(INFO) << "sec:"<<sec;
                                // double new_sec = out_pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                                // LOG(INFO) << "new_sec:"<<new_sec;
                                double sec_out = 0.0;

                                /* convert to destination format */
                                // int ret_real_nb_samples = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)frame->extended_data, src_nb_samples);
                                int ret_real_nb_samples = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)&frame->data, src_nb_samples);
                                if (ret_real_nb_samples < 0)
                                {
                                    fprintf(stderr, "Error while converting\n");
                                    out_data = false;
                                }
                                else
                                { // 重采样后处理
#if 1                               
                                    // LOG(INFO) << " dst_nb_samples:" << dst_nb_samples;
                                    // LOG(INFO) << " ret_real_nb_samples:" << ret_real_nb_samples;
                                    // debug_src_nb_samples += src_nb_samples;
                                    debug_dst_nb_samples += ret_real_nb_samples;
                                    // LOG(INFO) << " debug_src_nb_samples:" << debug_src_nb_samples;
                                    // LOG(INFO) << " debug_dst_nb_samples:" << debug_dst_nb_samples;
                                    // LOG(INFO) << " xxx:" << debug_src_nb_samples-debug_dst_nb_samples;
                                    // LOG(INFO) << "  ";
                                    // int real_dst_linesize = 0;
                                    // int real_dst_bufsize = av_samples_get_buffer_size(&real_dst_linesize, dst_channels, ret_real_nb_samples, dst_sample_fmt, 0);
                                    int real_dst_bufsize = av_samples_get_buffer_size(NULL, dst_channels, ret_real_nb_samples,dst_sample_fmt, 1); // 不对齐，输出正确的大小
                                    // int real_dst_bufsize = ret_real_nb_samples * 4* dst_channels;
                                    // LOG(INFO) << " dst_bufsize:" << dst_bufsize;
                                    // LOG(INFO) << " real_dst_bufsize:" << real_dst_bufsize;
                                    // LOG(INFO) << " swr_convert ret:" << ret;
                                    // LOG(INFO) << " dst_bufsize:" << dst_bufsize;
                                    {
                                        // 调试，输出目标样本 时长所需要的pts
                                        {
                                            out_pts += pre_pts_diff/1000/1000/1000.0; // 累加计算出的pts到out_pts
                                            
                                            // LOG(INFO) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
                                            int sample_rate = dst_rate;                                                    // 采样率，例如 44.1kHz
                                            int num_samples = ret_real_nb_samples;                                         // 样本数，例如一个音频帧的样本数
                                            double duration_seconds = (double)num_samples / sample_rate;                   // 计算时长（秒）
                                            int64_t duration_nanosecoend = lrint(duration_seconds * 1000 * 1000 * 1000.0); // 转换为微秒，并四舍五入到最近的整数
                                            AVRational time_base = fmtCtx->streams[pkt->stream_index]->time_base;
                                            pre_pts_diff = av_rescale_rnd(duration_nanosecoend, time_base.den, time_base.num, AV_ROUND_NEAR_INF);

                                            // int64_t pts_in_nanoseconds = ((double)num_samples / sample_rate) * 1000 * 1000 * 1000;
                                            // out_pts += pts_in_nanoseconds;
                                            // LOG(INFO) << "PTS diff:" << pts_diff;
                                            // LOG(INFO) << "Cumulative pts_in_nanoseconds:" << pts_in_nanoseconds;
                                            double sec_ = out_pts * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
                                            sec_out = sec_;
                                            // LOG(INFO) << "sec_out:" << sec_out;
                                            // LOG(INFO) << "debug_dst_nb_samples:" << debug_dst_nb_samples;
                                            // LOG(INFO) << "";
                                            // LOG(INFO) << "....................................................";
                                        }
                                    }
#endif
                                    PcmFormatInfo info;
                                    info.bitsSample = speakerInfo.bitsSample;
                                    info.channels = dst_channels;
                                    info.is_float = speakerInfo.is_float;
                                    info.nb_samples = ret_real_nb_samples;
                                    info.planar = false;
                                    info.sample_rate = dst_rate;
                                    info.time_sec = sec_out;
                                    info.max_time_sec = les;

                                    // LOG(INFO) << " - debug - pcm_data 000";
                                    if (data_dst_size < real_dst_bufsize)
                                    {
                                        if (data_dst != nullptr)
                                        {
                                            free(data_dst);
                                            data_dst = nullptr;
                                        }
                                        data_dst = (uint8_t *)malloc(sizeof(uint8_t) * real_dst_bufsize);
                                        data_dst_size = real_dst_bufsize;
                                    }
                                    
                                    // uint8_t* data_dst = pcm_data->data();
                                    // uint8_t* data_dst = (uint8_t* )malloc(sizeof(uint8_t) * all_data_size);
                                    if (data_dst != nullptr && output_frame->data[0] != nullptr)
                                    {

                                        // debug 重采样的pcm保存到文件
                                        auto pcm_data = dst_data[0];
                                        outfile->write(reinterpret_cast<char *>(pcm_data), real_dst_bufsize);
                                        // outfile->write(reinterpret_cast<char *>(output_frame->data[1]), output_frame->linesize[0]);

                                        memcpy(data_dst, pcm_data, real_dst_bufsize);
                                        _u8data_back(data_dst, real_dst_bufsize, info);
                                    }
                                    else
                                    {
                                        LOG(ERROR) << "ERROR:Either pcm_data or frame->data[0] is null. ...............";
                                    }
                                }
                            }
                            // printf("Sample rate: %d\n", sample_rate);
                        }
                        break;
                    // ... 其他格式 ...
                    default:
                        fmt_str = "Unknown";
                    }
                    // LOG(INFO) << "Audio is " << fmt_str;
                }
            }
        }
        dec_ret = true;

        // 释放数据包
        av_packet_unref(pkt);
        // LOG(INFO) << "av_packet_unref ..";
    } while (dowhile);
    return dec_ret;
}

bool Mp3Decoder::seek(double value)
{

    int64_t timestamp = (value / double(av_q2d(fmtCtx->streams[aStreamIndex]->time_base)));
    seek_time = timestamp;
    seek_dec = value;
    double dec = timestamp * av_q2d(fmtCtx->streams[aStreamIndex]->time_base);
    LOG(WARNING) << "seek ... timestamp:" << timestamp;
    LOG(WARNING) << "seek ... dec:" << dec;
    LOG(WARNING) << "seek ... value:" << value;

    int ret = av_seek_frame(fmtCtx, aStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
    {
        // 处理错误
    }
    else
    {
        out_pts = 0; // seek 重采样的时间需要重新 算
        out_sec_one = 0;
    }
    return false;
}

bool Mp3Decoder::isInitialized() const
{
    return false;
}

void Mp3Decoder::setSpeakerInfo(PcmFormatInfo info)
{
    speakerInfo = info;
}

Mp3Decoder::Mp3Decoder()
{
}

Mp3Decoder::~Mp3Decoder()
{
    if (data_dst != nullptr)
    {
        free(data_dst);
        data_dst = nullptr;
    }
    release();
}

void Mp3Decoder::init_swr()
{
    // LOG(INFO) << "init_swr 1.";
    int output_sample_rate = speakerInfo.sample_rate;
    int64_t out_ch_layout = speakerInfo.channels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;
    // dst_sample_fmt = speakerInfo.bitsSample == 16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_U8;
    
    LOG(INFO) << " out output_sample_rate: " << output_sample_rate;
    LOG(INFO) << " out channels: "<< speakerInfo.channels;
    switch (speakerInfo.bitsSample)
    {
    case 8:
        dst_sample_fmt = AV_SAMPLE_FMT_U8;
        LOG(INFO) << " out dst_sample_fmt: AV_SAMPLE_FMT_U8";
        break;
    case 16:
        dst_sample_fmt = AV_SAMPLE_FMT_S16;
        LOG(INFO) << " out dst_sample_fmt: AV_SAMPLE_FMT_S16";
        break;
    case 32:
        dst_sample_fmt = AV_SAMPLE_FMT_S32;
        LOG(INFO) << " out dst_sample_fmt: AV_SAMPLE_FMT_S32";
        break;
    default:
        break;
    }
    // LOG(INFO) << "init_swr 2.";
    if (speakerInfo.is_float)
    {
        dst_sample_fmt = AV_SAMPLE_FMT_FLT;
        LOG(INFO) << " out dst_sample_fmt: AV_SAMPLE_FMT_FLT";
    }

    // 设置输出帧的通道布局和样本格式  
    output_frame = av_frame_alloc();  
    output_frame->channels = speakerInfo.channels;
    output_frame->channel_layout = av_get_default_channel_layout(speakerInfo.channels);
    output_frame->format = dst_sample_fmt;
    output_frame->sample_rate = output_sample_rate;

    // LOG(INFO) << "init_swr 3.";
    swr_ctx = swr_alloc_set_opts(NULL, output_frame->channel_layout, (AVSampleFormat)output_frame->format, output_frame->sample_rate,
                                 codecCtx->channel_layout, (AVSampleFormat)codecCtx->sample_fmt, codecCtx->sample_rate, 0, NULL);

    LOG(INFO) << "init_swr 4.";
    swr_init(swr_ctx);
}
