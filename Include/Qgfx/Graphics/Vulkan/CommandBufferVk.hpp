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

		CommandBufferVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, CommandQueueVk* pCommandQueue, vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer);

		~CommandBufferVk();

		virtual void Finish() override;

		virtual CommandBufferState GetCurrentState() override { return m_State; }

	private:

		friend class CommandQueueVk;

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;
		RefAutoPtr<CommandQueueVk> m_spCommandQueue;

		vk::CommandPool m_VkCmdPool;
		vk::CommandBuffer m_VkCmdBuffer;

		CommandBufferState m_State;
	};

}