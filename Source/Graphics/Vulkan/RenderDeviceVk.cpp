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

	RenderDeviceVk::RenderDeviceVk(RefCounter* pRefCounter, EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo)
		: IRenderDevice(pRefCounter)
	{
		m_spEngineFactory = pEngineFactory;
		m_PhysicalDevice = CreateInfo.PhysicalDeviceVk;

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

		m_PhysicalDevice.getFeatures2(&SupportedFeatures2);

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

		//////////////////////////
		// Queues ////////////////
		//////////////////////////

		std::vector<vk::QueueFamilyProperties> QueueFamiles = m_PhysicalDevice.getQueueFamilyProperties();

		std::vector<uint32_t> UniversalQueueFamilies{};
		std::vector<uint32_t> ComputeQueueFamilies{};
		std::vector<uint32_t> TransferQueueFamilies{};

		struct QueueFamilyDesc
		{
			uint32_t QueueCount;
			uint32_t CurrentIndex;
		};

		// Keeps track of the current index within each family
		std::vector<uint32_t> CurrentQueueIndex{};
		
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
				if(bSupportsPresentation)
					UniversalQueueFamilies.push_back(QueueFamilyIndex);
			}
			else if (QueueFamily.queueFlags & vk::QueueFlagBits::eCompute)
			{ // Supports Compute and Transfer
				ComputeQueueFamilies.push_back(QueueFamilyIndex);
			}
			else if (QueueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
			{ // Supports Transfer

				TransferQueueFamilies.push_back(QueueFamilyIndex);
			}

			CurrentQueueIndex.push_back(0);
		}

		if (UniversalQueueFamilies.size() >= 1)
		{
			QGFX_LOG_ERROR_AND_THROW("Device doesn't support graphics queue family with present capibilities");
		}

		struct PerQueueFamily
		{
			vk::DeviceQueueCreateInfo CreateInfo;
			std::vector<float> Priorities;
		};

		std::vector<PerQueueFamily> PerQueueFamilyInfo;

		for (uint32_t Index; Index < CreateInfo.NumRequestedQueues; Index++)
		{
			const DeviceQueueInfoVk& QueueInfo = CreateInfo.pRequestedQueues[Index];

			uint32_t QueueFamily = UINT32_MAX;
			uint32_t QueueIndex = UINT32_MAX;

			//////////////////////
			// Finds best queue //
			//////////////////////

			if (QueueFamily == UINT32_MAX && QueueInfo.Type == RenderContextType::AsyncTransfer)
			{
				for (uint32_t QueueFamilyIndex : TransferQueueFamilies)
				{
					if (CurrentQueueIndex[QueueFamilyIndex] < QueueFamiles[QueueFamilyIndex].queueCount)
					{
						QueueFamily = QueueFamilyIndex;
						QueueIndex = CurrentQueueIndex[QueueFamilyIndex];
						CurrentQueueIndex[QueueFamilyIndex] += 1;
					}
				}
			}

			if (QueueFamily == UINT32_MAX && (QueueInfo.Type == RenderContextType::AsyncTransfer ||
											 QueueInfo.Type == RenderContextType::AsyncCompute))
			{
				for (uint32_t QueueFamilyIndex : ComputeQueueFamilies)
				{
					if (CurrentQueueIndex[QueueFamilyIndex] < QueueFamiles[QueueFamilyIndex].queueCount)
					{
						QueueFamily = QueueFamilyIndex;
						QueueIndex = CurrentQueueIndex[QueueFamilyIndex];
						CurrentQueueIndex[QueueFamilyIndex] += 1;
					}
				}
			}

			if (QueueFamily == UINT32_MAX && (QueueInfo.Type == RenderContextType::AsyncTransfer ||
											 QueueInfo.Type == RenderContextType::AsyncCompute || 
											 QueueInfo.Type == RenderContextType::Universal))
			{
				for (uint32_t QueueFamilyIndex : UniversalQueueFamilies)
				{
					if (CurrentQueueIndex[QueueFamilyIndex] < QueueFamiles[QueueFamilyIndex].queueCount)
					{
						QueueFamily = QueueFamilyIndex;
						QueueIndex = CurrentQueueIndex[QueueFamilyIndex];
						CurrentQueueIndex[QueueFamilyIndex] += 1;
					}
				}
			}

			///////////////////////
			///////////////////////
			///////////////////////

			if (QueueFamily != UINT32_MAX)
			{
				while (QueueFamily >= PerQueueFamilyInfo.size())
				{
					vk::DeviceQueueCreateInfo QueueCreateInfo{};
					QueueCreateInfo.pNext = nullptr;
					QueueCreateInfo.flags = {};
					QueueCreateInfo.pQueuePriorities = nullptr;
					QueueCreateInfo.queueFamilyIndex = UINT32_MAX;
					QueueCreateInfo.queueCount = 0;

					PerQueueFamily PerFamily;
					PerFamily.CreateInfo = QueueCreateInfo;
					PerFamily.Priorities = {};

					PerQueueFamilyInfo.push_back(PerFamily);
				}

				while (QueueIndex >= PerQueueFamilyInfo[QueueFamily].Priorities.size())
				{
					PerQueueFamilyInfo[QueueFamily].Priorities.push_back(0.0f);
				}

				PerQueueFamilyInfo[QueueFamily].Priorities[QueueIndex] = QueueInfo.Priority;
				PerQueueFamilyInfo[QueueFamily].CreateInfo.pQueuePriorities = PerQueueFamilyInfo[QueueFamily].Priorities.data();
				PerQueueFamilyInfo[QueueFamily].CreateInfo.queueFamilyIndex = QueueFamily;
				PerQueueFamilyInfo[QueueFamily].CreateInfo.queueCount = QueueIndex;

				QueueDesc Queue;
				Queue.FamilyIndex = QueueFamily;
				Queue.Index = QueueIndex;
				Queue.Info = QueueInfo;
				// Queue::Handle is filled after creating the device
				m_Queues.push_back(Queue);
			}
			else
			{
				// No remaining queues can support a universal render type.
				break;
			}

			m_NumSupportedQueues++;
		}

		////////////////////////
		// Device Create Info //
		////////////////////////

		std::vector<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfos{};

		for (const PerQueueFamily& Info : PerQueueFamilyInfo)
		{
			if (Info.CreateInfo.queueCount > 0)
			{
				DeviceQueueCreateInfos.push_back(Info.CreateInfo);
			}
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

		// Set queue handles
		for (QueueDesc& Queue : m_Queues)
		{
			Queue.Handle = vkq::Queue::create(m_LogicalDevice, Queue.FamilyIndex, Queue.Index);
		}
	}

	RenderDeviceVk::~RenderDeviceVk()
	{
		m_LogicalDevice.destroy();
		m_PhysicalDevice.reset();
	}

	void RenderDeviceVk::QueueWaitIdle(uint32_t QueueIndex)
	{
		QGFX_VERIFY(QueueIndex < m_NumSupportedQueues, "Queue Index not supported by physical device. Make sure QueueIndex < RenderDeviceVk::GetNumSupportedQueues().");
		m_Queues[QueueIndex].Handle.waitIdle();
	}

	void RenderDeviceVk::QueueSubmit(uint32_t QueueIndex, const vk::SubmitInfo& SubmitInfo, vk::Fence Fence)
	{
		QGFX_VERIFY(QueueIndex < m_NumSupportedQueues, "Queue Index not supported by physical device. Make sure QueueIndex < RenderDeviceVk::GetNumSupportedQueues().");
		m_Queues[QueueIndex].Handle.submit(SubmitInfo, Fence);
	}

	void RenderDeviceVk::QueuePresent(uint32_t QueueIndex, const vk::PresentInfoKHR& PresentInfo)
	{
		QGFX_VERIFY(QueueIndex < m_NumSupportedQueues, "Queue Index not supported by physical device. Make sure QueueIndex < RenderDeviceVk::GetNumSupportedQueues().");
		m_Queues[QueueIndex].Handle.presentKHR(PresentInfo);
	}

	vkq::Queue RenderDeviceVk::GetVkqQueue(uint32_t QueueIndex)
	{
		QGFX_VERIFY(QueueIndex < m_NumSupportedQueues, "Queue Index not supported by physical device. Make sure QueueIndex < RenderDeviceVk::GetNumSupportedQueues().");
		return m_Queues[QueueIndex].Handle;
	}

	RenderContextType RenderDeviceVk::GetQueueType(uint32_t QueueIndex)
	{
		QGFX_VERIFY(QueueIndex < m_NumSupportedQueues, "Queue Index not supported by physical device. Make sure QueueIndex < RenderDeviceVk::GetNumSupportedQueues().");
		return m_Queues[QueueIndex].Info.Type;
	}

	void RenderDeviceVk::WaitIdle()
	{
		m_LogicalDevice.waitIdle();
	}
}