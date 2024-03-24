#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include "file_plugin_handle.h"
#include <memory>

#include <windows.h>
#include <commdlg.h>
#include <tchar.h>

// INT_PTR CALLBACK FilePluginHandler::FileDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
// {
//   return 0;
// }

void FilePluginHandler::HandleOpen(const flutter::EncodableMap &arguments, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
  std::vector<std::string> filePaths;
  OPENFILENAMEA ofn;  
  char szFile[MAX_PATH * 10]; 
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.hInstance = NULL;
  ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.txt\0";
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrTitle = "Open Files";
  ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST; // 允许选择多个文件
  ofn.lpstrDefExt = NULL;

  if (GetOpenFileNameA(&ofn) == TRUE)
  {
    
    char* pch = ofn.lpstrFile;  

    while (*pch)
    {
      if (*pch == '\0')
      {
        
        filePaths.push_back(std::string(ofn.lpstrFile, pch));
       
        pch += 2;
        ofn.lpstrFile = pch;
      }
      else
      {
       
        ++pch;
      }
    }
  }
  std::string response = "Hello from C++";  
  result->Success(flutter::EncodableValue(response));  
}

// 添加其他方法处理函数...