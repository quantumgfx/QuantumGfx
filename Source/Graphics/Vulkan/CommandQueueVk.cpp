#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"

namespace Qgfx
{

	CommandQueueVk::CommandQueueVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex)
		: ICommandQueue(pRefCounter)
	{
		m_spRenderDevice = pRenderDevice;
		m_QueueIndex = QueueIndex;
		m_QueueType = pRenderDevice->GetQueueType(QueueIndex);
	}

	CommandQueueVk::~CommandQueueVk()
	{
	}

	void CommandQueueVk::WaitIdle()
	{
		m_spRenderDevice->QueueWaitIdle(m_QueueIndex);
	}

}