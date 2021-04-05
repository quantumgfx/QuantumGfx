#pragma once

#include "BaseVk.hpp"
#include "InstanceVk.hpp"

#include "../IInstance.hpp"
#include "../IRenderDevice.hpp"

#include "../../Common/RefAutoPtr.hpp"

#include <cstdint>

namespace Qgfx
{
	

	class RenderDeviceVk final : public IRenderDevice
	{
	public:

	private:

		RenderDeviceVk(RefCounter* pRefCounter, InstanceVk* pInstance, const RenderDeviceCreateInfo& CreateInfo, vkq::PhysicalDevice VkPhDev);

		~RenderDeviceVk();

		virtual void CreateSwapChain(const SwapChainDesc& Desc, const NativeWindow& Window, IRenderContext* pContext, ISwapChain** ppSwapChain) override;

		virtual void WaitIdle() override;

		inline virtual const DeviceFeatures& GetFeatures() const override
		{
			return m_DeviceFeatures;
		}

	private:

		RefAutoPtr<InstanceVk> m_spInstance;

		vkq::PhysicalDevice m_PhysicalDevice;
		vkq::Device m_LogicalDevice;

		DeviceFeatures m_DeviceFeatures;

		uint32_t GraphicsQueueFamily = UINT32_MAX;
		uint32_t GraphicsNumQueues = 0;
		std::vector<vkq::Queue> GraphicsQueues;

		uint32_t ComputeQueueFamily = UINT32_MAX;
		uint32_t ComputeNumQueues = 0;
		std::vector<vkq::Queue> ComputeQueues;

		uint32_t TransferQueueFamily = UINT32_MAX;
		uint32_t TransferNumQueues = 0;
		std::vector<vkq::Queue> TransferQueues;
	};
}