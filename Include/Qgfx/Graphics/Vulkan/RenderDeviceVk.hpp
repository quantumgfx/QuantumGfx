#pragma once

#include "BaseVk.hpp"
#include "MemAllocVk.hpp"
#include "EngineFactoryVk.hpp"

#include "../IEngineFactory.hpp"
#include "../IRenderDevice.hpp"

#include "../../Common/RefPtr.hpp"

#include <cstdint>

namespace Qgfx
{
	class SwapChainVk;
	class HardwareQueueVk;

	class RenderDeviceVk final : public IRenderDevice
	{
	public:

		RenderDeviceVk(IRefCounter* pRefCounter, EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo, const ArrayProxy<HardwareQueueInfoVk>& RequestedExtraHardwareQueues);

		~RenderDeviceVk();

		///////////////////////////
		// IRenderDevice Funcs ////
		///////////////////////////

		inline virtual const DeviceFeatures& GetFeatures() const override { return m_DeviceFeatures; }

		// virtual void CreateFence(uint64_t InitialValue, IFence** ppFence) override;

		virtual void WaitIdle() override;

		virtual ICommandQueue* GetDefaultQueue() override { return m_spDefaultCommandQueue; }

		virtual void CreateBuffer(const BufferCreateInfo& CreateInfo, IBuffer** ppBuffer) override;

		virtual void CreateTexture(const TextureCreateInfo& CreateInfo, ITexture** ppTexture) override;

		void CreateTextureFromVkImage(const TextureCreateInfo& CreateInfo, vk::Image VkImage, ITexture** ppTexture);

		///////////////////////////
		// Native Vk Functions ////
		///////////////////////////

		// Queues

		HardwareQueueVk* GetDefaultHardwareQueue();

		uint32_t GetNumExtraHardwareQueues();

		HardwareQueueVk* GetExtraHardwareQueue(uint32_t HardwareQueueIndex);

		// Device

		inline vk::Device GetVkDevice() const { return m_VkDevice; }

		vk::Semaphore CreateVkBinarySemaphore() const;

		void DestroyVkSemaphore(vk::Semaphore Sem) const;

		std::pair<vk::Image, VmaAllocation> CreateVkTexture(const vk::ImageCreateInfo& ImageCI, const VmaAllocationCreateInfo& AllocCI);

		void DestroyVkTexture(vk::Image Image, VmaAllocation Allocation) const;

		// Vma Allocator

		inline VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }

		// Physical Device

		inline vk::PhysicalDevice GetVkPhysicalDevice() const { return m_VkPhysicalDevice; }

		// Instance

		inline vk::Instance GetVkInstance() const { return m_spEngineFactory->GetVkInstance(); }

		// Dispatch

		inline const vk::DispatchLoaderDynamic& GetVkDispatch() const { return m_VkDispatch; }

	private:

		vk::DispatchLoaderDynamic m_VkDispatch;
		vk::PhysicalDevice m_VkPhysicalDevice;
		vk::Device m_VkDevice;

		VmaAllocator m_VmaAllocator;

		HardwareQueueVk* m_pDefaultHardwareQueue;

		std::vector<HardwareQueueVk*> m_ExtraHardwareQueues;

		RefPtr<EngineFactoryVk> m_spEngineFactory;

		DeviceFeatures m_DeviceFeatures;

		bool m_bTimelineSemaphoresSupported = false;

		RefPtr<ICommandQueue> m_spDefaultCommandQueue;
	};
}