#pragma warning(disable : 4819)
#include "flutter_window.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <windows.h>
#include <memory>
#include <optional>
#include <thread>

#include "flutter/generated_plugin_registrant.h"
#include "audio_plugin_handle.h"
// #include "file_plugin_handle.h"
// #include "logger.h"
#include "audio_manager.h"
#include <glog/export.h>
#include <glog/logging.h>

#define WM_MY_CUSTOM_MESSAGE (WM_USER + 1)     // 自定义消息，确保不与现有消息冲突
#define WM_AUDIO_DECODE_MESSAGE (WM_USER + 2)  // 解码下一帧
#define WM_AUDIO_STATA_MESSAGE (WM_USER + 3)   // 音频状态变更
#define WM_AUDIO_SECOEND_MESSAGE (WM_USER + 4) // 音频状态变更

FlutterWindow::FlutterWindow(const flutter::DartProject &project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate()
{
  if (!Win32Window::OnCreate())
  {
    return false;
  }

  semaphore = CreateSemaphore(
      NULL,
      INITIAL_SEMAPHORE_COUNT,
      MAX_SEMAPHORE_COUNT,
      NULL);

  if (semaphore == NULL)
  {
    std::cerr << "Failed to create semaphore!" << std::endl;
    return false;
  }

  RECT frame = GetClientArea();

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view())
  {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());

  MyAudioPluginHandler handler;
  // FilePluginHandler file_handler;
  // 注册flutter函数
  flutter::MethodChannel<> channel(
      flutter_controller_->engine()->messenger(), "flutter_windows_cpp_plugin",
      &flutter::StandardMethodCodec::GetInstance());

  channel.SetMethodCallHandler(
      [&](const flutter::MethodCall<flutter::EncodableValue> &method_call,
          std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
      {
        const flutter::EncodableValue *param = method_call.arguments();

        if (method_call.method_name() == "play")
        {
          handler.HandlePlay(param, std::move(result));
        }
        else if (method_call.method_name() == "pause")
        {
          handler.HandlePause(param, std::move(result));
        }
        else if (method_call.method_name() == "stop")
        {
          const flutter::EncodableMap arguments = std::get<flutter::EncodableMap>(*param);
          handler.HandleStop(arguments, std::move(result));
          // handler.HandlePlay();
        }
        else if (method_call.method_name() == "resume")
        {
          handler.HandleResume(param, std::move(result));
        }
        else if (method_call.method_name() == "seek")
        {
          handler.HandleSeek(param, std::move(result));
        }
        else
        {
          result->NotImplemented();
        }
      });

  // 注册flutter函数
  // flutter::MethodChannel<> channel2(
  //     flutter_controller_->engine()->messenger(), "flutter_windows_plugin/file_picker",
  //     &flutter::StandardMethodCodec::GetInstance());

  // channel2.SetMethodCallHandler(
  //     [&](const flutter::MethodCall<flutter::EncodableValue> &method_call,
  //         std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
  //     {
  //       const flutter::EncodableMap arguments = std::get<flutter::EncodableMap>(*method_call.arguments());
  //       if (method_call.method_name() == "openFilePicker")
  //       {
  //         file_handler.HandleOpen(arguments, std::move(result));
  //       }
  //       else
  //       {
  //         result->NotImplemented();
  //       }
  //     });

  // auto timer_callback = [](FlutterWindow *flutterW)
  // {
  //   std::cout << " seconds_idx: " << flutterW->seconds_idx << std::endl;
  // };
  flutter::EventChannel<> charging_channel(
      flutter_controller_->engine()->messenger(), "samples.flutter.io/charging",
      &flutter::StandardMethodCodec::GetInstance());
  charging_channel.SetStreamHandler(
      std::make_unique<flutter::StreamHandlerFunctions<>>(
          [&](auto arguments, auto events)
          {
            // std::thread timer_thread([&]() {
            //      while (true) {
            //         std::this_thread::sleep_for(std::chrono::seconds(1));

            //         WaitForSingleObject(semaphore, INFINITE);
            //         SendMessage(GetHandle(), WM_MY_CUSTOM_MESSAGE, 0, 0);
            //         // LDEBUG("This is a debug message");
            //      }
            //   });
            // timer_thread.detach();
            AudioManager::getInstance().initialize([&](AudioEnum msg)
                                                   { SendMessage(GetHandle(), WM_AUDIO_STATA_MESSAGE, msg, 0); },
                                                   [&](double max, double sec)
                                                   {
                                                     SendMessage(GetHandle(), WM_AUDIO_SECOEND_MESSAGE, max, sec);
                                                   });

            this->OnStreamListen(std::move(events), "");
            return nullptr;
          },
          [this](auto arguments)
          {
            this->OnStreamCancel();
            return nullptr;
          }));

  SetChildContent(flutter_controller_->view()->GetNativeWindow());

  flutter_controller_->engine()->SetNextFrameCallback([&]()
                                                      { this->Show(); });

  // Flutter can complete the first frame before the "show window" callback is
  // registered. The following call ensures a frame is pending to ensure the
  // window is shown. It is a no-op if the first frame hasn't completed yet.
  flutter_controller_->ForceRedraw();

  return true;
}

void FlutterWindow::OnDestroy()
{
  if (flutter_controller_)
  {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept
{
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (flutter_controller_)
  {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result)
    {
      return *result;
    }
  }

  switch (message)
  {
  case WM_FONTCHANGE:
    flutter_controller_->engine()->ReloadSystemFonts();
    break;
  case WM_MY_CUSTOM_MESSAGE:
    ReleaseSemaphore(semaphore, 1, NULL);
    // message_idx++;
    // std::cout << " MessageHandler ... " << message_idx<< std::endl;
    // SendStateEvent();
    break;
  case WM_AUDIO_STATA_MESSAGE:
    AudioManager::getInstance().setStatus((AudioEnum)wparam);
    if ((AudioEnum)wparam == AUDIO_STATA_STOPED)
    {
      // 通知上层更新状态
      SendStateEvent("stoped");
    }
    if ((AudioEnum)wparam == AUDIO_STATA_PAUSED)
    {
      // 通知上层更新状态
      SendStateEvent("paused");
    }
    if ((AudioEnum)wparam == AUDIO_STATA_PLAYED)
    {
      // 通知上层更新状态
      SendStateEvent("playing");
    }
    break;

  case WM_AUDIO_SECOEND_MESSAGE:
    SendStateEventProcess((double)wparam, (double)lparam);
    break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}

void FlutterWindow::OnStreamListen(
    std::unique_ptr<flutter::EventSink<>> &&events, const std::string value)
{
  event_sink_ = std::move(events);
  SendStateEvent(value);
}

void FlutterWindow::OnStreamCancel() { event_sink_ = nullptr; }

void FlutterWindow::SendStateEvent(const std::string value)
{
  // event_sink_->Error("UNAVAILABLE", "Charging status unavailable");
  // SYSTEM_POWER_STATUS status;
  // if (GetSystemPowerStatus(&status) == 0 || status.ACLineStatus == 255)
  // {
  //   event_sink_->Error("UNAVAILABLE", "Charging status unavailable");
  // }
  // else
  // {
  //   event_sink_->Success(flutter::EncodableValue(
  //       status.ACLineStatus == 1 ? "charging" : "discharging"));
  // }
  // event_sink_->Success(flutter::EncodableValue(message_idx));
  // event_sink_->Success(flutter::EncodableValue(value));
  

  // std::map<flutter::EncodableValue, flutter::EncodableValue> res = flutter::EncodableMap{};

  // std::string k1 = "stata";
  // double v1 = value;
  // res[k1] = v1;

  // event_sink_->Success(res);
  event_sink_->Success(flutter::EncodableValue(value));
}

void FlutterWindow::SendStateEventProcess(double max, double sec)
{

  std::map<flutter::EncodableValue, flutter::EncodableValue> res = flutter::EncodableMap{};

  std::string k1 = "max";
  double v1 = max;
  std::string k2 = "sec";
  double v2 = sec;

  res[k1] = v1;
  res[k2] = v2;

  event_sink_->Success(res);
}
