#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"

namespace Qgfx
{
	RenderDeviceVk::RenderDeviceVk(RefCounter* pRefCounter, InstanceVk* Instance)
		: IRenderDevice(pRefCounter)
	{
		m_spInstance = Instance;

		/**
		 * @brief This will be set to something later
		*/
		m_PhysicalDevice = {};

		vk::PhysicalDeviceFeatures2 SupportedFeatures2{};
		vk::PhysicalDeviceVulkan11Features Supported11Features{};
		vk::PhysicalDeviceVulkan12Features Supported12Features{};
		SupportedFeatures2.pNext = &Supported11Features;
		Supported11Features.pNext = &Supported12Features;
		
		m_PhysicalDevice.getFeatures2(&SupportedFeatures2);

		vk::PhysicalDeviceFeatures2 EnabledFeatures2{};
		vk::PhysicalDeviceVulkan11Features Enabled11Features{};
		vk::PhysicalDeviceVulkan12Features Enabled12Features{};
		EnabledFeatures2.pNext = &Enabled11Features;
		Enabled11Features.pNext = &Enabled12Features;

		vk::DeviceCreateInfo CreateInfo{};
		CreateInfo.pNext = &EnabledFeatures2;
		

	}

	RenderDeviceVk::~RenderDeviceVk()
	{
	}
}