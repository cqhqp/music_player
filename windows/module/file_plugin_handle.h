
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

// #include <windows.h>  
// #include <commdlg.h>  
// #include <tchar.h>  

class FilePluginHandler {  
public:  
  void HandleOpen(const flutter::EncodableMap& arguments, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);  
  
  // 添加其他方法处理函数...  

// private:
//   INT_PTR CALLBACK FileDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
};
