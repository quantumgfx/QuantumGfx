#pragma once

#include <quantumvk/quantumvk.hpp>

#ifndef VK_KHR_SURFACE_EXTENSION_NAME
#error Vulkan Headers do not include VK_KHR_surface extension. This is required for Qgfx!
#endif

#ifdef QGFX_PLATFORM_WIN32
#ifndef VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#error Vulkan Headers do not include VK_KHR_win32_surface. This is required for Qgfx to create SwapChains on Win32.
#endif
#endif

#ifndef QGFX_VULKAN_SUPPORTED
#error QGFX_VULKAN_SUPPORTED must be defined to include Vulkan related headers.
#endif
