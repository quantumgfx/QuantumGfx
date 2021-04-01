#pragma once

#include "BaseVk.hpp"

#include "../IInstance.hpp"
#include "../IRenderDevice.hpp"

namespace Qgfx
{
	/**
	 * @brief Struct describing how to create a new InstanceVk.
	*/
	struct InstanceVkCreateInfo
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


	class InstanceVk final : public IInstance
	{
	public:

		InstanceVk(const InstanceVkCreateInfo& CreateInfo);

		InstanceVk(vkq::Loader Loader, vkq::Instance Instance)
			: m_Loader(Loader), m_Instance(Instance)
		{
		}
		
		virtual ~InstanceVk();

		inline virtual const APIInfo& GetAPIInfo() const override { return m_APIInfo; }

		inline virtual GraphicsInstanceType GetType() const override { return GraphicsInstanceType::Vulkan; }

		const std::vector<vkq::PhysicalDevice> EnumeratePhysicalDevices();

		IRenderDevice* CreateRenderDevice();

		void DestroyRenderDevice(IRenderDevice* pRenderDevice);

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

}