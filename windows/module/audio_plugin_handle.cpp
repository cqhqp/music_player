#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include "audio_plugin_handle.h"
#include <memory>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <windows.h>
#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
};
#include <glog/export.h>
#include <glog/logging.h>
// bool IsFileExist(const char *filePath)
// {
//   HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//   if (hFile == INVALID_HANDLE_VALUE)
//   {
//     DWORD dw = GetLastError();
//     if (dw == ERROR_FILE_NOT_FOUND || dw == ERROR_PATH_NOT_FOUND)
//     {
//       return false; //
//     }
//     // ...
//   }
//   CloseHandle(hFile); //
//   return true;        //
// }

// std::wstring stringToWstring(const std::string &str)
// {
//   int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
//   std::wstring wstrTo(size_needed, 0);
//   MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
//   return wstrTo;
// }

// size_t getFileSize1(const char *fileName)
// {
//   if (fileName == NULL)
//   {
//     return 0;
//   }
//   struct stat statbuf;
//   statbuf.st_size = 0;
//   stat(fileName, &statbuf);
//   size_t filesize = statbuf.st_size;
//   return filesize;
// }

MyAudioPluginHandler::MyAudioPluginHandler()
{
  google::InitGoogleLogging("test");

  // ������־���Ŀ¼
  FLAGS_log_dir = "./logs";

  FLAGS_logtostdout = true;
  FLAGS_colorlogtostdout = true;
  FLAGS_logtostderr = true;
  FLAGS_alsologtostderr = true;

  LOG(INFO) << "Hello, GOOGLE!";  // INFO �������־
  LOG(WARNING) << "warning test";  // �����һ��Warning��־  
  LOG(ERROR) << "This should work";
  // LOG_IF(ERROR, x > y) << "This should be also OK";

  google::ShutdownGoogleLogging();

  // google::InitGoogleLogging();
  // std::string str = "class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl MyAudioPluginHandler::{ctor}::<lambda_21c91b596babde87656e45c85728742d>::operator ()(void) const";
  // std::string str2 =str.substr(88, str.length()-88);
  // std::cout << "str2 :: " << str2 << std::endl;
  // Logger::LDEBUG("This is a debug message");
  // LINFO("This is an info message");
  // LWARNING("This is a warning message"); // �����
  // LERROR("This is an error message");    // �����
}

MyAudioPluginHandler::~MyAudioPluginHandler()
{
}

bool IsFileExist_from_utf8(const char *utf8FilePath)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8FilePath, -1, nullptr, 0);
  if (len == 0)
  {
    return false;
  }

  std::wstring wideFilePath(len, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8FilePath, -1, &wideFilePath[0], len);

  HANDLE hFile = CreateFileW(wideFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    DWORD dw = GetLastError();
    if (dw == ERROR_FILE_NOT_FOUND || dw == ERROR_PATH_NOT_FOUND)
    {
      return false;
    }
  }
  CloseHandle(hFile);
  return true;
}

void MyAudioPluginHandler::HandlePlay(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
  const std::string arguments = std::get<std::string>(*param);
  // std::cout << " arguments: " << arguments << std::endl;
  // size_t siize = getFileSize1(arguments.c_str());
  // std::cout << " siize: " << siize << std::endl;

  if (IsFileExist_from_utf8(arguments.c_str()))
  {
    std::cout << " ... file can open ...!" << std::endl;

    std::map<flutter::EncodableValue, flutter::EncodableValue> res = flutter::EncodableMap{};

    do
    {
      AVFormatContext *fmt_ctx = NULL;
      const AVDictionaryEntry *tag = NULL;

      int ret;
      const char *inFileName = arguments.c_str();
      if ((ret = avformat_open_input(&fmt_ctx, inFileName, NULL, NULL)))
        break;

      if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
      {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        break;
      }

      while ((tag = av_dict_iterate(fmt_ctx->metadata, tag)))
      {
        printf("%s=%s\n", tag->key, tag->value);
        std::string key = tag->key;
        std::string value = tag->value;

        res[flutter::EncodableValue(key)] = flutter::EncodableValue(value); // ����mapʧ��
        // res[flutter::EncodableValue(tag->key)] = flutter::EncodableValue(tag->value); // ����mapʧ��
      }

      av_dump_format(fmt_ctx, 0, inFileName, 0);
      do
      {
        AVCodecContext *codecCtx = NULL;
        AVPacket *pkt = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        int stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

        printf("stream_index=%d\n", stream_index);
        if (stream_index != -1)
        {
          AVStream *audioStream = fmt_ctx->streams[stream_index];
          AVCodecParameters *aCodecPara = fmt_ctx->streams[stream_index]->codecpar;
          const AVCodec *codec = avcodec_find_decoder(aCodecPara->codec_id);
          if (!codec)
          {
            printf("Cannot find any codec for audio.\n");
            break;
          }
          codecCtx = avcodec_alloc_context3(codec);
          if (avcodec_parameters_to_context(codecCtx, aCodecPara) < 0)
          {
            printf("Cannot alloc codec context.\n");
            break;
          }
          codecCtx->pkt_timebase = fmt_ctx->streams[stream_index]->time_base;

          if (avcodec_open2(codecCtx, codec, NULL) < 0)
          {
            printf("Cannot open audio codec.\n");
            break;
          }
        }
      } while (false);

      avformat_close_input(&fmt_ctx);
    } while (false);
    std::cout << " ... end ...!" << std::endl;
    for (auto it : res)
    {
      flutter::EncodableValue v1 = it.first;
      flutter::EncodableValue v2 = it.second;
      std::string f = std::get<std::string>(v1);
      std::string s = std::get<std::string>(v2);
      std::cout << f << " " << s << std::endl;
    }

    result->Success(res);
    // result->Success(true);
    return;
  }
  else
  {
    std::cout << " ... no file ...!" << std::endl;
    result->Success(false);
    return;
  }

  // ʵ�ֲ�����Ƶ���߼�
  // ...
}

void MyAudioPluginHandler::HandlePause(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
  std::cout << " ... HandlePause ...!" << std::endl;
  // ʵ����ͣ��Ƶ���߼�
  // ...
  result->Success(true);
}

void MyAudioPluginHandler::HandleStop(const flutter::EncodableMap &arguments, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{

  std::cout << " ... HandleStop ...!" << std::endl;

  flutter::EncodableValue v1 = arguments.find(flutter::EncodableValue("key1"))->second;
  if (std::holds_alternative<std::string>(v1))
  {
    std::string some_string = std::get<std::string>(v1);
    std::cout << some_string << std::endl;
  }
  else
  {
    std::cout << "key1 type error" << std::endl;
  }

  flutter::EncodableValue ev2 = arguments.find(flutter::EncodableValue("key2"))->second;
  if (std::holds_alternative<int>(ev2))
  {
    int value2 = std::get<int>(ev2);
    std::cout << value2 << std::endl;
  }
  else
  {
    std::cout << "key2 type error" << std::endl;
  }

  flutter::EncodableValue ev3 = arguments.find(flutter::EncodableValue("key3"))->second;
  if (std::holds_alternative<bool>(ev3))
  {
    int value3 = std::get<bool>(ev3);
    std::cout << value3 << std::endl;
  }
  else
  {
    std::cout << "key3 type error" << std::endl;
  }
  result->Success(true);
  // result->Error("Error", "Music already stoped!");
}
// �����������������...