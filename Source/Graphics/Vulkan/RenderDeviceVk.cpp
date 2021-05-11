#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"
#include "Qgfx/Graphics/Vulkan/FenceVk.hpp"
#include "Qgfx/Graphics/Vulkan/HardwareQueueVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"
#include "Qgfx/Graphics/Vulkan/BufferVk.hpp"
#include "Qgfx/Graphics/Vulkan/ShaderModuleVk.hpp"
#include "Qgfx/Graphics/Vulkan/SwapChainVk.hpp"
#include "Qgfx/Graphics/Vulkan/TextureVk.hpp"

namespace Qgfx
{

	static inline RenderDeviceFeatureState GetFeatureState(RenderDeviceFeatureState RequestedState, vk::Bool32 IsFeatureSupported, vk::Bool32& EnableFeature, const char* FeatureName)
	{
		switch (RequestedState)
		{
		case RenderDeviceFeatureState::eDisabled:
		{
			EnableFeature = VK_FALSE;
			return RenderDeviceFeatureState::eDisabled;
		}
		case RenderDeviceFeatureState::eEnabled:
		{
			EnableFeature = IsFeatureSupported;
			if (IsFeatureSupported)
				return RenderDeviceFeatureState::eEnabled;
			else
				QGFX_LOG_ERROR_AND_THROW(FeatureName, " not supported by this device");
		}

		case RenderDeviceFeatureState::eOptional:
		{
			EnableFeature = IsFeatureSupported;
			return IsFeatureSupported ? RenderDeviceFeatureState::eEnabled : RenderDeviceFeatureState::eDisabled;
		}
		default:
			QGFX_UNEXPECTED("Unexpected feature state");
			EnableFeature = VK_FALSE;
			return RenderDeviceFeatureState::eDisabled;
		}
	}

	RenderDeviceVk::RenderDeviceVk(EngineFactoryVk* pEngineFactory, const RenderDeviceCreateInfoVk& CreateInfo, IMemoryAllocator& RawMemAllocator)
		: IRenderDevice(pEngineFactory), 
		m_TextureObjAllocator(RawMemAllocator, sizeof(TextureVk), 128)
	{
		m_spEngineFactory =  pEngineFactory;
		m_VkDispatch = pEngineFactory->GetVkDispatch();
		m_VkPhysicalDevice = CreateInfo.PhysicalDeviceVk;

		// Features

		auto& RequestedFeatures = CreateInfo.Features;

		m_Features.IndirectRendering = RenderDeviceFeatureState::eEnabled;
		m_Features.ComputeShaders = RenderDeviceFeatureState::eEnabled;

		vk::PhysicalDeviceFeatures2 SupportedFeatures2{};
		vk::PhysicalDeviceFeatures& Supported10Features = SupportedFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Supported11Features{};
		vk::PhysicalDeviceVulkan12Features Supported12Features{};
		SupportedFeatures2.pNext = &Supported11Features;
		Supported11Features.pNext = &Supported12Features;

		m_VkPhysicalDevice.getFeatures2(&SupportedFeatures2, m_VkDispatch);

		vk::PhysicalDeviceFeatures2 EnabledFeatures2{};
		vk::PhysicalDeviceFeatures& Enabled10Features = EnabledFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Enabled11Features{};
		vk::PhysicalDeviceVulkan12Features Enabled12Features{};
		EnabledFeatures2.pNext = &Enabled11Features;
		Enabled11Features.pNext = &Enabled12Features;

		m_Features.GeometryShaders = GetFeatureState(RequestedFeatures.GeometryShaders, Supported10Features.geometryShader, Enabled10Features.geometryShader, "Geometry shaders are");
		m_Features.TesselationShaders = GetFeatureState(RequestedFeatures.TesselationShaders, Supported10Features.tessellationShader, Enabled10Features.tessellationShader, "Tesselation is");
		m_Features.PolygonModeLine = GetFeatureState(RequestedFeatures.PolygonModeLine, Supported10Features.fillModeNonSolid, Enabled10Features.fillModeNonSolid, "Line polygon mode is");
		m_Features.PolygonModePoint = GetFeatureState(RequestedFeatures.PolygonModePoint, Supported10Features.fillModeNonSolid, Enabled10Features.fillModeNonSolid, "Point polygon mode is");
		

		if (Supported12Features.timelineSemaphore)
		{
			Enabled12Features.timelineSemaphore = true;
			//m_bTimelineSemaphoresSupported = true;
		}

		// Extensions
		std::vector<vk::ExtensionProperties> SupportedExtensions = m_VkPhysicalDevice.enumerateDeviceExtensionProperties(nullptr, m_VkDispatch);

		bool bMemoryBudgetExtEnabled = false;

		std::vector<const char*> EnabledExtensions{};
		EnabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		for (auto& Extension : SupportedExtensions)
		{
			if (std::strcmp(Extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
			{
				EnabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
				bMemoryBudgetExtEnabled = true;
			}
		}

		//////////////////////////
		// Queues ////////////////
		//////////////////////////

		std::vector<vk::QueueFamilyProperties> QueueFamiles = m_VkPhysicalDevice.getQueueFamilyProperties(m_VkDispatch);

		struct QueueFamily
		{
			uint32_t Index;
			uint32_t NumQueues;
			uint32_t NumUsedQueues;

			QueueFamily(uint32_t Index, uint32_t NumQueues)
				: Index(Index), NumQueues(NumQueues), NumUsedQueues(0)
			{
			}
		};

		std::vector<QueueFamily> GeneralQueueFamilies{};
		std::vector<QueueFamily> ComputeQueueFamilies{};
		std::vector<QueueFamily> TransferQueueFamilies{};

		struct QueueFamilyDesc
		{
			uint32_t QueueCount;
			uint32_t CurrentIndex;
		};
		
		for (size_t QueueFamilyIndex = 0; QueueFamilyIndex < QueueFamiles.size(); QueueFamilyIndex++)
		{
			const vk::QueueFamilyProperties& QueueFamilyProps = QueueFamiles[QueueFamilyIndex];

			bool bSupportsPresentation = false; // Whether or not the queue supports presentation

#ifdef VK_USE_PLATFORM_WIN32_KHR
			bSupportsPresentation = m_VkPhysicalDevice.getWin32PresentationSupportKHR(QueueFamilyIndex, m_VkDispatch);
#else
			bSupportsPresentation = static_cast<bool>(QueueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
#endif
			if (QueueFamilyProps.queueFlags & vk::QueueFlagBits::eGraphics)
			{ // Supports Graphics, Compute, and Transfer
				if(bSupportsPresentation)
					GeneralQueueFamilies.push_back(QueueFamily{ QueueFamilyIndex , QueueFamilyProps.queueCount });
			}
			else if (QueueFamilyProps.queueFlags & vk::QueueFlagBits::eCompute)
			{ // Supports Compute and Transfer
				ComputeQueueFamilies.push_back(QueueFamily{ QueueFamilyIndex , QueueFamilyProps.queueCount });
			}
			else if (QueueFamilyProps.queueFlags & vk::QueueFlagBits::eTransfer)
			{ // Supports Transfer

				TransferQueueFamilies.push_back(QueueFamily{ QueueFamilyIndex , QueueFamilyProps.queueCount });
			}
		}

		if (GeneralQueueFamilies.size() < 1)
		{
			QGFX_LOG_ERROR_AND_THROW("Device doesn't support a graphics queue family with present capibilities");
		}

		QueueFamily& DefaultQueueFamily = GeneralQueueFamilies.front();
		QGFX_VERIFY_EXPR(DefaultQueueFamily.NumQueues >= 1);
		DefaultQueueFamily.NumUsedQueues += 1;

		HardwareQueueVkCreateInfo DefaultHardwareQueueCI{};
		DefaultHardwareQueueCI.QueueFamilyIndex = DefaultQueueFamily.Index;
		DefaultHardwareQueueCI.QueueIndex = 0;
		DefaultHardwareQueueCI.Info.Priority = 1.0f;
		DefaultHardwareQueueCI.Info.QueueType = CommandQueueType::eGeneral;

		std::vector<HardwareQueueVkCreateInfo> ExtraHardwareQueueCreateInfos{};

		ArrayProxy<HardwareQueueInfoVk> RequestedExtraHardwareQueues(CreateInfo.NumRequestedExtraHardwareQueues, CreateInfo.pRequestedExtraHardwareQueues);

		for (const HardwareQueueInfoVk& HardwareQueueInfo : RequestedExtraHardwareQueues)
		{
			uint32_t QueueFamilyIndex = UINT32_MAX;
			uint32_t QueueIndex = UINT32_MAX;

			//////////////////////
			// Finds best queue //
			//////////////////////

			if (QueueFamilyIndex == UINT32_MAX && HardwareQueueInfo.QueueType == CommandQueueType::eTransfer)
			{
				for (QueueFamily& Family : TransferQueueFamilies)
				{
					if (Family.NumUsedQueues < Family.NumQueues)
					{
						QueueFamilyIndex = Family.Index;
						QueueIndex = Family.NumUsedQueues;
						Family.NumUsedQueues += 1;
						break;
					}
				}
			}

			if (QueueFamilyIndex == UINT32_MAX && (HardwareQueueInfo.QueueType == CommandQueueType::eTransfer ||
				HardwareQueueInfo.QueueType == CommandQueueType::eCompute))
			{
				for (QueueFamily& Family : ComputeQueueFamilies)
				{
					if (Family.NumUsedQueues < Family.NumQueues)
					{
						QueueFamilyIndex = Family.Index;
						QueueIndex = Family.NumUsedQueues;
						Family.NumUsedQueues += 1;
						break;
					}
				}
			}

			if (QueueFamilyIndex == UINT32_MAX && (HardwareQueueInfo.QueueType == CommandQueueType::eTransfer ||
				HardwareQueueInfo.QueueType == CommandQueueType::eCompute ||
				HardwareQueueInfo.QueueType == CommandQueueType::eGeneral))
			{
				for (QueueFamily& Family : GeneralQueueFamilies)
				{
					if (Family.NumUsedQueues < Family.NumQueues)
					{
						QueueFamilyIndex = Family.Index;
						QueueIndex = Family.NumUsedQueues;
						Family.NumUsedQueues += 1;
						break;
					}
				}
			}

			///////////////////////
			///////////////////////
			///////////////////////

			if (QueueFamilyIndex != UINT32_MAX)
			{
				HardwareQueueVkCreateInfo HardwareQueueCI{};
				HardwareQueueCI.QueueFamilyIndex = QueueFamilyIndex;
				HardwareQueueCI.QueueIndex = QueueIndex;
				HardwareQueueCI.Info.Priority = HardwareQueueInfo.Priority;
				HardwareQueueCI.Info.QueueType = HardwareQueueInfo.QueueType;

				ExtraHardwareQueueCreateInfos.push_back(HardwareQueueCI);
			}
			else
			{
				// No remaining queues can support a universal render type.
				break;
			}
		}

		struct QueueCreateInfoAndPriorities
		{
			vk::DeviceQueueCreateInfo CreateInfo;
			std::vector<float> Priorities;

			QueueCreateInfoAndPriorities()
			{
				CreateInfo.pNext = nullptr;
				CreateInfo.flags = {};
				CreateInfo.pQueuePriorities = nullptr;
				CreateInfo.queueFamilyIndex = UINT32_MAX;
				CreateInfo.queueCount = 0;

				Priorities = {};
			}
		};

		std::vector<QueueCreateInfoAndPriorities> QueueCreateInfosAndPriorities;

		std::vector<HardwareQueueVkCreateInfo> HardwareQueueCreateInfos(ExtraHardwareQueueCreateInfos);
		HardwareQueueCreateInfos.push_back(DefaultHardwareQueueCI);

		for (const auto& HardwareQueueCreateInfo : HardwareQueueCreateInfos)
		{
			auto QueueFamilyIndex = HardwareQueueCreateInfo.QueueFamilyIndex;
			auto QueueIndex = HardwareQueueCreateInfo.QueueIndex;

			while (QueueFamilyIndex >= QueueCreateInfosAndPriorities.size())
			{
				QueueCreateInfosAndPriorities.emplace_back();
			}

			auto& CreateInfoAndPriorities = QueueCreateInfosAndPriorities[HardwareQueueCreateInfo.QueueFamilyIndex];

			while (HardwareQueueCreateInfo.QueueIndex >= CreateInfoAndPriorities.Priorities.size())
			{
				CreateInfoAndPriorities.Priorities.push_back(0.0f);
			}

			CreateInfoAndPriorities.Priorities[QueueIndex] = HardwareQueueCreateInfo.Info.Priority;

			CreateInfoAndPriorities.CreateInfo.pQueuePriorities = CreateInfoAndPriorities.Priorities.data();
			CreateInfoAndPriorities.CreateInfo.queueFamilyIndex = QueueFamilyIndex;
			CreateInfoAndPriorities.CreateInfo.queueCount = QueueIndex;
		}

		////////////////////////
		// Device Create Info //
		////////////////////////

		std::vector<vk::DeviceQueueCreateInfo> DeviceQueueCreateInfos{};

		for (const QueueCreateInfoAndPriorities& Info : QueueCreateInfosAndPriorities)
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
			m_VkDevice = m_VkPhysicalDevice.createDevice(DeviceCreateInfo, nullptr, m_VkDispatch);
			m_VkDispatch.init(m_VkDevice);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vkCreateDevice failed with error: ", Error.what());
		}

		m_pDefaultHardwareQueue = new HardwareQueueVk(this, DefaultHardwareQueueCI);

		m_pDefaultCommandQueue = new CommandQueueVk(m_pEngineFactory, this, m_pDefaultHardwareQueue, true);

		for (const auto& ExtraQueueCreateInfo : ExtraHardwareQueueCreateInfos)
		{
			m_ExtraHardwareQueues.push_back(new HardwareQueueVk(this, ExtraQueueCreateInfo));
		}

		////////////////////////////
		// Memory Allocator ////////
		////////////////////////////

		VmaVulkanFunctions Funcs{};
		Funcs.vkAllocateMemory =   m_VkDispatch.vkAllocateMemory;
		Funcs.vkBindBufferMemory = m_VkDispatch.vkBindBufferMemory;
		Funcs.vkBindImageMemory =  m_VkDispatch.vkBindImageMemory;
		Funcs.vkCmdCopyBuffer =    m_VkDispatch.vkCmdCopyBuffer;
		Funcs.vkCreateBuffer =     m_VkDispatch.vkCreateBuffer;
		Funcs.vkCreateImage =      m_VkDispatch.vkCreateImage;
		Funcs.vkDestroyBuffer =    m_VkDispatch.vkDestroyBuffer;
		Funcs.vkDestroyImage =     m_VkDispatch.vkDestroyImage;
		Funcs.vkFlushMappedMemoryRanges =     m_VkDispatch.vkFlushMappedMemoryRanges;
		Funcs.vkFreeMemory =                  m_VkDispatch.vkFreeMemory;
		Funcs.vkGetBufferMemoryRequirements = m_VkDispatch.vkGetBufferMemoryRequirements;
		Funcs.vkGetImageMemoryRequirements =  m_VkDispatch.vkGetImageMemoryRequirements;
		Funcs.vkGetPhysicalDeviceMemoryProperties =     m_VkDispatch.vkGetPhysicalDeviceMemoryProperties;
		Funcs.vkInvalidateMappedMemoryRanges =          m_VkDispatch.vkInvalidateMappedMemoryRanges;
		Funcs.vkMapMemory =                             m_VkDispatch.vkMapMemory;
		Funcs.vkUnmapMemory =                           m_VkDispatch.vkUnmapMemory;
		Funcs.vkGetPhysicalDeviceMemoryProperties2KHR = m_VkDispatch.vkGetPhysicalDeviceMemoryProperties2;
		Funcs.vkBindBufferMemory2KHR =                  m_VkDispatch.vkBindBufferMemory2;
		Funcs.vkBindImageMemory2KHR =                   m_VkDispatch.vkBindImageMemory2;
		Funcs.vkGetBufferMemoryRequirements2KHR =       m_VkDispatch.vkGetBufferMemoryRequirements2;
		Funcs.vkGetImageMemoryRequirements2KHR =        m_VkDispatch.vkGetImageMemoryRequirements2;

		VmaAllocatorCreateFlags Flags = 0;

		if (bMemoryBudgetExtEnabled)
		{
			Flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		}

		VmaAllocatorCreateInfo AllocatorCI{};
		AllocatorCI.instance = pEngineFactory->GetVkInstance();
		AllocatorCI.physicalDevice = m_VkPhysicalDevice;
		AllocatorCI.device =         m_VkDevice;
		AllocatorCI.pVulkanFunctions = &Funcs;
		AllocatorCI.pRecordSettings = nullptr;
		AllocatorCI.pAllocationCallbacks = nullptr;
		AllocatorCI.pHeapSizeLimit = nullptr;
		AllocatorCI.frameInUseCount = 0;
		//AllocatorCI.preferredLargeHeapBlockSize = 0;
		AllocatorCI.flags = Flags;
		AllocatorCI.vulkanApiVersion = VK_MAKE_VERSION(1, 2, 0);

		vmaCreateAllocator(nullptr, &m_VmaAllocator);
	}

	RenderDeviceVk::~RenderDeviceVk()
	{
		m_VkDevice.waitIdle(m_VkDispatch);

		m_pDefaultCommandQueue->Release();

		vmaDestroyAllocator(m_VmaAllocator);

		for (HardwareQueueVk* pHardwareQueue : m_ExtraHardwareQueues)
		{
			delete pHardwareQueue;
		}

		delete m_pDefaultHardwareQueue;

		m_VkDevice.destroy();
		m_VkPhysicalDevice = nullptr;
	}

	vk::Semaphore RenderDeviceVk::CreateVkBinarySemaphore() const
	{
		vk::SemaphoreCreateInfo SemaphoreCI{};
		SemaphoreCI.pNext = nullptr;
		SemaphoreCI.flags = {};

		return m_VkDevice.createSemaphore(SemaphoreCI, nullptr, m_VkDispatch);
	}

	void RenderDeviceVk::DestroyVkSemaphore(vk::Semaphore Sem) const
	{
		m_VkDevice.destroySemaphore(Sem, nullptr, m_VkDispatch);
	}

	std::pair<vk::Image, VmaAllocation> RenderDeviceVk::CreateVkTexture(const vk::ImageCreateInfo& ImageCI, const VmaAllocationCreateInfo& AllocCI)
	{
		VkImage Image;
		VmaAllocation Alloc;

		vmaCreateImage(m_VmaAllocator, &static_cast<const VkImageCreateInfo&>(ImageCI), &AllocCI, &Image, &Alloc, nullptr);

		return std::make_pair(vk::Image(Image), Alloc);
	}

	void RenderDeviceVk::DestroyVkTexture(vk::Image Image, VmaAllocation Allocation) const
	{
		vmaDestroyImage(m_VmaAllocator, static_cast<VkImage>(Image), Allocation);
	}

	void RenderDeviceVk::WaitIdle()
	{
		m_VkDevice.waitIdle(m_VkDispatch);
	}

	IBuffer* RenderDeviceVk::CreateBuffer(const BufferCreateInfo& CreateInfo)
	{
	}

	ISwapChain* RenderDeviceVk::CreateSwapChain(const SwapChainCreateInfo& CreateInfo)
	{
		return new SwapChainVk(this, CreateInfo);
	}

	void RenderDeviceVk::DestroySwapChain(SwapChainVk* pSwapChain)
	{
		delete pSwapChain;
	}

	ITexture* RenderDeviceVk::CreateTexture(const TextureCreateInfo& CreateInfo)
	{
	}

	ITexture* RenderDeviceVk::CreateTextureFromVkImage(const TextureCreateInfo& CreateInfo, vk::Image VkImage)
	{
	}

	void RenderDeviceVk::DestroyTexture(TextureVk* pTexture)
	{
	}

	HardwareQueueVk* RenderDeviceVk::GetDefaultHardwareQueue()
	{
		return m_pDefaultHardwareQueue;
	}

	uint32_t RenderDeviceVk::GetNumExtraHardwareQueues()
	{
		return m_ExtraHardwareQueues.size();
	}

	HardwareQueueVk* RenderDeviceVk::GetExtraHardwareQueue(uint32_t HardwareQueueIndex)
	{
		QGFX_VERIFY(HardwareQueueIndex < m_ExtraHardwareQueues.size(), "Hardware Queue Index (", HardwareQueueIndex, ") is unsupported. Render device only supports (", m_ExtraHardwareQueues.size(),") queues.");
		return m_ExtraHardwareQueues[HardwareQueueIndex];
	}

	/*void RenderDeviceVk::CreateFence(uint64_t InitialValue, IFence** ppFence)
	{
		if (m_bTimelineSemaphoresSupported)
		{
			*ppFence = MakeRefCountedObj<TimelineFenceVk>()(this, InitialValue);
		}
		else
		{
			QGFX_UNSUPPORTED("This is currently unsupported");
		}
	}*/
}