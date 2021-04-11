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

		virtual uint32_t GetWidth() override { return m_Width; }
		virtual uint32_t GetHeight() override { return m_Height; }

		virtual uint32_t GetBufferCount() override { return m_BufferCount; }

		virtual SurfaceTransform GetSurfaceTransform() override { return m_PreTransform; }

	private:

		void CreateSurface();
		void CreateSwapChain();

		void AcquireNextImage();

	private:

		////////////////////////////////
		// Handles /////////////////////
		////////////////////////////////
		
		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		NativeWindow m_Window;

		////////////////////////////////
		// Settings ////////////////////
		////////////////////////////////

		TextureFormat       m_ColorBufferFormat;
		TextureFormat       m_DepthBufferFormat;
		SurfaceTransform    m_DesiredPreTransform;
		uint32_t            m_DesiredBufferCount;
		SwapChainUsageFlags m_Usage;
		
		/////////////////////////////////
		// Current state ////////////////
		/////////////////////////////////

		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_BufferCount = 0;
		bool     m_bIsMinimized = false;
		bool     m_bVSyncEnabled = true;

		SurfaceTransform m_PreTransform;

		///////////////////////////////////
		// Vulkan Objects /////////////////
		///////////////////////////////////
		
		vk::SurfaceKHR m_VkSurface;
		vk::SwapchainKHR m_VkSwapChain;

		std::vector<vk::Semaphore> m_ImageAcquiredSemaphores;
		std::vector<vk::Fence>     m_ImageAcquiredFences;

		std::vector<vk::Semaphore> m_DrawCompleteSemaphores;

		vk::Format m_VkColorFormat;

#if 0 // For Andriod
		// Surface extent corresponding to identity transform. We have to store this value,
		// because on Android vkGetPhysicalDeviceSurfaceCapabilitiesKHR is not reliable and
		// starts reporting incorrect dimensions after few rotations.
		vk::Extent2D m_SurfaceIdentityExtent = {};

		// Keep track of current surface transform to detect orientation changes.
		vk::SurfaceTransformFlagBitsKHR m_CurrentSurfaceTransform = {};
#endif

	};
}