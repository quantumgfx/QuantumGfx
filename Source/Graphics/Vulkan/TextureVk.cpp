#include "Qgfx/Graphics/Vulkan/TextureVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"

namespace Qgfx
{
	TextureVk::TextureVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, const TextureCreateInfo& CreateInfo)
		: ITexture(pRefCounter, CreateInfo), m_bExternalImage(false)
	{
		m_spRenderDevice = pRenderDevice;

		vk::ImageCreateInfo ImageCI{};

		VmaAllocationCreateInfo AllocCI{};
		AllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		auto Texture = m_spRenderDevice->CreateVkTexture(ImageCI, AllocCI);
		m_VkImage = Texture.first;
		m_VmaAllocation = Texture.second;

		m_spCommandQueue = CreateInfo.pInitialQueue == nullptr ? pRenderDevice->GetDefaultQueue() : CreateInfo.pInitialQueue;
	}

	TextureVk::TextureVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, const TextureCreateInfo& CreateInfo, vk::Image VkExternalImage)
		: ITexture(pRefCounter, CreateInfo), m_VkImage(VkExternalImage), m_VmaAllocation(VMA_NULL), m_bExternalImage(true)
	{
	}

	TextureVk::~TextureVk()
	{
		if(!m_bExternalImage)
			m_spCommandQueue.Raw<CommandQueueVk>()->DeleteTextureWhenUnused(m_VkImage, m_VmaAllocation);
		
	}
}