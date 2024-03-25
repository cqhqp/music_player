#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

#define LOG_DEBUG
#define LOG_INFO
#define LOG_WARNING
#define LOG_ERROR
#define LOG_OFF

class Logger
{

public:
    static void debug(const std::string &className, const std::string &msg)
    {
#ifdef LOG_DEBUG
        std::cout << "[DEBUG]:: " << className << " >>" << msg << std::endl;
#endif
    }

    static void info(const std::string &className, const std::string &msg)
    {
#ifdef LOG_INFO
        std::cout << "[INFO] " << msg << std::endl;
#endif
    }

    static void warning(const std::string &className, const std::string &msg)
    {
#ifdef LOG_WARNING
        std::cout << "[WARNING] " << msg << std::endl;
#endif
    }

    static void error(const std::string &className, const std::string &msg)
    {
#ifdef LOG_ERROR
        std::cout << "[LERROR] " << msg << std::endl;
#endif
    }

    static std::string EXTRACT_CLASS_NAME5()
    {
        size_t endPos = 0;
        size_t cdeclPos = 0;
        const char *funstr_ = __FUNCSIG__;
        std::string funcsig = std::string(funstr_, strlen(funstr_));
        std::string cdecl_str = "__cdecl ";
        cdeclPos = funcsig.find(cdecl_str);
        size_t max = funcsig.length();
        size_t _Count = max - cdeclPos;
        std::string funcsig1 = funcsig.substr(cdeclPos + cdecl_str.length(), _Count);
        std::cout << "funcsig 1:: " << funcsig1 << std::endl;
        endPos = funcsig1.find("::");
        std::cout << "endPos :: " << endPos << std::endl;
        std::string funcsig2 = funcsig1.substr(0, endPos);
        std::cout << "funcsig 2:: " << funcsig2 << std::endl;
        return funcsig2;
    }

    static void LDEBUG(std::string message)
    {
        Logger::debug(EXTRACT_CLASS_NAME5(), message);
    }
};

#define EXTRACT_CLASS_NAME()                              \
    []() -> std::string {                                 \
        const char *funcsig = __FUNCSIG__;                \
        const char *start = funcsig;                      \
        const char *end = funcsig;                        \
        while (*end != ' ')                               \
            end++;                                        \
        while (*end == ' ')                               \
            end++;                                        \
        const char *classKeyword = strstr(end, "class "); \
        if (!classKeyword)                                \
            classKeyword = strstr(end, "struct ");        \
        if (!classKeyword)                                \
            return "";                                    \
        start = classKeyword + strlen("class ");          \
        end = strstr(start, "::");                        \
        if (!end)                                         \
            return "";                                    \
        return std::string(start, end - start);           \
    }()

// class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > __cdecl FlutterWindow::OnCreate::
// <lambda_4c4dccfda5c2725166390ddda6687020>::()::<lambda_205432d240d3319543dd82ef681a8fde>::()::
// <lambda_87f330f7fc2f392f3d31490893b3502c>::operator ()(void) const
//

#define EXTRACT_CLASS_NAME4()                                                     \
    []() -> std::string {                                                         \
        size_t endPos = 0;                                                        \
        size_t cdeclPos = 0;                                                      \
        const char *funstr_ = __FUNCSIG__;                                        \
        std::string funcsig = std::string(funstr_, strlen(funstr_));              \
        std::cout << "__FUNCSIG__ :: " << funcsig << std::endl;                   \
        cdeclPos = funcsig.find("__cdecl");                                       \
        std::cout << "cdeclPos :: " << cdeclPos << std::endl;                     \
        std::string funcsig1 = funcsig.substr(endPos, funcsig.length() - endPos); \
        std::cout << "funcsig 1:: " << funcsig1 << std::endl;                     \
        endPos = funcsig1.find("::");                                             \
        std::cout << "endPos :: " << endPos << std::endl;                         \
        std::string funcsig2 = funcsig1.substr(0, endPos);                        \
        std::cout << "funcsig 2:: " << funcsig2 << std::endl;                     \
        return funcsig2;                                                          \
    }()

// #define LDEBUG(message) Logger::debug(EXTRACT_CLASS_NAME4(), message)
#define LINFO(message) Logger::info(EXTRACT_CLASS_NAME(), message)
#define LWARNING(message) Logger::warning(EXTRACT_CLASS_NAME(), message)
#define LERROR(message) Logger::error(EXTRACT_CLASS_NAME(), message)

#endif