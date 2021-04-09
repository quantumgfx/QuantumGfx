#pragma once

#include "../ISwapChain.hpp"

#include "../../Platform/NativeWindow.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"
#include "RenderContextVk.hpp"

namespace Qgfx
{
	class SwapChainVk final : public ISwapChain
	{
	public:

		SwapChainVk(RefCounter* pRefCounter, const SwapChainCreateInfo& CreateInfo, const NativeWindow& Window, RenderContextVk* pRenderContext, RenderDeviceVk* pRenderDevice);

		~SwapChainVk();

	private:

		void CreateSurface();
		void CreateSwapChain();

	private:

		uint32_t m_DesiredBufferCount;
		SurfaceTransform m_DesiredPreTransform;
		SwapChainUsageFlags m_Usage;

		uint32_t m_Width;
		uint32_t m_Height;

		uint32_t m_BufferCount;

		SurfaceTransform m_PreTransform;

		TextureFormat m_ColorBufferFormat;

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		NativeWindow m_Window;
		vk::SurfaceKHR m_VkSurface;
		vk::SwapchainKHR m_VkSwapChain;

		vk::Format m_VkColorFormat;

#if 1
		// Surface extent corresponding to identity transform. We have to store this value,
		// because on Android vkGetPhysicalDeviceSurfaceCapabilitiesKHR is not reliable and
		// starts reporting incorrect dimensions after few rotations.
		vk::Extent2D m_SurfaceIdentityExtent = {};

		// Keep track of current surface transform to detect orientation changes.
		vk::SurfaceTransformFlagBitsKHR m_CurrentSurfaceTransform = {};
#endif
		bool     m_bIsMinimized = false;
		bool     m_bVSyncEnabled = true;

		std::vector<vk::Semaphore> m_ImageAcquiredSemaphores;
		std::vector<vk::Fence>     m_ImageAcquiredFences;

		std::vector<vk::Semaphore> m_DrawCompleteSemaphores;

	};
}