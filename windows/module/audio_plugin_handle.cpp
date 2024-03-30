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
#include <filesystem>  

namespace fs = std::filesystem;  

std::string GetExecutableDirectory() {  
    char buffer[MAX_PATH];  
    if (GetModuleFileNameA(NULL, buffer, sizeof(buffer)) != 0) {  
        fs::path fullPath(buffer);  
        return fullPath.parent_path().string();  
    }  
    return ""; // ���ʧ�ܣ����ؿ��ַ���  
}  

MyAudioPluginHandler::MyAudioPluginHandler()
{
  google::InitGoogleLogging("test");

  // ������־���Ŀ¼
  FLAGS_log_dir = "./logs";

  std::string dir = GetExecutableDirectory();  
  std::string home = dir+"/logs/";

    // ������־����  
  FLAGS_alsologtostderr = true; // ������־��Ϣ������־�ļ�֮���Ƿ�ȥ��׼���
  FLAGS_max_log_size = 10; // ���������־�ļ���С����MBΪ��λ��
  FLAGS_stop_logging_if_full_disk = true; // �����Ƿ��ڴ�������ʱ������־��¼������

  FLAGS_minloglevel = google::GLOG_WARNING;
  // FLAGS_minloglevel = google::GLOG_INFO;
  std::string info_log = home+"info_";
  google::SetLogDestination(google::GLOG_INFO, info_log.c_str());
  std::string w_log = home+"w_";
  google::SetLogDestination(google::GLOG_WARNING, w_log.c_str());
  std::string e_log = home+"e_";
  google::SetLogDestination(google::GLOG_ERROR, e_log.c_str());
  std::string f_log = home+"f_";
  google::SetLogDestination(google::GLOG_FATAL, f_log.c_str());

  LOG(INFO) << "Hello, GOOGLE!";  // INFO �������־
  LOG(WARNING) << "Hello, GOOGLE! warning test";  // �����һ��Warning��־  
  LOG(ERROR) << "Hello, GOOGLE! This should work";
}

MyAudioPluginHandler::~MyAudioPluginHandler()
{
  google::ShutdownGoogleLogging();
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
  const std::string file_path = arguments;

  if (IsFileExist_from_utf8(file_path.c_str()))
  {
    std::cout << " ... file can open ...!" << std::endl;

    std::map<flutter::EncodableValue, flutter::EncodableValue> res = flutter::EncodableMap{};

    AudioManager::getInstance().metadata(file_path, [&res](std::string key, std::string value){
        res[flutter::EncodableValue(key)] = flutter::EncodableValue(value); // ����mapʧ��
    });

    for (auto it : res)
    {
      flutter::EncodableValue v1 = it.first;
      flutter::EncodableValue v2 = it.second;
      std::string f = std::get<std::string>(v1);
      std::string s = std::get<std::string>(v2);
    }

    result->Success(res);
    // result->Success(true);
    AudioManager::getInstance().play(file_path);
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
  AudioManager::getInstance().pause();
  // ʵ����ͣ��Ƶ���߼�
  // ...
  result->Success(true);
}

void MyAudioPluginHandler::HandleStop(const flutter::EncodableMap &arguments, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{

  std::cout << " ... HandleStop ...!" << std::endl;
  
  AudioManager::getInstance().stop();

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

void MyAudioPluginHandler::HandleResume(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{

  std::cout << " ... HandleResume ...!" << std::endl;
  
  AudioManager::getInstance().resume();
  result->Success(true);
}
void MyAudioPluginHandler::HandleSeek(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
  const double value = std::get<double>(*param);
  AudioManager::getInstance().seek(value);
  result->Success(true);
}
// �����������������...