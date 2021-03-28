#pragma once

#if !QGFX_VULKAN_SUPPORTED
#error QGFX_VULKAN_SUPPORTED must be defined to include IVkInstance.hpp
#endif

#include "../IInstance.hpp"

#include <quantumvk/quantumvk.hpp>

namespace Qgfx
{
	struct InstanceVkCreateInfo
	{
		PFN_vkGetInstanceProcAddr LoaderHandle = nullptr;

		const char* AppName = "Qgfx";
		const uint32_t AppVersion = VK_MAKE_VERSION(1, 0, 0);
		const char* EngineName = "Qgfx";
		const uint32_t EngineVersion = VK_MAKE_VERSION(1, 0, 0);

		bool EnableValidation = false;
	};


	class InstanceVk
	{
	public:

		InstanceVk(const InstanceVkCreateInfo& CreateInfo);

		InstanceVk(vkq::Loader Loader, vkq::Instance Instance)
			: m_Loader(Loader), m_Instance(Instance)
		{
		}
		
		virtual ~InstanceVk();

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
	};

}