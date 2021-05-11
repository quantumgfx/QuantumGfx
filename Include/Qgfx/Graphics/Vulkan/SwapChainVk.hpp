#pragma once

#include "../ISwapChain.hpp"

#include "../../Platform/NativeWindow.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"
#include "CommandQueueVk.hpp"

namespace Qgfx
{
	class SwapChainVk final : public ISwapChain
	{
	public:

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform) override;

		virtual void AcquireNextTexture() override;

		virtual ITexture* GetCurrentTexture() override;

		virtual ITextureView* GetCurrentTextureView() override;

		virtual void Present() override;

		virtual void Destroy() override;

	private:

		friend RenderDeviceVk;

		void CreateSurface();
		void CreateSwapChain();

		void RecreateSwapChain();

		void ReleaseSwapChainResources(bool bDestroySwapChain);

		void WaitForImageAcquiredFences();

		SwapChainVk(RenderDeviceVk* pRenderDevice, const SwapChainCreateInfo& CreateInfo);

		~SwapChainVk();

	private:

		friend EngineFactoryVk;
		friend CommandQueueVk;
		friend RenderDeviceVk;

		////////////////////////////////
		// Handles /////////////////////
		////////////////////////////////

		RenderDeviceVk* m_pRenderDevice;

		RefPtr<CommandQueueVk> m_spCommandQueue;

		NativeWindow m_Window;

		SurfaceTransform    m_DesiredPreTransform;
		uint32_t            m_DesiredTextureCount;
		
		vk::SurfaceKHR m_VkSurface;
		vk::SwapchainKHR m_VkSwapchain;

		uint32_t m_SemaphoreIndex;

		std::vector<bool> m_bImageAcquired;
		std::vector<vk::Fence> m_ImageAcquiredFences;
		std::vector<vk::Semaphore> m_SubmitCompleteSemaphores;

		uint32_t m_TextureIndex;

		vk::Format m_VkColorFormat;

		std::vector<ITexture*> m_FrameTextures;

		bool m_bAcquired = false;

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