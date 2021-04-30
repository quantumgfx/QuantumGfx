#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandBufferVk.hpp"
#include "Qgfx/Graphics/Vulkan/SwapChainVk.hpp"
#include "Qgfx/Common/ValidatedCast.hpp"

namespace Qgfx
{

	FencePoolVk::FencePoolVk(RenderDeviceVk* pRenderDevice)
	{
		m_spRenderDevice = pRenderDevice;
	}

	FencePoolVk::~FencePoolVk()
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		for (auto Fence : m_FencePool)
		{
			VkDevice.destroyFence(Fence, nullptr, VkDispatch);
		}

		m_FencePool.clear();
	}

	vk::Fence FencePoolVk::GetFence()
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		vk::Fence Fence{};
		if (!m_FencePool.empty())
		{
			Fence = m_FencePool.back();
			VkDevice.resetFences(Fence, VkDispatch);
			m_FencePool.pop_back();
		}
		else
		{
			vk::FenceCreateInfo FenceCI{};
			FenceCI.pNext = nullptr;
			FenceCI.flags = {};

			Fence = VkDevice.createFence(FenceCI, nullptr, VkDispatch);
		}
	}

	void FencePoolVk::DisposeFence(vk::Fence FenceVk)
	{
		m_FencePool.push_back(FenceVk);
	}

	BinarySemaphorePoolVk::BinarySemaphorePoolVk(RenderDeviceVk* pRenderDevice)
	{
		m_spRenderDevice = pRenderDevice;
	}

	BinarySemaphorePoolVk::~BinarySemaphorePoolVk()
	{
		for (auto Semaphore : m_SemaphorePool)
		{
			m_spRenderDevice->DestroyVkSemaphore(Semaphore);
		}

		m_SemaphorePool.clear();
	}

	vk::Semaphore BinarySemaphorePoolVk::GetSemaphore()
	{

		vk::Semaphore Semaphore{};
		if (!m_SemaphorePool.empty())
		{
			Semaphore = m_SemaphorePool.back();
			m_SemaphorePool.pop_back();
		}
		else
		{
			Semaphore = m_spRenderDevice->CreateVkBinarySemaphore();
		}

		return Semaphore;
	}

	void BinarySemaphorePoolVk::DestroySemaphore(vk::Semaphore SemaphoreVk)
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		VkDevice.destroySemaphore(SemaphoreVk, nullptr, VkDispatch);
	}

	void BinarySemaphorePoolVk::RecycleSemaphore(vk::Semaphore SemaphoreVk)
	{
		m_SemaphorePool.push_back(SemaphoreVk);
	}

	CommandQueueVk::CommandQueueVk(IRefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, HardwareQueueVk* pHardwareQueue, bool bIsDefaultQueue)
		: ICommandQueue(pRefCounter), m_pRenderDevice(pRenderDevice), m_pHardwareQueue(pHardwareQueue), m_FencePool(pRenderDevice), m_AcquiredSemaphorePool(pRenderDevice)
	{
		m_spRenderDevice = bIsDefaultQueue ? nullptr : pRenderDevice;
	}

	CommandQueueVk::~CommandQueueVk()
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		m_pHardwareQueue->WaitIdle();

		CheckPendingSubmissions(true);

		while (!m_AvailablePoolsAndBuffers.empty())
		{
			CommandPoolAndBuffer& Back = m_AvailablePoolsAndBuffers.back();
			VkDevice.freeCommandBuffers(Back.Pool, Back.Buffer, VkDispatch);
			VkDevice.destroyCommandPool(Back.Pool, nullptr, VkDispatch);

			m_AvailablePoolsAndBuffers.pop_back();
		}
	}

	void CommandQueueVk::SubmitCommandBuffers(uint32_t NumCommandBuffers, ICommandBuffer** ppCommandBuffers)
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		std::lock_guard Lock{ m_Mutex };

		CheckPendingSubmissions(false);

		if (NumCommandBuffers == 0 && m_WaitSemaphores.size() == 0 && m_SignalSemaphores.size() == 0)
			return;

		vk::FenceCreateInfo FenceCI{};
		FenceCI.pNext = nullptr;
		FenceCI.flags = {};

		vk::Fence CompletionFence = m_FencePool.GetFence();

		std::vector<vk::CommandBuffer> CommandBuffers;

		for (uint32_t Index = 0; Index < NumCommandBuffers; Index++)
		{
			CommandBufferVk* pCommandBuffer = ValidatedCast<CommandBufferVk>(ppCommandBuffers[Index]);
			pCommandBuffer->m_State = CommandBufferState::Executing;

			CommandBufferToFree CmdBufferToFree{};
			CmdBufferToFree.Index = m_NextSubmissionIndex;
			CmdBufferToFree.PoolAndBuffer = { pCommandBuffer->m_VkCmdPool, pCommandBuffer->m_VkCmdBuffer };

			m_CommandBuffersToFree.push_back(CmdBufferToFree);

			CommandBuffers.push_back(pCommandBuffer->m_VkCmdBuffer);
			pCommandBuffer->m_VkCmdPool = nullptr;
			pCommandBuffer->m_VkCmdBuffer = nullptr;
		}

		std::vector<vk::PipelineStageFlags> WaitStageMasks{ m_WaitSemaphores.size(), vk::PipelineStageFlagBits::eAllCommands };

		vk::SubmitInfo SubmitInfo{};
		SubmitInfo.pNext = nullptr;
		SubmitInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());
		SubmitInfo.pCommandBuffers = nullptr;
		SubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(m_SignalSemaphores.size());
		SubmitInfo.pSignalSemaphores = m_SignalSemaphores.data();
		SubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(m_WaitSemaphores.size());
		SubmitInfo.pWaitSemaphores = m_WaitSemaphores.data();
		SubmitInfo.pWaitDstStageMask = WaitStageMasks.data();

		m_pHardwareQueue->Submit(SubmitInfo, CompletionFence);

		m_SignalSemaphores.clear();
		m_WaitSemaphores.clear();

		SubmissionFence Submission{};
		Submission.Index = m_NextSubmissionIndex;
		Submission.CompletetionFence = CompletionFence;

		m_SubmissionFences.push_back(std::move(Submission));

		++m_NextSubmissionIndex;
	}

	void CommandQueueVk::Present(const vk::PresentInfoKHR& PresentInfo)
	{
		std::lock_guard Lock{ m_Mutex };

		m_pHardwareQueue->Present(PresentInfo);
	}

	void CommandQueueVk::ReleasePoolAndBuffer(vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer)
	{
		std::lock_guard Lock{ m_Mutex };

		CommandPoolAndBuffer PoolAndBuffer{};
		PoolAndBuffer.Pool = VkCmdPool;
		PoolAndBuffer.Buffer = VkCmdBuffer;

		m_AvailablePoolsAndBuffers.push_back(PoolAndBuffer);
	}

	void CommandQueueVk::DeleteSemaphoreWhenUnused(vk::Semaphore Semaphore)
	{
		std::lock_guard Lock{ m_Mutex };

		SemaphoreToDelete SemToDelete{};
		SemToDelete.Index = m_NextSubmissionIndex;
		SemToDelete.Semaphore = Semaphore;

		m_SemaphoresToDelete.push_back(SemToDelete);
	}

	void CommandQueueVk::DeleteTextureWhenUnused(vk::Image Image, VmaAllocation Allocation)
	{
		std::lock_guard Lock{ m_Mutex };

		TextureToDelete TexToDelete{};
		TexToDelete.Index = m_NextSubmissionIndex;
		TexToDelete.Image = Image;
		TexToDelete.Allocation = Allocation;

		m_TexturesToDelete.push_back(TexToDelete);
	}

	void CommandQueueVk::AddSignalSemaphore(vk::Semaphore Signal)
	{
		std::lock_guard Lock{ m_Mutex };

		m_SignalSemaphores.push_back(Signal);
	}

	void CommandQueueVk::AddWaitSemaphore(vk::Semaphore Wait)
	{
		std::lock_guard Lock{ m_Mutex };

		m_WaitSemaphores.push_back(Wait);
	}

	void CommandQueueVk::CheckPendingSubmissions(bool bForceWaitIdle)
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		while (!m_SubmissionFences.empty())
		{
			SubmissionFence& Fence = m_SubmissionFences.front();
			
			vk::Result Res = VkDevice.getFenceStatus(Fence.CompletetionFence, VkDispatch);

			bool bCanRemove = (Res == vk::Result::eSuccess);

			if(bForceWaitIdle)
				if (Res == vk::Result::eNotReady)
				{
					VkDevice.waitForFences(Fence.CompletetionFence, true, UINT64_MAX, VkDispatch);
					bCanRemove = true;
				}

			if (bCanRemove)
			{
				m_CompletedSubmissionIndex = std::max(m_CompletedSubmissionIndex, Fence.Index);
				m_FencePool.DisposeFence(Fence.CompletetionFence);
				m_SubmissionFences.pop_front();
			}
			else
			{
				break;
			}
		}

		while (!m_CommandBuffersToFree.empty())
		{
			CommandBufferToFree& CmdToFree = m_CommandBuffersToFree.front();

			if (CmdToFree.Index <= m_CompletedSubmissionIndex)
			{
				VkDevice.resetCommandPool(CmdToFree.PoolAndBuffer.Pool, {}, VkDispatch);
				m_AvailablePoolsAndBuffers.push_back(CmdToFree.PoolAndBuffer);
				m_CommandBuffersToFree.pop_front();
			}
			else
			{
				break;
			}
		}

		while (!m_SemaphoresToDelete.empty())
		{
			SemaphoreToDelete& SemToDelete = m_SemaphoresToDelete.front();

			if (SemToDelete.Index <= m_CompletedSubmissionIndex)
			{
				m_spRenderDevice->DestroyVkSemaphore(SemToDelete.Semaphore);
				m_SemaphoresToDelete.pop_front();
			}
			else
			{
				break;
			}
		}

		while (!m_TexturesToDelete.empty())
		{
			TextureToDelete& TexToDelete = m_TexturesToDelete.front();

			if (TexToDelete.Index <= m_CompletedSubmissionIndex)
			{
				m_spRenderDevice->DestroyVkTexture(TexToDelete.Image, TexToDelete.Allocation);
				m_TexturesToDelete.pop_front();
			}
			else
			{
				break;
			}
		}
	}

	void CommandQueueVk::CreateCommandBuffer(ICommandBuffer** ppCommandBuffer)
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

		std::lock_guard Lock{ m_Mutex };

		if (m_AvailablePoolsAndBuffers.empty())
		{
			CommandPoolAndBuffer PoolAndBuffer{};

			vk::CommandPoolCreateInfo PoolCI{};
			PoolCI.pNext = nullptr;
			PoolCI.flags = {};
			PoolCI.queueFamilyIndex = m_pHardwareQueue->GetVkQueueFamilyIndex();

			PoolAndBuffer.Pool = VkDevice.createCommandPool(PoolCI, nullptr, VkDispatch);

			vk::CommandBufferAllocateInfo AllocInfo{};
			AllocInfo.pNext = nullptr;
			AllocInfo.commandPool = PoolAndBuffer.Pool;
			AllocInfo.level = vk::CommandBufferLevel::ePrimary;
			AllocInfo.commandBufferCount = 1;

			vk::throwResultException(VkDevice.allocateCommandBuffers(&AllocInfo, &PoolAndBuffer.Buffer, VkDispatch), "Failed to allocate command buffer");

			m_AvailablePoolsAndBuffers.push_back(PoolAndBuffer);
		}

		CommandPoolAndBuffer PoolAndBuffer = m_AvailablePoolsAndBuffers.back();
		m_AvailablePoolsAndBuffers.pop_back();

		vk::CommandBufferBeginInfo BeginInfo{};
		BeginInfo.pNext = nullptr;
		BeginInfo.pInheritanceInfo = nullptr;
		BeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		PoolAndBuffer.Buffer.begin(BeginInfo, VkDispatch);

		*ppCommandBuffer = MakeRefCountedObj<CommandBufferVk, PoolAllocator>(m_CommandBufferHandleAllocator, this)(m_spRenderDevice.Raw(), this, PoolAndBuffer.Pool, PoolAndBuffer.Buffer);
	}

	void CommandQueueVk::WaitIdle()
	{
		std::lock_guard Lock{ m_Mutex };

		m_pHardwareQueue->WaitIdle();
	}

}