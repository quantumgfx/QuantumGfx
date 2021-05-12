#pragma once

#ifndef QGFX_VULKAN_SUPPORTED
#error QGFX_VULKAN_SUPPORTED must be defined to include Vulkan related headers.
#endif

#ifdef QGFX_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#error Unsupported platform for Vulkan
#endif

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.hpp>

#ifndef VK_VERSION_1_2
#error Vulkan Headers do not support vulkan 1.2.
#endif

#ifndef VK_KHR_SURFACE_EXTENSION_NAME
#error Vulkan Headers do not include VK_KHR_surface extension. This is required for Qgfx.
#endif

#ifndef VK_KHR_SWAPCHAIN_EXTENSION_NAME
#error Vulkan Headers do not include VK_KHR_swapchain extension. This is required for Qgfx.
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#ifndef VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#error Vulkan Headers do not include VK_KHR_win32_surface. This is required for Qgfx to create Surfaces on Win32.
#endif
#endif

#ifndef VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#error Vulkan Headers do not include VK_EXT_debug_utils. This is required for debugging in Qgfx
#endif

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include <vk_mem_alloc.h>

#include "../IBase.hpp"
#include "../IResource.hpp"
#include "../IRenderer.hpp"

namespace Qgfx
{
	class VulkanConversion
	{
	public:

		static vk::BufferUsageFlags GetVkBufferUsage(ResourceUsageFlags Usage);

		static vk::ImageUsageFlags GetVkImageUsage(ResourceUsageFlags Usage);

		static ResourceUsageFlags GetResourceUsage(vk::BufferUsageFlags Usage);

		static ResourceUsageFlags GetResourceUsage(vk::ImageUsageFlags Usage);

		static vk::ClearValue GetVkClearValue(ClearValue Clear, TextureFormat Fmt);

		static vk::Format GetColorVkFormat(TextureFormat ColorFmt);

		static vk::SurfaceTransformFlagBitsKHR GetVkSurfaceTransformKHR(SurfaceTransform Transform);

		static SurfaceTransform GetSurfaceTransform(vk::SurfaceTransformFlagBitsKHR Transform);
	};
}