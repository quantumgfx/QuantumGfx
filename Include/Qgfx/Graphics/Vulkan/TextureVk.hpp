#pragma once

#include "RenderDeviceVk.hpp"
#include "CommandQueueVk.hpp"

#include "../ITexture.hpp"

namespace Qgfx
{
	class TextureVk : public ITexture
	{
	public:

		vk::Image GetVkImage() { return m_VkImage; }

	private:

		friend RenderDeviceVk;

		TextureVk(IRenderDevice* pRenderDevice, const TextureCreateInfo& CreateInfo);

		TextureVk(IRenderDevice* pRenderDevice, const TextureCreateInfo& CreateInfo, vk::Image VkExternalImage);

		~TextureVk();

	private:


		RefPtr<RenderDeviceVk> m_spRenderDevice;
		RefPtr<ICommandQueue> m_spCommandQueue;

		vk::Format m_VkFormat;

		vk::Image m_VkImage;

		VmaAllocation m_VmaAllocation = VMA_NULL;

		bool m_bExternalImage;

		// bool bBackedByMemory;

	};
}