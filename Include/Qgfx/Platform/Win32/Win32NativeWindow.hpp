#pragma once

#if !QGFX_PLATFORM_WIN32
#error QGFX_PLATFORM_WIN32 must be defined to include Win32Atomics.hpp
#endif

namespace Qgfx
{
    struct Win32NativeWindow
    {
        void* hWnd = nullptr;

        Win32NativeWindow() noexcept
        {}

        explicit Win32NativeWindow(void* _hWnd) noexcept :
            hWnd{ _hWnd }
        {}
    };
}