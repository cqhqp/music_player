#ifndef AUDIO_PLUGIN_HANDLE_H
#define AUDIO_PLUGIN_HANDLE_H

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include "audio_manager.h"

class MyAudioPluginHandler {  
public:  
  void HandlePlay(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);  
  void HandlePause(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);  
  void HandleStop(const flutter::EncodableMap& args, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void HandleResume(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void HandleSeek(const flutter::EncodableValue *param, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  MyAudioPluginHandler();
  virtual ~MyAudioPluginHandler();
  // 添加其他方法处理函数...  
private:
};
#endif