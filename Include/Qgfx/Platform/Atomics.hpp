#pragma once

#if QGFX_PLATFORM_WIN32
#include "Win32/Win32Atomics.hpp"
#else
// Use c++11 standard atomics
#include "Basic/BasicAtomics.hpp"
#endif 


namespace Qgfx
{
#if QGFX_PLATFORM_WIN32
	using Atomics = WindowsAtomics;
#else
	using Atomics = BasicAtomics;
#endif
}