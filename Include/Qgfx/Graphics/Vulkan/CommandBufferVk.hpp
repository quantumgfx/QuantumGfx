#pragma once

#include "../ICommandBuffer.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"
#include "CommandQueueVk.hpp"

#include <vector>

namespace Qgfx
{

	class CommandBufferVk : public ICommandBuffer
	{
	public:

		CommandBufferVk(ICommandQueue* pCommandQueue, RenderDeviceVk* pRenderDevice, vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer);

		~CommandBufferVk();

		virtual void Finish() override;

	private:

		friend class CommandQueueVk;

		RenderDeviceVk* const m_pRenderDevice;

		vk::CommandPool m_VkCmdPool;
		vk::CommandBuffer m_VkCmdBuffer;

		CommandBufferState m_State;
	};

}