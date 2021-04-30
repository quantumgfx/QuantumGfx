#pragma once

#include "BaseVk.hpp"

#include "../IEngineFactory.hpp"
#include "../IRenderDevice.hpp"

#include "../../Common/ArrayProxy.hpp"

namespace Qgfx
{
	class HardwareQueueVk;

	/**
	 * @brief Struct describing how to create a new InstanceVk.
	*/
	struct EngineFactoryCreateInfoVk
	{
		/**
		 * @brief Optional pointer to the function that loads all vulkan related function pointers.
		*/
		PFN_vkGetInstanceProcAddr pfnLoaderHandle = nullptr;

		/**
		 * @brief Setting this to true will cause the instance to try to find and loader vulkan layers
		*/
		bool bEnableValidation = false;

		const char* AppName = "Qgfx";
		const uint32_t AppVersion = VK_MAKE_VERSION(1, 0, 0);
		const char* EngineName = "Qgfx";
		const uint32_t EngineVersion = VK_MAKE_VERSION(1, 0, 0);
	};

	/**
	 * @brief Info used to create hardware-level queue.
	*/
	struct HardwareQueueInfoVk
	{
		CommandQueueType QueueType = CommandQueueType::eGeneral;
		float Priority = 0.0f;
	};

	/**
	 * @brief Vulkan backend specific settings used to create render device.
	*/
	struct RenderDeviceCreateInfoVk : public RenderDeviceCreateInfo
	{
		vk::PhysicalDevice PhysicalDeviceVk = {};
	};

	/**
	 * @brief A factory for Vulkan specific initialization.
	*/
	class EngineFactoryVk final : public IEngineFactory
	{
	public:

		EngineFactoryVk(IRefCounter* pRefCounter, const EngineFactoryCreateInfoVk& CreateInfo);
		
		virtual ~EngineFactoryVk();

		inline virtual const APIInfo& GetAPIInfo() const override { return m_APIInfo; }

		inline virtual GraphicsInstanceType GetType() const override { return GraphicsInstanceType::eVulkan; }

		void CreateRenderDevice(const RenderDeviceCreateInfoVk& DeviceCreateInfo, const ArrayProxy<HardwareQueueInfoVk>& RequestedExtraHardwareQueues, IRenderDevice** ppDevice);

		void CreateCommandQueue(IRenderDevice* pDevice, HardwareQueueVk* pHardwareQueue, ICommandQueue** ppCommandQueue);

		/**
		 * @brief Gets a handle to the vkq::Instance object from the quantumvk library, used to interface with the native Vulkan API
		 * @return A handle to the internal vkq::Instance object
		*/
		inline vk::Instance GetVkInstance() const { return m_VkInstance; }

		inline const vk::DispatchLoaderDynamic& GetVkDispatch() const { return m_VkDispatch; }

		 
	private:

		vk::DispatchLoaderDynamic m_VkDispatch;
		vk::Instance m_VkInstance;

		APIInfo m_APIInfo = {};
	};

	void CreateEngineFactoryVk(const EngineFactoryCreateInfoVk& CreateInfo, EngineFactoryVk** ppEngineFactory);

}