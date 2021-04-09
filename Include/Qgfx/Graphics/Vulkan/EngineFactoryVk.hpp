#pragma once

#include "BaseVk.hpp"

#include "../IEngineFactory.hpp"
#include "../IRenderDevice.hpp"

namespace Qgfx
{
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

	struct DeviceQueueInfoVk
	{
		RenderContextType Type = RenderContextType::Universal;
		float Priority = 0.0f;
	};

	struct RenderDeviceCreateInfoVk : public RenderDeviceCreateInfo
	{
		vkq::PhysicalDevice PhysicalDeviceVk = {};

		uint32_t NumRequestedQueues = 0;
		DeviceQueueInfoVk* pRequestedQueues = nullptr;
	};

	/**
	 * @brief A factory for Vulkan specific initialization.
	*/
	class EngineFactoryVk final : public IEngineFactory
	{
	public:

		EngineFactoryVk(RefCounter* pRefCounter, const EngineFactoryCreateInfoVk& CreateInfo);
		
		virtual ~EngineFactoryVk();

		inline virtual const APIInfo& GetAPIInfo() const override { return m_APIInfo; }

		inline virtual GraphicsInstanceType GetType() const override { return GraphicsInstanceType::Vulkan; }

		void CreateRenderDevice(const RenderDeviceCreateInfoVk& DeviceCreateInfo, uint32_t* pNumSupportedQueues, IRenderDevice** ppDevice);

		void CreateRenderContext(IRenderDevice* pDevice, uint32_t QueueIndex, IRenderContext** ppContext);

		/**
		 * @brief Gets a handle to the vkq::Loader object from the quantumvk library, used to interface with the native Vulkan API
		 * @return A handle to the internal vkq::Loader object
		*/
		inline vkq::Loader GetVkqLoader() { return m_Loader; }

		/**
		 * @brief Gets a handle to the vkq::Instance object from the quantumvk library, used to interface with the native Vulkan API
		 * @return A handle to the internal vkq::Instance object
		*/
		inline vkq::Instance GetVkqInstance() { return m_Instance; }

		 
	private:

		vkq::Loader m_Loader;
		vkq::Instance m_Instance;

		APIInfo m_APIInfo = {};
	};

	void CreateEngineFactoryVk(const EngineFactoryCreateInfoVk& CreateInfo, EngineFactoryVk** ppEngineFactory);

}