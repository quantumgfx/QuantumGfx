#pragma once

#include "RenderDeviceVk.hpp"
#include "CommandQueueVk.hpp"

#include "../ITexture.hpp"

namespace Qgfx
{
	class TextureVk : public ITexture
	{
	public:

		TextureVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, const TextureCreateInfo& CreateInfo);

		TextureVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, const TextureCreateInfo& CreateInfo, vk::Image VkExternalImage);

		~TextureVk();

		vk::Image GetVkImage() { return m_VkImage; }

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