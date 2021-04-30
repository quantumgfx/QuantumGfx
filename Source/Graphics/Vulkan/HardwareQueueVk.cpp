#include "Qgfx/Graphics/Vulkan/HardwareQueueVk.hpp"

namespace Qgfx
{
	HardwareQueueVk::HardwareQueueVk(RenderDeviceVk* pRenderDevice, const HardwareQueueInfoVk& Info, uint32_t QueueFamilyIndex, uint32_t QueueIndex)
	{
		m_pRenderDevice = pRenderDevice;

		vk::Device VkDevice = pRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = pRenderDevice->GetVkDispatch();

		m_VkQueueFamilyIndex = QueueFamilyIndex;
		m_VkQueueIndex = QueueIndex;
		m_VkQueue = VkDevice.getQueue(QueueFamilyIndex, QueueIndex, VkDispatch);
	}

	HardwareQueueVk::~HardwareQueueVk()
	{
	}

	void HardwareQueueVk::WaitIdle()
	{
		const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();

		m_VkQueue.waitIdle(VkDispatch);
	}

	void HardwareQueueVk::Submit(const vk::ArrayProxy<const vk::SubmitInfo>& Submits, vk::Fence Fence)
	{

		const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();

		m_VkQueue.submit(Submits, Fence, VkDispatch);
	}

	void HardwareQueueVk::Present(const vk::PresentInfoKHR& PresentInfo)
	{
		const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();

		m_VkQueue.presentKHR(PresentInfo, VkDispatch);
	}
}