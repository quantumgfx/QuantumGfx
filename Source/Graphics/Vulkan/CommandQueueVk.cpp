#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandBufferVk.hpp"
#include "Qgfx/Graphics/Vulkan/SwapChainVk.hpp"
#include "Qgfx/Common/ValidatedCast.hpp"

namespace Qgfx
{

	CommandQueueVk::CommandQueueVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex)
		: ICommandQueue(pRefCounter)
	{
		m_spRenderDevice = pRenderDevice;
		m_QueueIndex = QueueIndex;
		m_QueueType = pRenderDevice->GetQueueType(QueueIndex);

		vk::CommandPoolCreateInfo CommandPoolCI{};
		CommandPoolCI.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		CommandPoolCI.pNext = nullptr;
		CommandPoolCI.queueFamilyIndex = pRenderDevice->GetQueueFamilyIndex(QueueIndex);

		//m_DefaultCommandPool = vkq::CommandPool::create(pRenderDevice->GetVkqDevice(), CommandPoolCI);
	}

	CommandQueueVk::~CommandQueueVk()
	{
		vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();
		//m_DefaultCommandPool.destory();

		m_spRenderDevice->QueueWaitIdle(m_QueueIndex);

		CheckPendingSubmissions(true);

		while (!m_AvailablePoolsAndBuffers.empty())
		{
			CommandPoolAndBuffer& Back = m_AvailablePoolsAndBuffers.back();
			VkDevice.freeCommandBuffers(Back.Pool, Back.Buffer, VkDispatch);
			VkDevice.destroyCommandPool(Back.Pool, nullptr, VkDispatch);

			m_AvailablePoolsAndBuffers.pop_back();
		}

		while (!m_AvailableSubmissions.empty())
		{
			Submission& Back = m_AvailableSubmissions.back();
			VkDevice.destroyFence(Back.CompletetionFence, nullptr, VkDispatch);

			m_AvailableSubmissions.pop_back();
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

		if (m_AvailableSubmissions.empty())
		{
			vk::FenceCreateInfo FenceCI{};
			FenceCI.pNext = nullptr;
			FenceCI.flags = {};

			Submission Submit{};
			Submit.CompletetionFence = VkDevice.createFence(FenceCI, nullptr, VkDispatch);

			m_AvailableSubmissions.push_back(Submit);
		}

		Submission Submit = std::move(m_AvailableSubmissions.back());
		m_AvailableSubmissions.pop_back();

		std::vector<vk::CommandBuffer> CommandBuffers;

		for (uint32_t Index = 0; Index < NumCommandBuffers; Index++)
		{
			CommandBufferVk* pCommandBuffer = ValidatedCast<CommandBufferVk>(ppCommandBuffers[Index]);
			pCommandBuffer->m_State = CommandBufferState::Executing;
			Submit.CommandBuffers.emplace_back(pCommandBuffer->m_VkCmdPool, pCommandBuffer->m_VkCmdBuffer);
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

		m_spRenderDevice->QueueSubmit(m_QueueIndex, SubmitInfo, Submit.CompletetionFence);

		m_SignalSemaphores.clear();
		m_WaitSemaphores.clear();

		Submit.ExecutionIndex = m_NextExecutionIndex;
		++m_NextExecutionIndex;

		m_SubmissionsInFlight.push_back(std::move(Submit));
	}

	void CommandQueueVk::Present(const vk::PresentInfoKHR& PresentInfo)
	{
		std::lock_guard Lock{ m_Mutex };

		m_spRenderDevice->QueuePresent(m_QueueIndex, PresentInfo);
	}

	void CommandQueueVk::ReleasePoolAndBuffer(vk::CommandPool VkCmdPool, vk::CommandBuffer VkCmdBuffer)
	{
		std::lock_guard Lock{ m_Mutex };

		CommandPoolAndBuffer PoolAndBuffer{};
		PoolAndBuffer.Pool = VkCmdPool;
		PoolAndBuffer.Buffer = VkCmdBuffer;

		m_AvailablePoolsAndBuffers.push_back(PoolAndBuffer);
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

		while (!m_SubmissionsInFlight.empty())
		{
			Submission& Front = m_SubmissionsInFlight.front();

			vk::Result Res = VkDevice.getFenceStatus(Front.CompletetionFence, VkDispatch);

			if (Res == vk::Result::eErrorDeviceLost)
			{
				QGFX_LOG_ERROR_AND_THROW("Unexpected device lost when checking fence status!");
			}

			if (bForceWaitIdle)
			{
				if (Res != vk::Result::eSuccess)
					VkDevice.waitForFences(Front.CompletetionFence, true, UINT64_MAX, VkDispatch);

				Front.ExecutionIndex = 1;
				VkDevice.resetFences(Front.CompletetionFence, VkDispatch);

				for (auto& PoolAndBuffer : Front.CommandBuffers)
				{
					VkDevice.resetCommandPool(PoolAndBuffer.Pool, {}, VkDispatch);
					m_AvailablePoolsAndBuffers.push_back(std::move(PoolAndBuffer));
				}

				m_AvailableSubmissions.push_back(std::move(Front));

				m_SubmissionsInFlight.pop_front();
			}
			else
			{
				if (Res == vk::Result::eSuccess)
				{
					Front.ExecutionIndex = 1;
					VkDevice.resetFences(Front.CompletetionFence, VkDispatch);
					
					for (auto& PoolAndBuffer : Front.CommandBuffers)
					{
						VkDevice.resetCommandPool(PoolAndBuffer.Pool, {}, VkDispatch);
						m_AvailablePoolsAndBuffers.push_back(std::move(PoolAndBuffer));
					}

					m_AvailableSubmissions.push_back(std::move(Front));

					m_SubmissionsInFlight.pop_front();
				}
				else
				{
					break;
				}
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
			PoolCI.queueFamilyIndex = m_spRenderDevice->GetQueueFamilyIndex(m_QueueIndex);

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

		m_spRenderDevice->QueueWaitIdle(m_QueueIndex);
	}

}