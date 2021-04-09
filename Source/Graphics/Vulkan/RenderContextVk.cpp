#include "Qgfx/Graphics/Vulkan/RenderContextVk.hpp"

namespace Qgfx
{
	RenderContextVk::RenderContextVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex)
		: IRenderContext(pRefCounter)
	{
		m_Type = pRenderDevice->GetQueueType(QueueIndex);
		m_spRenderDevice = pRenderDevice;
		m_QueueIndex = QueueIndex;
	}

	RenderContextVk::~RenderContextVk()
	{
	}

	void RenderContextVk::InvalidateState()
	{
	}

	void RenderContextVk::Flush()
	{
	}

	void RenderContextVk::WaitIdle()
	{
	}
}