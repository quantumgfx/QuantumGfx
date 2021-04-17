#include "Qgfx/Graphics/Vulkan/FenceVk.hpp"

namespace Qgfx
{
	TimelineFenceVk::TimelineFenceVk(RefCounter* pRefCounter, RenderDeviceVk* pDevice, uint64_t InitialValue)
		: FenceVk(pRefCounter)
	{
		m_spRenderDevice = pDevice;

		vk::SemaphoreTypeCreateInfo SemaphoreTypeCI{};
		SemaphoreTypeCI.pNext = nullptr;
		SemaphoreTypeCI.semaphoreType = vk::SemaphoreType::eTimeline;
		SemaphoreTypeCI.initialValue = InitialValue;

		vk::SemaphoreCreateInfo SemaphoreCI{};
		SemaphoreCI.flags = {};
		SemaphoreCI.pNext = &SemaphoreTypeCI;

		m_VkSemaphore = m_spRenderDevice->GetVkqDevice().createSemaphore(SemaphoreCI);
	}

	TimelineFenceVk::~TimelineFenceVk()
	{
		// This is allowed because any queues that reference the fence will hold a reference to it until that queue submission is complete.
		m_spRenderDevice->GetVkqDevice().destroySemaphore(m_VkSemaphore);
	}

	uint64_t TimelineFenceVk::GetCompletedValue()
	{
		return m_spRenderDevice->GetVkqDevice().getSemaphoreCounterValue(m_VkSemaphore);
	}

	void TimelineFenceVk::Signal(uint64_t Value)
	{
		vk::SemaphoreSignalInfo SignalInfo{};
		SignalInfo.pNext = nullptr;
		SignalInfo.semaphore = m_VkSemaphore;
		SignalInfo.value = Value;

		m_spRenderDevice->GetVkqDevice().signalSemaphore(SignalInfo);
	}
	void TimelineFenceVk::Wait(uint64_t Value)
	{
		vk::SemaphoreWaitInfo WaitInfo{};
		WaitInfo.pNext = nullptr;
		WaitInfo.flags = {};
		WaitInfo.semaphoreCount = 1;
		WaitInfo.pSemaphores = &m_VkSemaphore;
		WaitInfo.pValues = &Value;

		vk::Result res = m_spRenderDevice->GetVkqDevice().waitSemaphore(WaitInfo, UINT64_MAX);

		if (res != vk::Result::eSuccess)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to wait for timeline semaphore");
		}
	}
}