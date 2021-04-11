#pragma once

#include "BaseVk.hpp"
#include "EngineFactoryVk.hpp"

#include "../IEngineFactory.hpp"
#include "../IRenderDevice.hpp"

#include "../../Common/RefAutoPtr.hpp"

#include <cstdint>

namespace Qgfx
{
	class SwapChainVk;

	class RenderDeviceVk final : public IRenderDevice
	{
	public:

		virtual void WaitIdle() override;

		inline virtual const DeviceFeatures& GetFeatures() const override
		{
			return m_DeviceFeatures;
		}

		inline uint32_t GetNumSupportedQueues() { return m_NumSupportedQueues; }

		void QueueWaitIdle(uint32_t QueueIndex);
		void QueueSubmit(uint32_t QueueIndex, const vk::SubmitInfo& SubmitInfo, vk::Fence Fence);
		void QueuePresent(uint32_t QueueIndex, const vk::PresentInfoKHR& PresentInfo);

		vkq::Queue GetVkqQueue(uint32_t QueueIndex);
		CommandQueueType GetQueueType(uint32_t QueueIndex);

		vkq::PhysicalDevice GetVkqPhysicalDevice() { return m_PhysicalDevice; }
		vkq::Device GetVkqDevice() { return m_LogicalDevice; }
		vkq::Instance GetVkqInstance() { return m_spEngineFactory->GetVkqInstance(); }

	private:

		RenderDeviceVk(RefCounter* pRefCounter, EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo);

		~RenderDeviceVk();

	private:

		RefAutoPtr<EngineFactoryVk> m_spEngineFactory;

		vkq::PhysicalDevice m_PhysicalDevice;
		vkq::Device m_LogicalDevice;

		DeviceFeatures m_DeviceFeatures;

		uint32_t m_NumSupportedQueues = 0;

		struct QueueDesc
		{
			uint32_t FamilyIndex;
			uint32_t Index;
			DeviceQueueInfoVk Info;
			vkq::Queue Handle;
		};

		std::vector<QueueDesc> m_Queues;
	};
}