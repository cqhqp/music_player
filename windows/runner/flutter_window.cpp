#include "flutter_window.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <memory>
#include <optional>

#include "flutter/generated_plugin_registrant.h"

FlutterWindow::FlutterWindow(const flutter::DartProject& project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  RECT frame = GetClientArea();

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());
  
  // 注册flutter函数
  flutter::MethodChannel<> channel(
      flutter_controller_->engine()->messenger(), "flutter_windows_cpp_plugin",
      &flutter::StandardMethodCodec::GetInstance());
  channel.SetMethodCallHandler(
      [](const flutter::MethodCall<>& call,
         std::unique_ptr<flutter::MethodResult<>> result) {
        if (call.method_name() == "yourMethodName") {
          std::cout << " ... yourMethodName ...!" << std::endl;

          const flutter::EncodableMap arguments = std::get<flutter::EncodableMap>(*call.arguments());
          flutter::EncodableValue v1 = arguments.find(flutter::EncodableValue("key1"))->second;
          if (std::holds_alternative<std::string>(v1)) {
            std::string some_string = std::get<std::string>(v1);
            std::cout << some_string << std::endl;
          }else{
            std::cout << "warning key1 类型不匹配" << std::endl;
          }

          flutter::EncodableValue ev2 = arguments.find(flutter::EncodableValue("key2"))->second;
          if (std::holds_alternative<int>(ev2)) {
            int value2 = std::get<int>(ev2);
            std::cout << value2 << std::endl;
          }else{
            std::cout << "warning key2 类型不匹配" << std::endl;
          }

          flutter::EncodableValue ev3 = arguments.find(flutter::EncodableValue("key3"))->second;
          if (std::holds_alternative<bool>(ev3)) {
            int value3 = std::get<bool>(ev3);
            std::cout << value3 << std::endl;
          }else{
            std::cout << "warning key3 类型不匹配" << std::endl;
          }
          // const void *arguments = call.arguments();
          // int counter = std::get<int32_t>((*call.arguments()).find(flutter::EncodableValue("key2"))->second);
          // auto* v1 = get_if<string>(&(arguments->find(flutter::EncodableValue("key1"))->second));
          // auto* v2 = get_if<int>(&(arguments->find(flutter::EncodableValue("key2"))->second));

          // std::cout << "value1 =" << v1 << std::endl;
          // std::cout << "value2 =" << v2 << std::endl;
          // auto key1_it = arguments->find("key1");
          // if (key1_it != arguments->end())
          // {
          //   auto value1 = std::get<std::string>(key1_it->second);
          //   std::cout << "value1 =" << value1 << std::endl;
          // }
          result->Success(true);
          // result->Error("Error", "Music already stoped!");
        } else {
          result->NotImplemented();
        }
      });
  
  SetChildContent(flutter_controller_->view()->GetNativeWindow());

  flutter_controller_->engine()->SetNextFrameCallback([&]() {
    this->Show();
  });

  // Flutter can complete the first frame before the "show window" callback is
  // registered. The following call ensures a frame is pending to ensure the
  // window is shown. It is a no-op if the first frame hasn't completed yet.
  flutter_controller_->ForceRedraw();

  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_controller_) {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (flutter_controller_) {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_->engine()->ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
