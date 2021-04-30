#include "Qgfx/Graphics/Vulkan/CommandBufferVk.hpp"

namespace Qgfx
{
	CommandBufferVk::CommandBufferVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, CommandQueueVk* pCommandQueue, vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer)
		: ICommandBuffer(pRefCounter)
	{
		m_State = CommandBufferState::Recording;
		m_spRenderDevice = pRenderDevice;
		m_spCommandQueue = pCommandQueue;
		m_VkCmdPool = VkCmdPool;
		m_VkCmdBuffer = VkCmdBuffer;
	}

	CommandBufferVk::~CommandBufferVk()
	{
		if (m_State == CommandBufferState::Recording)
		{
			m_VkCmdBuffer.end(m_spRenderDevice->GetVkDispatch());
			m_State = CommandBufferState::Ready;
		}
		
		if (m_State == CommandBufferState::Ready)
		{
			m_spCommandQueue->ReleasePoolAndBuffer(m_VkCmdPool, m_VkCmdBuffer);
		}
	}

	void CommandBufferVk::Finish()
	{
		QGFX_VERIFY(m_State == CommandBufferState::Recording, "Command Buffer must be in recording states to call ICommandBuffer::Finish()");
		m_State = CommandBufferState::Ready;
		m_VkCmdBuffer.end(m_spRenderDevice->GetVkDispatch());
	}
}