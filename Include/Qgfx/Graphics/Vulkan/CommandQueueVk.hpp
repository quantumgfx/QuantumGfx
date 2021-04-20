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
	class FencePoolVk
	{
	public:

		FencePoolVk(RenderDeviceVk* pRenderDevice);
		~FencePoolVk();

		vk::Fence GetFence();

		void DisposeFence(vk::Fence FenceVk);

	private:

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		std::vector<vk::Fence> m_FencePool;
	};

	class BinarySemaphorePoolVk
	{
	public:

		BinarySemaphorePoolVk(RenderDeviceVk* pRenderDevice);
		~BinarySemaphorePoolVk();

		vk::Semaphore GetSemaphore();

		void DestroySemaphore(vk::Semaphore SemaphoreVk);

		void RecycleSemaphore(vk::Semaphore SemaphoreVk);

	private:

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		std::vector<vk::Semaphore> m_SemaphorePool;


	};

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

		void ReleasePoolAndBuffer(vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer);

		/////////////////////////
		// Semaphores ///////////
		/////////////////////////

		void AddSignalSemaphore(vk::Semaphore Signal);

		void AddWaitSemaphore(vk::Semaphore Wait);

		void DeleteSemaphoreWhenUnused(vk::Semaphore Semaphore);

		/*vk::Semaphore GetAcquireSemaphore();

		void DestroyAcquireSemaphore(vk::Semaphore Semaphore);

		void RecycleAcquireSemaphoreOnceUnused();*/

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

		uint64_t m_CompletedSubmissionIndex = 0;
		uint64_t m_NextSubmissionIndex = 1;

		struct SubmissionFence
		{
			uint64_t Index;
			vk::Fence CompletetionFence;
		};

		std::deque<SubmissionFence> m_SubmissionFences;

		struct CommandBufferToFree
		{
			uint64_t Index;
			CommandPoolAndBuffer PoolAndBuffer;
		};

		std::deque<CommandBufferToFree> m_CommandBuffersToFree;

		struct SemaphoreToDelete
		{
			uint32_t Index;
			vk::Semaphore Semaphore;
		};

		std::deque<SemaphoreToDelete> m_SemaphoresToDelete;

		//////////////////////////
		// Handles ///////////////
		//////////////////////////

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		std::mutex m_Mutex;

		PoolAllocator m_CommandBufferHandleAllocator;

		FencePoolVk m_FencePool;

		BinarySemaphorePoolVk m_AcquiredSemaphorePool;

		//////////////////////////
		// Basic Settings ////////
		//////////////////////////

		CommandQueueType m_QueueType;

		uint32_t m_QueueIndex;

	};
}