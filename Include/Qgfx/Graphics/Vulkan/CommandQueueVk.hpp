#pragma once

#include <vector>
#include <mutex>
#include <deque>
#include <queue>

#include "BaseVk.hpp"
#include "MemAllocVk.hpp"
#include "RenderDeviceVk.hpp"
#include "HardwareQueueVk.hpp"

#include "../ICommandQueue.hpp"

#include "../../Common/PoolAllocator.hpp"

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

		RefPtr<RenderDeviceVk> m_spRenderDevice;

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

		RefPtr<RenderDeviceVk> m_spRenderDevice;

		std::vector<vk::Semaphore> m_SemaphorePool;


	};

	class CommandQueueVk final : public ICommandQueue
	{
	public:

		CommandQueueVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, HardwareQueueVk* pHardwareQueue, bool bIsDefaultQueue);

		~CommandQueueVk();

		virtual CommandQueueType GetType() const override { return m_pHardwareQueue->GetQueueType(); }

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

		void DeleteTextureWhenUnused(vk::Image Image, VmaAllocation Allocation);

	private:

		void CheckPendingSubmissions(bool bForceWaitIdle);

	private:

		// Strong reference to device. Used by non default queues to keep device alive.
		RefPtr<RenderDeviceVk> m_spRenderDevice;

		RenderDeviceVk* const m_pRenderDevice;

		HardwareQueueVk* const m_pHardwareQueue;

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

		struct TextureToDelete
		{
			uint32_t Index;
			vk::Image Image;
			VmaAllocation Allocation;
		};

		std::deque<TextureToDelete> m_TexturesToDelete;

		//////////////////////////
		// Handles ///////////////
		//////////////////////////

		std::mutex m_Mutex;

		PoolAllocator m_CommandBufferHandleAllocator;

		FencePoolVk m_FencePool;

		BinarySemaphorePoolVk m_AcquiredSemaphorePool;

		//////////////////////////
		// Basic Settings ////////
		//////////////////////////

	};
}