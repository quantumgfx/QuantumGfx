#pragma once

#include "../ICommandQueue.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"

#include <vector>
#include <mutex>

namespace Qgfx
{
	class CommandQueueVk final : public ICommandQueue
	{
	public:

		CommandQueueVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex);

		~CommandQueueVk();

		virtual CommandQueueType GetType() const override { return m_QueueType; }

		virtual void WaitIdle() override;

	private:

		//////////////////////////
		// Handles ///////////////
		//////////////////////////

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		std::mutex m_Mutex;

		//////////////////////////
		// Basic Settings ////////
		//////////////////////////

		CommandQueueType m_QueueType;

		uint32_t m_QueueIndex;

		//////////////////////////
		// Utility Objects ///////
		//////////////////////////

		vkq::CommandPool m_DefaultCommandPool;
	};
}