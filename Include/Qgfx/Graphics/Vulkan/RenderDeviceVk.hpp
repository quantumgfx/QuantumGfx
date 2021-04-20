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

		RenderDeviceVk(RefCounter* pRefCounter, EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo);

		~RenderDeviceVk();

		///////////////////////////
		// IRenderDevice Funcs ////
		///////////////////////////

		inline virtual const DeviceFeatures& GetFeatures() const override { return m_DeviceFeatures; }

		inline uint32_t GetNumSupportedQueues() { return m_NumSupportedQueues; }

		virtual void CreateFence(uint64_t InitialValue, IFence** ppFence) override;

		virtual void WaitIdle() override;

		///////////////////////////
		// Native Vk Functions ////
		///////////////////////////

		// Queues

		vk::Queue GetVkQueue(uint32_t QueueIndex);

		uint32_t GetQueueFamilyIndex(uint32_t QueueIndex);

		CommandQueueType GetQueueType(uint32_t QueueIndex);

		void QueueWaitIdle(uint32_t QueueIndex);
		void QueueSubmit(uint32_t QueueIndex, const vk::SubmitInfo& SubmitInfo, vk::Fence Fence);
		void QueuePresent(uint32_t QueueIndex, const vk::PresentInfoKHR& PresentInfo);

		// Device

		inline vk::Device GetVkDevice() const { return m_VkDevice; }

		vk::Semaphore CreateVkBinarySemaphore() const;

		void DestroyVkSemaphore(vk::Semaphore Sem) const;

		// Physical Device

		inline vk::PhysicalDevice GetVkPhysicalDevice() const { return m_VkPhysicalDevice; }

		// Instance

		inline vk::Instance GetVkInstance() const { return m_spEngineFactory->GetVkInstance(); }

		// Dispatch

		inline const vk::DispatchLoaderDynamic& GetVkDispatch() const { return m_VkDispatch; }

	private:

		RefAutoPtr<EngineFactoryVk> m_spEngineFactory;

		vk::DispatchLoaderDynamic m_VkDispatch;
		vk::PhysicalDevice m_VkPhysicalDevice;
		vk::Device m_VkDevice;

		DeviceFeatures m_DeviceFeatures;

		uint32_t m_NumSupportedQueues = 0;

		struct QueueDesc
		{
			uint32_t FamilyIndex;
			uint32_t Index;
			DeviceQueueInfoVk Info;
			vk::Queue Handle;
		};

		std::vector<QueueDesc> m_Queues;

		bool m_bTimelineSemaphoresSupported = false;
	};
}