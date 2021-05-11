#pragma once

#include "BaseVk.hpp"
#include "MemAllocVk.hpp"
#include "EngineFactoryVk.hpp"

#include "../IEngineFactory.hpp"
#include "../IRenderDevice.hpp"
#include "../StateObjectsRegistry.hpp"

#include "../../Common/RefPtr.hpp"
#include "../../Common/FixedBlockMemoryAllocator.hpp"

#include <cstdint>

namespace Qgfx
{
	class BufferVk;
	class TextureVk;
	class SwapChainVk;
	class HardwareQueueVk;

	class RenderDeviceVk final : public IRenderDevice
	{

		friend SwapChainVk;

	public:

		///////////////////////////
		// IRenderDevice Funcs ////
		///////////////////////////
		// 
		// virtual void CreateFence(uint64_t InitialValue, IFence** ppFence) override;

		virtual void WaitIdle() override;

		virtual IBuffer* CreateBuffer(const BufferCreateInfo& CreateInfo) override;

		virtual ITexture* CreateTexture(const TextureCreateInfo& CreateInfo) override;

		ITexture* CreateTextureFromVkImage(const TextureCreateInfo& CreateInfo, vk::Image VkImage);

		// virtual void CreateSampler(const SamplerCreateInfo& CreateInfo) override;

		// void CreateShaderModule(const ShaderModuleCreateInfo& CreateInfo, IShaderModule** ppShaderModule);

		// void DestroyShaderModule(IShaderModule* ShaderModule);

		virtual ISwapChain* CreateSwapChain(const SwapChainCreateInfo& CreateInfo) override;

		virtual void Destroy() = 0;

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

		friend EngineFactoryVk;

		void DestroyBuffer(BufferVk* pBuffer);
		void DestroySwapChain(SwapChainVk* pSwapChain);
		void DestroyTexture(TextureVk* pTexture);

		RenderDeviceVk(EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo, IMemoryAllocator& RawMemAllocator);

		~RenderDeviceVk();

		RefPtr<EngineFactoryVk> m_spEngineFactory;

		vk::DispatchLoaderDynamic m_VkDispatch;
		vk::PhysicalDevice m_VkPhysicalDevice;
		vk::Device m_VkDevice;

		VmaAllocator m_VmaAllocator;

		HardwareQueueVk* m_pDefaultHardwareQueue;

		std::vector<HardwareQueueVk*> m_ExtraHardwareQueues;

		ICommandQueue* m_pDefaultCommandQueue;

		FixedBlockMemoryAllocator m_TextureObjAllocator;
	};
}