#include "Qgfx/Graphics/Vulkan/TextureVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"

namespace Qgfx
{
	TextureVk::TextureVk(IRenderDevice* pRenderDevice, const TextureCreateInfo& CreateInfo)
		: ITexture(pRenderDevice, CreateInfo), m_bExternalImage(false)
	{
		m_spRenderDevice = ValidatedCast<RenderDeviceVk>(pRenderDevice);
		m_spCommandQueue = CreateInfo.pInitialQueue == nullptr ? m_spRenderDevice->GetDefaultQueue() : CreateInfo.pInitialQueue;

		vk::ImageCreateInfo ImageCI{};

		VmaAllocationCreateInfo AllocCI{};
		AllocCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		auto Texture = m_spRenderDevice->CreateVkTexture(ImageCI, AllocCI);
		m_VkImage = Texture.first;
		m_VmaAllocation = Texture.second;
	}

	TextureVk::TextureVk(IRenderDevice* pRenderDevice, const TextureCreateInfo& CreateInfo, vk::Image VkExternalImage)
		: ITexture(pRenderDevice, CreateInfo), m_VkImage(VkExternalImage), m_VmaAllocation(VMA_NULL), m_bExternalImage(true)
	{
	}

	TextureVk::~TextureVk()
	{
		if(!m_bExternalImage)
			m_spCommandQueue.Raw<CommandQueueVk>()->DeleteTextureWhenUnused(m_VkImage, m_VmaAllocation);
		
	}
}