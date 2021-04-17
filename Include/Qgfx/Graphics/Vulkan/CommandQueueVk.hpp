#pragma once

#include "../../Common/PoolAllocator.hpp"

#include "../ICommandQueue.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"

#include <vector>
#include <mutex>
#include <deque>
#include <queue>

namespace Qgfx
{
	class CommandQueueVk final : public ICommandQueue
	{
	public:

		CommandQueueVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex);

		~CommandQueueVk();

		virtual CommandQueueType GetType() const override { return m_QueueType; }

		virtual void CreateCommandBuffer(ICommandBuffer** ppCommandBuffer) override;

		virtual void WaitIdle() override;

		virtual void SubmitCommandBuffers(uint32_t NumCommandBuffers, ICommandBuffer** ppCommandBuffers) override;

		void Present(const vk::PresentInfoKHR& PresentInfo);

		void AddSignalSemaphore(vk::Semaphore Signal);

		void AddWaitSemaphore(vk::Semaphore Wait);

		void ReleasePoolAndBuffer(vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer);

	private:

		void CheckPendingSubmissions(bool bForceWaitIdle);

		struct CommandPoolAndBuffer
		{
			vk::CommandPool Pool;
			vk::CommandBuffer Buffer;

			CommandPoolAndBuffer(vk::CommandPool Pool = nullptr, vk::CommandBuffer Buffer = nullptr)
				: Pool(Pool), Buffer(Buffer)
			{
			}
		};

		std::vector<CommandPoolAndBuffer> m_AvailablePoolsAndBuffers;

		std::vector<vk::Semaphore> m_SignalSemaphores;
		std::vector<vk::Semaphore> m_WaitSemaphores;

		struct Submission
		{
			vk::Fence CompletetionFence;
			std::vector<CommandPoolAndBuffer> CommandBuffers;
			uint64_t ExecutionIndex;
		};

		std::deque<Submission> m_SubmissionsInFlight;

		std::vector<Submission> m_AvailableSubmissions;

		uint64_t m_NextExecutionIndex = 1;

		//////////////////////////
		// Handles ///////////////
		//////////////////////////

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		std::mutex m_Mutex;

		PoolAllocator m_CommandBufferHandleAllocator;

		//////////////////////////
		// Basic Settings ////////
		//////////////////////////

		CommandQueueType m_QueueType;

		uint32_t m_QueueIndex;

	};
}