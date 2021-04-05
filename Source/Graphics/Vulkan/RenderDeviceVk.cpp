#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"

namespace Qgfx
{

	static inline DeviceFeatureState GetFeatureState(DeviceFeatureState RequestedState, vk::Bool32 IsFeatureSupported, vk::Bool32& EnableFeature, const char* FeatureName)
	{
		switch (RequestedState)
		{
		case DeviceFeatureState::Disabled:
		{
			EnableFeature = VK_FALSE;
			return DeviceFeatureState::Disabled;
		}
		case DeviceFeatureState::Enabled:
		{
			EnableFeature = IsFeatureSupported;
			if (IsFeatureSupported)
				return DeviceFeatureState::Enabled;
			else
				QGFX_LOG_ERROR_AND_THROW(FeatureName, " not supported by this device");
		}

		case DeviceFeatureState::Optional:
		{
			EnableFeature = IsFeatureSupported;
			return IsFeatureSupported ? DeviceFeatureState::Enabled : DeviceFeatureState::Disabled;
		}
		default:
			QGFX_UNEXPECTED("Unexpected feature state");
			EnableFeature = VK_FALSE;
			return DeviceFeatureState::Disabled;
		}
	}

	RenderDeviceVk::RenderDeviceVk(RefCounter* pRefCounter, InstanceVk* Instance, const RenderDeviceCreateInfo& CreateInfo, vkq::PhysicalDevice VkPhDev)
		: IRenderDevice(pRefCounter)
	{
		m_spInstance = Instance;
		m_PhysicalDevice = VkPhDev;

		// Features

		auto& RequestedFeatures = CreateInfo.Features;

		m_DeviceFeatures.IndirectRendering = DeviceFeatureState::Enabled;
		m_DeviceFeatures.ComputeShaders = DeviceFeatureState::Enabled;

		vk::PhysicalDeviceFeatures2 SupportedFeatures2{};
		vk::PhysicalDeviceFeatures& Supported10Features = SupportedFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Supported11Features{};
		vk::PhysicalDeviceVulkan12Features Supported12Features{};
		SupportedFeatures2.pNext = &Supported11Features;
		Supported11Features.pNext = &Supported12Features;

		m_PhysicalDevice.vkHandle().getFeatures2(&SupportedFeatures2, m_PhysicalDevice.dispatch());

		vk::PhysicalDeviceFeatures2 EnabledFeatures2{};
		vk::PhysicalDeviceFeatures& Enabled10Features = EnabledFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Enabled11Features{};
		vk::PhysicalDeviceVulkan12Features Enabled12Features{};
		EnabledFeatures2.pNext = &Enabled11Features;
		Enabled11Features.pNext = &Enabled12Features;

		m_DeviceFeatures.GeometryShaders = GetFeatureState(RequestedFeatures.GeometryShaders, Supported10Features.geometryShader, Enabled10Features.geometryShader, "Geometry shaders are");
		m_DeviceFeatures.TesselationShaders = GetFeatureState(RequestedFeatures.TesselationShaders, Supported10Features.tessellationShader, Enabled10Features.tessellationShader, "Tesselation is");
		m_DeviceFeatures.WireFrameFill = GetFeatureState(RequestedFeatures.WireFrameFill, Supported10Features.fillModeNonSolid, Enabled10Features.fillModeNonSolid, "Wireframe fill is");

		// Extensions
		std::vector<const char*> EnabledExtensions{};
		EnabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		// Queues
		std::vector<vk::QueueFamilyProperties> QueueFamiles = m_PhysicalDevice.getQueueFamilyProperties();
		
		for (size_t QueueFamilyIndex = 0; QueueFamilyIndex < QueueFamiles.size(); QueueFamilyIndex++)
		{
			const vk::QueueFamilyProperties& QueueFamily = QueueFamiles[QueueFamilyIndex];

			bool bSupportsPresentation = false; // Whether or not the queue supports presentation

#ifdef QGFX_PLATFORM_WIN32
			bSupportsPresentation = m_PhysicalDevice.vkHandle().getWin32PresentationSupportKHR(QueueFamilyIndex, m_PhysicalDevice.dispatch());
#else
			bSupportsPresentation = static_cast<bool>(QueueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
#endif
			if (QueueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			{ // Supports Graphics, Compute, and Transfer
				if (GraphicsQueueFamily == UINT32_MAX)
				{
					if (bSupportsPresentation)
					{
						GraphicsQueueFamily = QueueFamilyIndex;
						GraphicsNumQueues = 1;
					}
				}
			}
			else if (QueueFamily.queueFlags & vk::QueueFlagBits::eCompute)
			{ // Supports Compute and Transfer
				if (ComputeQueueFamily == UINT32_MAX)
				{
					ComputeQueueFamily = QueueFamilyIndex;
					ComputeNumQueues = 1;
				}
			}
			else if (QueueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
			{ // Supports Transfer
				if (TransferQueueFamily == UINT32_MAX)
				{
					TransferQueueFamily = QueueFamilyIndex;
					TransferNumQueues = 1;
				}
			}
		}

		if (GraphicsQueueFamily == UINT32_MAX)
		{
			QGFX_LOG_ERROR_AND_THROW("Device doesn't support graphics queue family with present capibilities");
		}

		std::vector<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfos{};

		float pPriorities[] = { 1.0f };

		vk::DeviceQueueCreateInfo GraphicsQueueFamilyCreateInfo{};
		GraphicsQueueFamilyCreateInfo.pNext = nullptr;
		GraphicsQueueFamilyCreateInfo.flags = {};
		GraphicsQueueFamilyCreateInfo.pQueuePriorities = pPriorities;
		GraphicsQueueFamilyCreateInfo.queueFamilyIndex = GraphicsQueueFamily;
		GraphicsQueueFamilyCreateInfo.queueCount = GraphicsNumQueues;

		DeviceQueueCreateInfos.push_back(GraphicsQueueFamilyCreateInfo);

		if (ComputeQueueFamily != UINT32_MAX)
		{
			vk::DeviceQueueCreateInfo ComputeQueueFamilyCreateInfo{};
			ComputeQueueFamilyCreateInfo.pNext = nullptr;
			ComputeQueueFamilyCreateInfo.flags = {};
			ComputeQueueFamilyCreateInfo.pQueuePriorities = pPriorities;
			ComputeQueueFamilyCreateInfo.queueFamilyIndex = ComputeQueueFamily;
			ComputeQueueFamilyCreateInfo.queueCount = ComputeNumQueues;

			DeviceQueueCreateInfos.push_back(ComputeQueueFamilyCreateInfo);
		}

		if (TransferQueueFamily != UINT32_MAX)
		{
			vk::DeviceQueueCreateInfo TransferQueueFamilyCreateInfo{};
			TransferQueueFamilyCreateInfo.pNext = nullptr;
			TransferQueueFamilyCreateInfo.flags = {};
			TransferQueueFamilyCreateInfo.pQueuePriorities = pPriorities;
			TransferQueueFamilyCreateInfo.queueFamilyIndex = TransferQueueFamily;
			TransferQueueFamilyCreateInfo.queueCount = TransferNumQueues;

			DeviceQueueCreateInfos.push_back(TransferQueueFamilyCreateInfo);
		}

		vk::DeviceCreateInfo DeviceCreateInfo{};
		DeviceCreateInfo.pNext = &EnabledFeatures2;
		DeviceCreateInfo.flags = {}; /*Reserved for future use*/
		DeviceCreateInfo.enabledLayerCount = 0; /*Deprecated */
		DeviceCreateInfo.ppEnabledLayerNames = nullptr; /*Deprecated */
		DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(EnabledExtensions.size());
		DeviceCreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
		DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(DeviceQueueCreateInfos.size());
		DeviceCreateInfo.pQueueCreateInfos = DeviceQueueCreateInfos.data();
		DeviceCreateInfo.pEnabledFeatures = nullptr; /*Set via vk::PhysicalDeviceFeatures2 in the pNext chain*/

		try
		{
			m_LogicalDevice = vkq::Device::create(m_PhysicalDevice, DeviceCreateInfo);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vkCreateDevice failed with error: ", Error.what());
		}

		GraphicsQueues.resize(GraphicsNumQueues);
		for (uint32_t Index = 0; Index < GraphicsNumQueues; Index++)
		{
			GraphicsQueues[Index] = vkq::Queue::create(m_LogicalDevice, GraphicsQueueFamily, Index);
		}

		if (ComputeQueueFamily != UINT32_MAX)
		{
			ComputeQueues.resize(ComputeNumQueues);
			for (uint32_t Index = 0; Index < ComputeNumQueues; Index++)
			{
				ComputeQueues[Index] = vkq::Queue::create(m_LogicalDevice, ComputeQueueFamily, Index);
			}
		}

		if (TransferQueueFamily != UINT32_MAX)
		{
			TransferQueues.resize(TransferNumQueues);
			for (uint32_t Index = 0; Index < TransferNumQueues; Index++)
			{
				TransferQueues[Index] = vkq::Queue::create(m_LogicalDevice, TransferQueueFamily, Index);
			}
		}
	}

	RenderDeviceVk::~RenderDeviceVk()
	{
		m_LogicalDevice.destroy();
		m_PhysicalDevice.reset();
	}

	void RenderDeviceVk::WaitIdle()
	{
		m_LogicalDevice.waitIdle();
	}
}