// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <commdlg.h>
#include <math.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <dwrite_3.h>
#include <wincodec.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

// c++ 17 ignore warning
#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#define ASIO_STANDALONE 
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

// #define ASIO_ENABLE_HANDLER_TRACKING

#include <asio.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <iostream>
#include <fstream>

// Windows COM Release
template<class Interface>
inline void SafeRelease(
    Interface **ppInterfaceToRelease
)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

using Byte = std::uint8_t;

// spdlog
inline std::shared_ptr<spdlog::logger> globalLog()
{
    static auto plog = spdlog::get("global_log");
    if (plog)
        return plog;

    plog = spdlog::rotating_logger_mt("global_log", "file-transfer.log", 1048576 * 5, 3);
    
    // TODO: 将 std err 重定向到文件日志中
#ifdef ASIO_ENABLE_HANDLER_TRACKING
    /* 下面方法不顶用
    static std::ofstream ofs("test.log", std::ios::binary);
    std::cerr.rdbuf(ofs.rdbuf());
    */
#endif // ASIO_ENABLE_HANDLER_TRACKING
    return plog;
}

// 将 wstring 转换为 string
inline std::string narrow(const std::wstring &s)
{
    static UINT cp = GetACP();
    std::string str;
    int len = WideCharToMultiByte(cp, 0,
        s.c_str(), (int)s.length(),
        NULL, 0, NULL, NULL);
    if (len > 0)
    {
        str.resize(len);
        WideCharToMultiByte(cp, 0,
            s.c_str(), (int)s.length(), &str[0],
            len, NULL, NULL);
    }
    return str;
}

inline char separator()
{
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

#include "state.h"
#include "TransferContext.h"
#include "TransferSender.h"
#include "TransferReceiver.h"
#include "TransferServer.h"


