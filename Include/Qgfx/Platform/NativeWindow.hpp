#pragma once

#pragma once

#if QGFX_PLATFORM_WIN32
#include "Win32/Win32NativeWindow.hpp"
#else
#error Unknown platform. Please define one of the following macros as 1: QGFX_PLATFORM_WIN32.
#endif

namespace Qgfx
{
#if QGFX_PLATFORM_WIN32
	typedef Win32NativeWindow NativeWindow;
#else
#error Unknown platform. Please define one of the following macros as 1: QGFX_PLATFORM_WIN32.
#endif
}