#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>

#include "DebugOutput.hpp"
#include "FormatString.hpp"

namespace Qgfx
{
    template <bool>
    void ThrowIf(std::string&&)
    {
    }

    template <>
    inline void ThrowIf<true>(std::string&& msg)
    {
        throw std::runtime_error(std::move(msg));
    }


    template <bool bThrowException, typename... ArgsType>
    void LogError(bool IsFatal, const char* Function, const char* FullFilePath, int Line, const ArgsType&... Args)
    {
        std::string FileName(FullFilePath);

        auto LastSlashPos = FileName.find_last_of("/\\");
        if (LastSlashPos != std::string::npos)
            FileName.erase(0, LastSlashPos + 1);
        auto Msg = FormatString(Args...);

        WriteDebugMessage(IsFatal ? DebugMessageSeverity::FatalError : DebugMessageSeverity::Error, Msg.c_str(), Function, FullName.c_str(), Line);

        ThrowIf<bThrowException>(std::move(Msg));
    }
}

#define QGFX_LOG_ERROR(...)                                                                                 \
    do                                                                                                 \
    {                                                                                                  \
        ::Qgfx::LogError<false>(false, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)


#define QGFX_LOG_FATAL_ERROR(...)                                                                          \
    do                                                                                                \
    {                                                                                                 \
        ::Qgfx::LogError<false>(true, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define QGFX_LOG_ERROR_ONCE(...)             \
    do                                  \
    {                                   \
        static bool IsFirstTime = true; \
        if (IsFirstTime)                \
        {                               \
            LOG_ERROR(##__VA_ARGS__);   \
            IsFirstTime = false;        \
        }                               \
    } while (false)

#define QGFX_LOG_ERROR_AND_THROW(...)                                                                      \
    do                                                                                                \
    {                                                                                                 \
        ::Qgfx::LogError<true>(false, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define QGFX_LOG_FATAL_ERROR_AND_THROW(...)                                                               \
    do                                                                                               \
    {                                                                                                \
        ::Qgfx::LogError<true>(true, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define QGFX_LOG_DEBUG_MESSAGE(Severity, ...)                                                                                            \
    do                                                                                                                              \
    {                                                                                                                               \
        auto _msg = ::Qgfx::FormatString(__VA_ARGS__); \
        ::Qgfx::WriteDebugMessage(Severity, _msg.c_str(), nullptr, nullptr, 0) \
    } while (false)

#define QGFX_LOG_FATAL_ERROR_MESSAGE(...) QGFX_LOG_DEBUG_MESSAGE(::Qgfx::DebugMessageSeverity::FatalError, ##__VA_ARGS__)
#define QGFX_LOG_ERROR_MESSAGE(...)       QGFX_LOG_DEBUG_MESSAGE(::Qgfx::DebugMessageSeverity::Error, ##__VA_ARGS__)
#define QGFX_LOG_WARNING_MESSAGE(...)     QGFX_LOG_DEBUG_MESSAGE(::Qgfx::DebugMessageSeverity::Warning, ##__VA_ARGS__)
#define QGFX_LOG_INFO_MESSAGE(...)        QGFX_LOG_DEBUG_MESSAGE(::Qgfx::DebugMessageSeverity::Info, ##__VA_ARGS__)

#define QGFX_LOG_DEBUG_MESSAGE_ONCE(Severity, ...)           \
    do                                                  \
    {                                                   \
        static bool IsFirstTime = true;                 \
        if (IsFirstTime)                                \
        {                                               \
            QGFX_LOG_DEBUG_MESSAGE(Severity, ##__VA_ARGS__); \
            IsFirstTime = false;                        \
        }                                               \
    } while (false)

#define QGFX_LOG_FATAL_ERROR_MESSAGE_ONCE(...) QGFX_LOG_DEBUG_MESSAGE_ONCE(::Qgfx::DebugMessageSeverity::FatalError, ##__VA_ARGS__)
#define QGFX_LOG_ERROR_MESSAGE_ONCE(...)       QGFX_LOG_DEBUG_MESSAGE_ONCE(::Qgfx::DebugMessageSeverity::Error, ##__VA_ARGS__)
#define QGFX_LOG_WARNING_MESSAGE_ONCE(...)     QGFX_LOG_DEBUG_MESSAGE_ONCE(::Qgfx::DebugMessageSeverity::Warning, ##__VA_ARGS__)
#define QGFX_LOG_INFO_MESSAGE_ONCE(...)        QGFX_LOG_DEBUG_MESSAGE_ONCE(::Qgfx::DebugMessageSeverity::Info, ##__VA_ARGS__)

#ifdef QGFX_DEBUG

#define QGFX_ASSERTION_FAILED(Message, ...)                                       \
        do                                                                       \
        {                                                                        \
            auto msg = ::Qgfx::FormatString(Message, ##__VA_ARGS__);           \
            ::Qgfx::DebugAssertFailed(msg.c_str(), __FUNCTION__, __FILE__, __LINE__); \
        } while (false)

#define QGFX_VERIFY(Expr, Message, ...)                    \
        do                                                \
        {                                                 \
            if (!(Expr))                                  \
            {                                             \
                QGFX_ASSERTION_FAILED(Message, ##__VA_ARGS__); \
            }                                             \
        } while (false)

#define QGFX_VERIFY_EXPR(Expr) QGFX_VERIFY(Expr, "Debug expression failed:\n", #    Expr)

#define QGFX_UNEXPECTED QGFX_ASSERTION_FAILED
#define QGFX_UNSUPPORTED QGFX_ASSERTION_FAILED

#else

//#define QGFX_LOG_ERROR(...)do{}while(false)
//#define QGFX_LOG_FATAL_ERROR(...)do{}while(false)
//#define QGFX_LOG_ERROR_ONCE(...)do{}while(false)
//#define QGFX_LOG_DEBUG_MESSAGE(...)do{}while(false)
//#define QGFX_LOG_FATAL_ERROR_MESSAGE(...)do{}while(false)
//#define QGFX_LOG_ERROR_MESSAGE(...)do{}while(false)
//#define QGFX_LOG_WARNING_MESSAGE(...)do{}while(false)
//#define QGFX_LOG_INFO_MESSAGE(...)do{}while(false)
//#define QGFX_LOG_DEBUG_MESSAGE_ONCE(...)do{}while(false)
//#define QGFX_LOG_FATAL_ERROR_MESSAGE_ONCE(...)do{}while(false)
//#define QGFX_LOG_ERROR_MESSAGE_ONCE(...)do{}while(false)
//#define QGFX_LOG_WARNING_MESSAGE_ONCE(...)do{}while(false)
//#define QGFX_LOG_INFO_MESSAGE_ONCE(...)do{}while(false)
//#define QGFX_LOG_ERROR(...)do{}while(false)

#define QGFX_ASSERTION_FAILED(...)do{}while(false)
#define QGFX_UNEXPECTED(...)do{}while(false)
#define QGFX_UNSUPPORTED(...)do{}while(false)
#define QGFX_VERIFY(...)do{}while(false)
#define QGFX_VERIFY_EXPR(...)do{}while(false)

#endif