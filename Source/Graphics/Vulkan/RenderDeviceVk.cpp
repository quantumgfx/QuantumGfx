#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"

namespace Qgfx
{
	RenderDeviceVk::RenderDeviceVk(InstanceVk Instance)
	{
		/**
		 * @brief This will be set to something later
		*/
		m_PhysicalDevice = {};

		vk::PhysicalDeviceFeatures2 SupportedFeatures2{};
		vk::PhysicalDeviceTimelineSemaphoreFeatures SupportedTimelineSemaphoreFeatures{};
		SupportedFeatures2.pNext = &SupportedTimelineSemaphoreFeatures;
		
		m_PhysicalDevice.getFeatures2(&SupportedFeatures2);



		vk::PhysicalDeviceFeatures2 EnabledFeatures2{};

		vk::DeviceCreateInfo CreateInfo{};
		CreateInfo.pNext = &EnabledFeatures2;
		

	}

	RenderDeviceVk::~RenderDeviceVk()
	{

	}
}