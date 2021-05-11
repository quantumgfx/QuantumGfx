#include "Qgfx/Graphics/Vulkan/CommandBufferVk.hpp"

namespace Qgfx
{
	CommandBufferVk::CommandBufferVk(ICommandQueue* pCommandQueue, RenderDeviceVk* pRenderDevice, vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer)
		: ICommandBuffer(pCommandQueue), m_pRenderDevice(pRenderDevice), m_VkCmdPool(VkCmdPool), m_VkCmdBuffer(VkCmdBuffer)
	{
		m_State = CommandBufferState::eRecording;
	}

	CommandBufferVk::~CommandBufferVk()
	{
		if (m_State == CommandBufferState::eRecording)
		{
			m_VkCmdBuffer.end(m_pRenderDevice->GetVkDispatch());
			m_State = CommandBufferState::eReady;
		}
		
		if (m_State == CommandBufferState::eReady)
		{
			ValidatedCast<CommandQueueVk>(m_pCommandQueue)->ReleasePoolAndBuffer(m_VkCmdPool, m_VkCmdBuffer);
		}
	}

	void CommandBufferVk::Finish()
	{
		QGFX_VERIFY(m_State == CommandBufferState::eRecording, "Command Buffer must be in recording states to call ICommandBuffer::Finish()");
		m_State = CommandBufferState::eReady;
		m_VkCmdBuffer.end(m_pRenderDevice->GetVkDispatch());
	}
}