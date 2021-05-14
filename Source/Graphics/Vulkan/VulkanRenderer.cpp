#include "Qgfx/Graphics/Vulkan/VulkanRenderer.hpp"
#include "Qgfx/Common/MemoryAllocator.hpp"

namespace Qgfx
{
	static inline FeatureState GetFeatureState(FeatureState RequestedState, vk::Bool32 IsFeatureSupported, vk::Bool32& EnableFeature, const char* FeatureName)
	{
		switch (RequestedState)
		{
		case FeatureState::eDisabled:
		{
			EnableFeature = VK_FALSE;
			return FeatureState::eDisabled;
		}
		case FeatureState::eEnabled:
		{
			EnableFeature = IsFeatureSupported;
			if (IsFeatureSupported)
				return FeatureState::eEnabled;
			else
				QGFX_LOG_ERROR_AND_THROW(FeatureName, " not supported by this device");
		}

		case FeatureState::eOptional:
		{
			EnableFeature = IsFeatureSupported;
			return IsFeatureSupported ? FeatureState::eEnabled : FeatureState::eDisabled;
		}
		default:
			QGFX_UNEXPECTED("Unexpected feature state");
			EnableFeature = VK_FALSE;
			return FeatureState::eDisabled;
		}
	}

	VulkanRenderer::VulkanRenderer(RendererDesc Descriptor)
	{
		QGFX_VERIFY_EXPR(Descriptor.Api == RendererApi::eVulkan);

		VulkanRendererDesc NativeDescriptor = Descriptor.pNativeDesc != nullptr ? *static_cast<VulkanRendererDesc*>(Descriptor.pNativeDesc) : VulkanRendererDesc{};

		m_VkDispatch.init(NativeDescriptor.pfnLoaderHandle);

		std::vector<const char*> EnabledExtensions{};
		std::vector<const char*> EnabledLayers{};

		EnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef QGFX_PLATFORM_WIN32
		EnabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		std::vector<vk::LayerProperties> SupportedLayers = vk::enumerateInstanceLayerProperties(m_VkDispatch);
		std::vector<vk::ExtensionProperties> SupportedExtensions = vk::enumerateInstanceExtensionProperties(nullptr, m_VkDispatch);

		auto IsLayerSupported = [&SupportedLayers](const char* LayerName) -> bool
		{
			for (const auto& Layer : SupportedLayers)
			{
				if (std::strcmp(Layer.layerName, LayerName) == 0)
				{
					return true;
				}
			}

			return false;
		};

		auto IsExtensionSupported = [&SupportedExtensions](const char* ExtensionName) -> bool
		{
			for (const auto& Extension : SupportedExtensions)
			{
				if (std::strcmp(Extension.extensionName, ExtensionName) == 0)
				{
					return true;
				}
			}

			return false;
		};


		if (NativeDescriptor.bEnableValidation)
		{
			if (IsLayerSupported("VK_LAYER_KHRONOS_validation"))
			{
				EnabledLayers.push_back("VK_LAYER_KHRONOS_validation");
			}
			else
			{
				QGFX_LOG_ERROR("Validation enabled but VK_LAYER_KHRONOS_validation is not available. Make sure the proper Vulkan SDK is installed, and the appropriate pfnLoaderHandle provided.");
			}

			if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				EnabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			else
			{
				QGFX_LOG_ERROR("Validation enabled but VK_EXT_DEBUG_UTILS_EXTENSION not supported.");
			}
		}

		vk::ApplicationInfo AppInfo{};
		AppInfo.pNext = nullptr;
		AppInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);
		AppInfo.applicationVersion = NativeDescriptor.AppVersion;
		AppInfo.pApplicationName = NativeDescriptor.pAppName;
		AppInfo.engineVersion = NativeDescriptor.EngineVersion;
		AppInfo.pEngineName = NativeDescriptor.pEngineName;

		vk::InstanceCreateInfo InstanceCI{};
		InstanceCI.pNext = nullptr;
		InstanceCI.flags = {};
		InstanceCI.pApplicationInfo = &AppInfo;
		InstanceCI.enabledLayerCount = static_cast<uint32_t>(EnabledLayers.size());
		InstanceCI.ppEnabledLayerNames = EnabledLayers.data();
		InstanceCI.enabledExtensionCount = static_cast<uint32_t>(EnabledExtensions.size());
		InstanceCI.ppEnabledExtensionNames = EnabledExtensions.data();

		try
		{
			m_VkInstance = vk::createInstance(InstanceCI, nullptr, m_VkDispatch);
			m_VkDispatch.init(m_VkInstance);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vkCreateInstance failed with error: ", Error.what());
		}

		m_PhysicalDevices = m_VkInstance.enumeratePhysicalDevices(m_VkDispatch);
	}

	VulkanRenderer::~VulkanRenderer()
	{
		m_VkInstance.destroy(nullptr, m_VkDispatch);
	}

	void VulkanRenderer::EnumerateAdapters(uint32_t Index, IAdapter** ppAdapter)
	{
		QGFX_VERIFY(Index < GetAdapterCount(), "Index must be less than IRenderer::GetAdapterCount()");
	}

	void VulkanRenderer::CreateDevice(IAdapter* pAdapter, const DeviceDesc& Descriptor, IDevice** ppDevice)
	{
		*ppDevice =  new VulkanDevice(this, ValidatedCast<VulkanAdapter>(pAdapter), Descriptor);
	}

	void VulkanRenderer::CreateSwapChain(IQueue* pQueue, const SwapChainDesc& Descriptor, ISwapChain** ppSwapChain)
	{
		*ppSwapChain = new VulkanSwapChain(this, ValidatedCast<VulkanQueue>(pQueue), Descriptor);
	}

	void VulkanRenderer::DeleteThis()
	{
		delete this;
	}

	///////////////////////////////
	// Adapter ////////////////////
	///////////////////////////////

	VulkanAdapter::VulkanAdapter(VulkanRenderer* pRenderer, vk::PhysicalDevice VkPhysicalDevice)
		: IAdapter(pRenderer), m_VkPhysicalDevice(VkPhysicalDevice)
	{
	}

	VulkanAdapter::~VulkanAdapter()
	{
	}


	void VulkanAdapter::DeleteThis()
	{
		delete this;
	}

	/////////////////////////////////
	// Device ///////////////////////
	/////////////////////////////////

	VulkanDevice::VulkanDevice(VulkanRenderer* pRenderer, VulkanAdapter* pAdapter, const DeviceDesc& Descriptor)
		: IDevice(pRenderer, pAdapter), m_pVulkanRenderer(pRenderer)
	{
		vk::Instance VkInstance = m_pVulkanRenderer->GetVkInstance();
		m_VkDispatch = m_pVulkanRenderer->GetVkInstanceDispatch();

		m_VkPhDevice = pAdapter->GetVkPhysicalDevice();

		// Features

		auto& RequestedFeatures = Descriptor.Features;

		m_SupportedFeatures.IndirectRendering = FeatureState::eEnabled;
		m_SupportedFeatures.ComputeShaders = FeatureState::eEnabled;

		vk::PhysicalDeviceFeatures2 SupportedFeatures2{};
		vk::PhysicalDeviceFeatures& Supported10Features = SupportedFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Supported11Features{};
		vk::PhysicalDeviceVulkan12Features Supported12Features{};
		SupportedFeatures2.pNext = &Supported11Features;
		Supported11Features.pNext = &Supported12Features;

		m_VkPhDevice.getFeatures2(&SupportedFeatures2, m_VkDispatch);

		vk::PhysicalDeviceFeatures2 EnabledFeatures2{};
		vk::PhysicalDeviceFeatures& Enabled10Features = EnabledFeatures2.features;
		vk::PhysicalDeviceVulkan11Features Enabled11Features{};
		vk::PhysicalDeviceVulkan12Features Enabled12Features{};
		EnabledFeatures2.pNext = &Enabled11Features;
		Enabled11Features.pNext = &Enabled12Features;

		m_SupportedFeatures.GeometryShaders =    GetFeatureState(RequestedFeatures.GeometryShaders, Supported10Features.geometryShader, Enabled10Features.geometryShader, "Geometry shaders are");
		m_SupportedFeatures.TesselationShaders = GetFeatureState(RequestedFeatures.TesselationShaders, Supported10Features.tessellationShader, Enabled10Features.tessellationShader, "Tesselation is");
		m_SupportedFeatures.PolygonModeLine =    GetFeatureState(RequestedFeatures.PolygonModeLine, Supported10Features.fillModeNonSolid, Enabled10Features.fillModeNonSolid, "Line polygon mode is");
		m_SupportedFeatures.PolygonModePoint =   GetFeatureState(RequestedFeatures.PolygonModePoint, Supported10Features.fillModeNonSolid, Enabled10Features.fillModeNonSolid, "Point polygon mode is");


		if (Supported12Features.timelineSemaphore)
		{
			Enabled12Features.timelineSemaphore = true;
			//m_bTimelineSemaphoresSupported = true;
		}

		// Extensions
		std::vector<vk::ExtensionProperties> SupportedExtensions = m_VkPhDevice.enumerateDeviceExtensionProperties(nullptr, m_VkDispatch);

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

		// Create one queue from every queue family

		std::vector<vk::QueueFamilyProperties> QueueFamilyProps = m_VkPhDevice.getQueueFamilyProperties(m_VkDispatch);

		uint32_t QueueCreateInfoCount = 0;
		std::vector<vk::DeviceQueueCreateInfo> QueueCreateInfos(QueueFamilyProps.size());
		std::vector<std::vector<float>> QueueCreateInfoPriorities(QueueCreateInfos.size());

		
		for (uint32_t Index = 0; Index < QueueFamilyProps.size(); Index++)
		{
			uint32_t QueueCount = QueueFamilyProps[Index].queueCount;
			if (QueueCount > 0)
			{
				QueueCreateInfoPriorities[QueueCreateInfoCount].resize(1, 1.0f);

				QueueCreateInfos[QueueCreateInfoCount].pNext = nullptr;
				QueueCreateInfos[QueueCreateInfoCount].flags = {};
				QueueCreateInfos[QueueCreateInfoCount].queueFamilyIndex = Index;
				QueueCreateInfos[QueueCreateInfoCount].queueCount = 1;
				QueueCreateInfos[QueueCreateInfoCount].pQueuePriorities = QueueCreateInfoPriorities[QueueCreateInfoCount].data();

				QueueCreateInfoCount++;
			}
		}

		////////////////////////
		// Device Create Info //
		////////////////////////

		vk::DeviceCreateInfo DeviceCreateInfo{};
		DeviceCreateInfo.pNext = &EnabledFeatures2;
		DeviceCreateInfo.flags = {}; /*Reserved for future use*/
		DeviceCreateInfo.enabledLayerCount = 0; /*Deprecated */
		DeviceCreateInfo.ppEnabledLayerNames = nullptr; /*Deprecated */
		DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(EnabledExtensions.size());
		DeviceCreateInfo.ppEnabledExtensionNames = EnabledExtensions.data();
		DeviceCreateInfo.queueCreateInfoCount = QueueCreateInfoCount;
		DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
		DeviceCreateInfo.pEnabledFeatures = nullptr; /*Set via vk::PhysicalDeviceFeatures2 in the pNext chain*/

		try
		{
			m_VkDevice = m_VkPhDevice.createDevice(DeviceCreateInfo, nullptr, m_VkDispatch);
			m_VkDispatch.init(m_VkDevice);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vkCreateDevice failed with error: ", Error.what());
		}

		//QueueFamilies.resize(QueueFamilyProps.size());
		//
		//for (uint32_t Index = 0; Index < QueueFamilyProps.size(); Index++)
		//{
		//	QueueFamilies[Index].QueueFlags = QueueFamilyProps[Index].queueFlags;

		//	uint32_t QueueCount = QueueFamilyProps[Index].queueCount > 0 ? 1 : 0; // This will change once flag is added
		//	QueueFamilies.resize(QueueCount);

		//	for (uint32_t QueueIndex = 0; QueueIndex < QueueCount; QueueIndex++)
		//	{
		//		QueueFamilies[Index].Queues[QueueIndex].pMutex = std::make_unique<std::mutex>();
		//		QueueFamilies[Index].Queues[QueueIndex].VkHandle = m_VkDevice.getQueue(Index, QueueIndex, m_VkDispatch);
		//	}
		//}

		m_GraphicsQueueFamilyIndex = UINT32_MAX;
		m_ComputeQueueFamilyIndex  = UINT32_MAX;
		m_TransferQueueFamilyIndex = UINT32_MAX;

		for (uint32_t Index = 0; Index < QueueFamilyProps.size(); Index++)
		{
			const auto& FamilyProps = QueueFamilyProps[Index];

			if (FamilyProps.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				if (m_GraphicsQueueFamilyIndex == UINT32_MAX)
				{
					m_GraphicsQueueFamilyIndex = Index;
					break;
				}
			}

			if ((FamilyProps.queueFlags & vk::QueueFlagBits::eCompute))
			{
				if (m_ComputeQueueFamilyIndex == UINT32_MAX)
				{
					m_ComputeQueueFamilyIndex = Index;
					break;
				}
			}

			if ((FamilyProps.queueFlags & vk::QueueFlagBits::eTransfer))
			{
				if (m_TransferQueueFamilyIndex == UINT32_MAX)
				{
					m_TransferQueueFamilyIndex = Index;
					break;
				}
			}
		}

		if (m_ComputeQueueFamilyIndex == UINT32_MAX)
		{ // No dedicated compute queue, use graphics
			m_ComputeQueueFamilyIndex = m_GraphicsQueueFamilyIndex;
		}

		if (m_TransferQueueFamilyIndex == UINT32_MAX)
		{ // No dedicated transfer queue, use graphics
			m_TransferQueueFamilyIndex = m_GraphicsQueueFamilyIndex;
		}

		////////////////////////////
		// Memory Allocator ////////
		////////////////////////////

		VmaVulkanFunctions Funcs{};
		Funcs.vkAllocateMemory = m_VkDispatch.vkAllocateMemory;
		Funcs.vkBindBufferMemory = m_VkDispatch.vkBindBufferMemory;
		Funcs.vkBindImageMemory = m_VkDispatch.vkBindImageMemory;
		Funcs.vkCmdCopyBuffer = m_VkDispatch.vkCmdCopyBuffer;
		Funcs.vkCreateBuffer = m_VkDispatch.vkCreateBuffer;
		Funcs.vkCreateImage = m_VkDispatch.vkCreateImage;
		Funcs.vkDestroyBuffer = m_VkDispatch.vkDestroyBuffer;
		Funcs.vkDestroyImage = m_VkDispatch.vkDestroyImage;
		Funcs.vkFlushMappedMemoryRanges = m_VkDispatch.vkFlushMappedMemoryRanges;
		Funcs.vkFreeMemory = m_VkDispatch.vkFreeMemory;
		Funcs.vkGetBufferMemoryRequirements = m_VkDispatch.vkGetBufferMemoryRequirements;
		Funcs.vkGetImageMemoryRequirements = m_VkDispatch.vkGetImageMemoryRequirements;
		Funcs.vkGetPhysicalDeviceMemoryProperties = m_VkDispatch.vkGetPhysicalDeviceMemoryProperties;
		Funcs.vkInvalidateMappedMemoryRanges = m_VkDispatch.vkInvalidateMappedMemoryRanges;
		Funcs.vkMapMemory = m_VkDispatch.vkMapMemory;
		Funcs.vkUnmapMemory = m_VkDispatch.vkUnmapMemory;
		Funcs.vkGetPhysicalDeviceMemoryProperties2KHR = m_VkDispatch.vkGetPhysicalDeviceMemoryProperties2;
		Funcs.vkBindBufferMemory2KHR = m_VkDispatch.vkBindBufferMemory2;
		Funcs.vkBindImageMemory2KHR = m_VkDispatch.vkBindImageMemory2;
		Funcs.vkGetBufferMemoryRequirements2KHR = m_VkDispatch.vkGetBufferMemoryRequirements2;
		Funcs.vkGetImageMemoryRequirements2KHR = m_VkDispatch.vkGetImageMemoryRequirements2;

		VmaAllocatorCreateFlags Flags = 0;

		if (bMemoryBudgetExtEnabled)
		{
			Flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		}

		VmaAllocatorCreateInfo AllocatorCI{};
		AllocatorCI.instance = VkInstance;
		AllocatorCI.physicalDevice = m_VkPhDevice;
		AllocatorCI.device = m_VkDevice;
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

	VulkanDevice::~VulkanDevice()
	{
		m_VkDevice.waitIdle(m_VkDispatch);

		vmaDestroyAllocator(m_VmaAllocator);

		m_VkDevice.destroy();
		m_VkPhDevice = nullptr;

		m_pAdapter->Release();
		m_pRenderer->Release();
	}

	IQueue* VulkanDevice::CreateQueue(const QueueDesc& Descriptor)
	{
		return new VulkanQueue(this, Descriptor);
	}

	void VulkanDevice::WaitIdle()
	{
		m_VkDevice.waitIdle(m_VkDispatch);
	}

	void VulkanDevice::DeleteThis()
	{
		delete this;
	}

	vk::Format VulkanDevice::GetVkFormat(TextureFormat Format)
	{
		return vk::Format();
	}

	vk::Semaphore VulkanDevice::CreateVkBinarySemaphore() const
	{
		vk::SemaphoreCreateInfo SemaphoreCI{};
		SemaphoreCI.pNext = nullptr;
		SemaphoreCI.flags = {};

		return m_VkDevice.createSemaphore(SemaphoreCI, nullptr, m_VkDispatch);
	}

	void VulkanDevice::DestroyVkSemaphore(vk::Semaphore Sem) const
	{
		m_VkDevice.destroySemaphore(Sem, nullptr, m_VkDispatch);
	}

	vk::Queue VulkanDevice::GetVkQueue(uint32_t QueueFamilyIndex) const
	{
		return m_VkDevice.getQueue(QueueFamilyIndex, 0, m_VkDispatch);
	}

	void VulkanDevice::VkQueueSubmit(vk::Queue VkQueue, const vk::ArrayProxy<const vk::SubmitInfo>& Submits, vk::Fence VkFence)
	{
		std::lock_guard Lock{ m_SubmitMutex };
		VkQueue.submit(Submits, VkFence, m_VkDispatch);
	}

	vk::Result VulkanDevice::VkQueuePresent(vk::Queue VkQueue, const vk::PresentInfoKHR& Present)
	{
		std::lock_guard Lock{ m_SubmitMutex };
		return VkQueue.presentKHR(Present, m_VkDispatch);
	}

	uint32_t VulkanDevice::GetQueueFamily(QueueType Type) const
	{
		switch (Type)
		{
		case Qgfx::QueueType::eGraphics: return m_GraphicsQueueFamilyIndex;
		case Qgfx::QueueType::eCompute:  return m_ComputeQueueFamilyIndex;
		case Qgfx::QueueType::eTransfer: return m_TransferQueueFamilyIndex;
		default:
			QGFX_UNEXPECTED("QueueType invalid");
		}
		return 0;
	}

	VulkanQueue::VulkanQueue(VulkanDevice* pDevice, const QueueDesc& Descriptor)
		: IQueue(pDevice), m_CommandBufferObjAllocator(pDevice->GetRawMemAllocator(), sizeof(VulkanCommandBuffer), 128)
	{

		vk::Device VkDevice = pDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = pDevice->GetVkDeviceDispatch();

		m_QueueFamilyIndex = pDevice->GetQueueFamily(Descriptor.Type);
		m_VkQueue = pDevice->GetVkQueue(m_QueueFamilyIndex);
	}

	VulkanQueue::~VulkanQueue()
	{
	}

	void VulkanQueue::CreateCommandBuffer(ICommandBuffer** ppCommandBuffer)
	{
		std::lock_guard Lock{ m_AllocMutex };

		VulkanCommandBuffer* pCommandBuffer = reinterpret_cast<VulkanCommandBuffer*>(m_CommandBufferObjAllocator.Allocate(sizeof(VulkanCommandBuffer)));
		new(pCommandBuffer) VulkanCommandBuffer(this);
		*ppCommandBuffer = pCommandBuffer;
	}

	void VulkanQueue::DeleteThis()
	{
		delete this;
	}

	void VulkanQueue::DestroyVulkanCommandBuffer(VulkanCommandBuffer* pCommandBuffer)
	{
		std::lock_guard Lock{ m_AllocMutex };

		pCommandBuffer->~VulkanCommandBuffer();
		m_CommandBufferObjAllocator.Free(pCommandBuffer);
	}

	///////////////////////////////
	// Command Buffer /////////////
	///////////////////////////////

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanQueue* pQueue)
		: ICommandBuffer(pQueue)
	{
		m_pVulkanQueue = pQueue;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
	}

	void VulkanCommandBuffer::Finish()
	{

	}

	void VulkanCommandBuffer::DeleteThis()
	{
		m_pVulkanQueue->DestroyVulkanCommandBuffer(this);
	}

	////////////////////////////////
	// SwapChain ///////////////////
	////////////////////////////////

	VulkanSwapChain::VulkanSwapChain(VulkanRenderer* pRenderer, VulkanQueue* pQueue, const SwapChainDesc& Descriptor)
		: ISwapChain(pRenderer, pQueue, Descriptor), m_pVulkanRenderer(pRenderer), m_pVulkanQueue(pQueue)
	{
		m_pVulkanQueue->GetDevice(reinterpret_cast<IDevice**>(&m_pVulkanDevice));

		vk::Device VkDevice = m_pVulkanDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		m_DesiredTextureCount = Descriptor.TextureCount;
		m_DesiredPreTransform = Descriptor.PreTransform;
		m_DesiredUsage =        Descriptor.Usage;

		m_GraphicsQueue = m_pVulkanQueue->GetVkQueue();

		vk::CommandPoolCreateInfo CommandPoolCI{};
		CommandPoolCI.flags = {};
		CommandPoolCI.pNext = nullptr;
		CommandPoolCI.queueFamilyIndex = m_pVulkanQueue->GetVkQueueFamily();

		m_CmdPool = VkDevice.createCommandPool(CommandPoolCI, nullptr, VkDispatch);

		CreateSurface();
		CreateSwapChain();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{

		vk::Instance VkInstance = m_pVulkanRenderer->GetVkInstance();
		vk::Device VkDevice = m_pVulkanDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		ReleaseSwapChainResources(true);

		if (m_VkSurface)
		{
			VkInstance.destroySurfaceKHR(m_VkSurface, nullptr, VkDispatch);
			m_VkSurface = nullptr;
		}

		VkDevice.destroyCommandPool(m_CmdPool, nullptr, VkDispatch);

		m_pVulkanDevice->Release();
	}

	SwapChainOpResult VulkanSwapChain::AcquireNextTextureImpl()
	{
		vk::Device VkDevice = m_pVulkanDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		// Applications should not rely on vkAcquireNextImageKHR blocking in order to
		// meter their rendering speed. The implementation may return from this function
		// immediately regardless of how many presentation requests are queued, and regardless
		// of when queued presentation requests will complete relative to the call. Instead,
		// applications can use fence to meter their frame generation work to match the
		// presentation rate.

		// Explicitly make sure that there are no more pending frames in the command queue
		// than the number of the swap chain images.
		//
		// Nsc = 3 - number of the swap chain images
		//
		//   N-Ns          N-2           N-1            N (Current frame)
		//    |             |             |             |
		//                  |
		//          Wait for this fence
		//
		// When acquiring swap chain image for frame N, we need to make sure that
		// frame N-Nsc has completed. To achieve that, we wait for the image acquire
		// fence for frame N-Nsc-1. Thus we will have no more than Nsc frames in the queue.

		m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_TextureCount;

		uint32_t m_OldestSemaphoreIndex = (m_SemaphoreIndex + 1) % m_TextureCount;

		if (m_bImageAcquired[m_OldestSemaphoreIndex])
		{
			vk::Result Res = VkDevice.getFenceStatus(m_ImageAcquiredFences[m_OldestSemaphoreIndex], VkDispatch);
			if (Res == vk::Result::eErrorDeviceLost)
				QGFX_LOG_ERROR_AND_THROW("Unexpected device loss.");

			if (Res == vk::Result::eNotReady)
				VkDevice.waitForFences(m_ImageAcquiredFences[m_OldestSemaphoreIndex], true, UINT64_MAX, VkDispatch);

			VkDevice.resetFences(m_ImageAcquiredFences[m_OldestSemaphoreIndex], VkDispatch);

			m_bImageAcquired[m_OldestSemaphoreIndex] = false;
		}

		vk::Result Res = VkDevice.acquireNextImageKHR(m_VkSwapchain, UINT64_MAX, m_ImageAcquiredSemaphores[m_SemaphoreIndex], {}, &m_TextureIndex, VkDispatch);

		m_bImageAcquired[m_SemaphoreIndex] = (Res == vk::Result::eSuccess);

		if (Res == vk::Result::eErrorOutOfDateKHR)
		{
			return SwapChainOpResult::eOutOfDate;
		}

		vk::PipelineStageFlags WaitDstStages = vk::PipelineStageFlagBits::eAllCommands;

		vk::SubmitInfo Submit{};
		Submit.pNext = nullptr;
		Submit.waitSemaphoreCount = 1;
		Submit.pWaitSemaphores = &m_ImageAcquiredSemaphores[m_SemaphoreIndex];
		Submit.pWaitDstStageMask = &WaitDstStages;
		Submit.signalSemaphoreCount = 0;
		Submit.pSignalSemaphores = nullptr;

		if (m_Flags & SwapChainCreationFlagBits::eClearOnAcquire)
		{
			Submit.commandBufferCount = 1;
			Submit.pCommandBuffers = &m_ClearOnAcquireCommands[m_TextureIndex];
		}
		else
		{
			Submit.commandBufferCount = 0;
			Submit.pCommandBuffers = nullptr;
		}
		
		m_pVulkanDevice->VkQueueSubmit(m_GraphicsQueue, Submit, m_ImageAcquiredFences[m_SemaphoreIndex]);

		if (Res == vk::Result::eSuboptimalKHR)
		{
			return SwapChainOpResult::eSuboptimal;
		}

		return SwapChainOpResult::eSuccess;
	}

	SwapChainOpResult VulkanSwapChain::PresentImpl()
	{
		vk::SubmitInfo Submit{};
		Submit.pNext = nullptr;
		Submit.commandBufferCount = 0;
		Submit.pCommandBuffers = nullptr;
		Submit.waitSemaphoreCount = 0;
		Submit.pWaitSemaphores = nullptr;
		Submit.pWaitDstStageMask = nullptr;
		Submit.signalSemaphoreCount = 1;
		Submit.pSignalSemaphores = &m_SubmitCompleteSemaphores[m_SemaphoreIndex];

		m_pVulkanDevice->VkQueueSubmit(m_GraphicsQueue, Submit, {});

		vk::PresentInfoKHR PresentInfo{};
		PresentInfo.pNext = nullptr;
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = &m_SubmitCompleteSemaphores[m_SemaphoreIndex];
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = &m_VkSwapchain;
		PresentInfo.pImageIndices = &m_TextureIndex;
		PresentInfo.pResults = nullptr;

		vk::Result Res = m_pVulkanDevice->VkQueuePresent(m_PresentQueue, PresentInfo);

		switch (Res)
		{
		case vk::Result::eSuccess:             return SwapChainOpResult::eSuccess;
		case vk::Result::eSuboptimalKHR:       return SwapChainOpResult::eSuboptimal;
		case vk::Result::eErrorOutOfDateKHR:   return SwapChainOpResult::eOutOfDate;
		case vk::Result::eErrorSurfaceLostKHR: return SwapChainOpResult::eSurfaceLost;
		default: 
		{
			QGFX_UNEXPECTED("Result of presentKHR unexpected");
			return SwapChainOpResult::eOutOfDate;
		}
		}
	}

	void VulkanSwapChain::ResizeImpl(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform)
	{
		bool bRecreateSwapChain = false;

#if 0
		if (m_VkSurface)
		{
			// Check orientation
			vk::PhysicalDevice VkPhDevice = m_spRenderDevice->GetVkPhysicalDevice();
			vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
			const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

			vk::SurfaceCapabilitiesKHR surfCapabilities = {};

			vk::Result err = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, &surfCapabilities, VkDispatch);
			if (err == vk::Result::eSuccess)
			{
				if (m_CurrentSurfaceTransform != surfCapabilities.currentTransform)
				{
					// Surface orientation has changed - we need to recreate the swap chain
					bRecreateSwapChain = true;
				}

				constexpr auto Rotate90TransformFlags =
					vk::SurfaceTransformFlagBitsKHR::eRotate90 |
					vk::SurfaceTransformFlagBitsKHR::eRotate270 |
					vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90 |
					vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;

				if (NewWidth == 0 || NewHeight == 0)
				{
					NewWidth = m_SurfaceIdentityExtent.width;
					NewHeight = m_SurfaceIdentityExtent.height;

					if (surfCapabilities.currentTransform & Rotate90TransformFlags)
					{
						// Swap to get logical dimensions as input NewWidth and NewHeight are
						// expected to be logical sizes.
						std::swap(NewWidth, NewHeight);
					}
				}

				if (NewPreTransform == SurfaceTransform::Optimal)
				{
					if (surfCapabilities.currentTransform & Rotate90TransformFlags)
					{
						// Swap to get physical dimensions
						std::swap(NewWidth, NewHeight);
					}
				}
				else
				{
					// Swap if necessary to get desired sizes after pre-transform
					if (NewPreTransform == SurfaceTransform::Rotate90 ||
						NewPreTransform == SurfaceTransform::Rotate270 ||
						NewPreTransform == SurfaceTransform::HorizontalMirrorRotate90 ||
						NewPreTransform == SurfaceTransform::HorizontalMirrorRotate270)
					{
						std::swap(NewWidth, NewHeight);
					}
				}
			}
			else
			{
				QGFX_LOG_ERROR_MESSAGE(err, "Failed to query physical device surface capabilities");
			}
		}
#endif

		if (NewWidth != m_Width || NewHeight != m_Height || NewPreTransform != m_PreTransform)
		{
			m_Width = NewWidth;
			m_Height = NewHeight;
			m_DesiredPreTransform = m_PreTransform;

			bRecreateSwapChain = true;
		}

		if (bRecreateSwapChain)
		{
			QGFX_LOG_INFO_MESSAGE("Resizing swapchain to (", NewWidth, ",", NewHeight, ")");

			RecreateSwapChain();
		}
	}

	void VulkanSwapChain::CreateSurface()
	{
		vk::Instance VkInstance = m_pVulkanRenderer->GetVkInstance();
		vk::PhysicalDevice VkPhDevice = m_pVulkanDevice->GetVkPhDevice();
		vk::Device VkDevice = m_pVulkanDevice->GetVkDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		if (m_VkSurface)
		{
			VkInstance.destroySurfaceKHR(m_VkSurface, nullptr, VkDispatch);
			m_VkSurface = nullptr;
		}

		try
		{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			if (m_Window.hWnd != NULL)
			{
				vk::Win32SurfaceCreateInfoKHR SurfaceCreateInfo = {};

				SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
				SurfaceCreateInfo.hwnd = (HWND)m_Window.hWnd;

				m_VkSurface = VkInstance.createWin32SurfaceKHR(SurfaceCreateInfo, nullptr, VkDispatch);
			}
#endif
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vk::SurfaceKHR creation failed with error: ", Error.what());
		}

		std::vector<vk::QueueFamilyProperties> QueueFamilyProps = VkPhDevice.getQueueFamilyProperties(VkDispatch);

		std::vector<uint32_t> PresentQueues;

		for (uint32_t Index = 0; Index < QueueFamilyProps.size(); Index++)
		{
			bool bSupportsPresent = VkPhDevice.getSurfaceSupportKHR(Index, m_VkSurface, VkDispatch);
			if (bSupportsPresent)
				PresentQueues.push_back(Index);
		}

		enum class PresentQueueType
		{
			eDedicated = 0,
			eGraphicsWithPresentSupport = 1,
			eSuboptimalWithPresentSupport = 2,
			eUnfound = 3
		};

		PresentQueueType PresentType = PresentQueueType::eUnfound;

		for (uint32_t PresentQueueFamilyIndex : PresentQueues)
		{
			PresentQueueType Type;

			const vk::QueueFamilyProperties& FamilyProps = QueueFamilyProps[PresentQueueFamilyIndex];

			if (!FamilyProps.queueFlags)
			{
				Type = PresentQueueType::eDedicated;
			}
			else if (FamilyProps.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				Type = PresentQueueType::eGraphicsWithPresentSupport;
			}
			else
			{
				Type = PresentQueueType::eSuboptimalWithPresentSupport;
			}

			if (Type < PresentType)
			{
				m_PresentQueueFamilyIndex = PresentQueueFamilyIndex;
			}
		}

		if (PresentType == PresentQueueType::eUnfound)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to find queue family which supports presentation to swapchain surface");
		}

		m_PresentQueue = m_pVulkanDevice->GetVkQueue(m_PresentQueueFamilyIndex);
	}

	void VulkanSwapChain::CreateSwapChain()
	{
		vk::Device VkDevice = m_pVulkanDevice->GetVkDevice();
		vk::PhysicalDevice VkPhDevice = m_pVulkanDevice->GetVkPhDevice();
		const vk::DispatchLoaderDynamic& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		std::vector<vk::SurfaceFormatKHR> SupportedFormats{};

		try
		{
			SupportedFormats = VkPhDevice.getSurfaceFormatsKHR(m_VkSurface, VkDispatch);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to query avalaible surface formats: ", Error.what());
		}

		m_VkColorFormat = m_pVulkanDevice->GetVkFormat(m_Format);

		vk::ColorSpaceKHR ColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		if (SupportedFormats.size() == 1 && SupportedFormats[0].format == vk::Format::eUndefined)
		{
			// If the format list includes just one entry of vk::Format::eUndefined,
			// the surface has no preferred format.  Otherwise, at least one
			// supported format will be returned.

			// Do nothing
		}
		else
		{
			bool FmtFound = false;
			for (const auto& SrfFmt : SupportedFormats)
			{
				if (SrfFmt.format == m_VkColorFormat)
				{
					FmtFound = true;
					ColorSpace = SrfFmt.colorSpace;
					break;
				}
			}
			if (!FmtFound)
			{
				vk::Format VkReplacementColorFormat = vk::Format::eUndefined;
				switch (m_VkColorFormat)
				{
				case vk::Format::eR8G8B8A8Unorm: VkReplacementColorFormat = vk::Format::eB8G8R8A8Unorm; break;
				case vk::Format::eB8G8R8A8Unorm: VkReplacementColorFormat = vk::Format::eR8G8B8A8Unorm; break;
				case vk::Format::eB8G8R8A8Srgb:  VkReplacementColorFormat = vk::Format::eR8G8B8A8Srgb;  break;
				case vk::Format::eR8G8B8A8Srgb:  VkReplacementColorFormat = vk::Format::eB8G8R8A8Srgb;  break;
				default: VkReplacementColorFormat = vk::Format::eUndefined;
				}

				bool ReplacementFmtFound = false;
				for (const auto& SrfFmt : SupportedFormats)
				{
					if (SrfFmt.format == VkReplacementColorFormat)
					{
						ReplacementFmtFound = true;
						ColorSpace = SrfFmt.colorSpace;
						break;
					}
				}

				if (ReplacementFmtFound)
				{
					QGFX_LOG_INFO_MESSAGE("Requested color buffer format ", vk::to_string(m_VkColorFormat), " is not supported by the surface and will be replaced with ", vk::to_string(VkReplacementColorFormat));
					m_VkColorFormat = VkReplacementColorFormat;

					switch (VkReplacementColorFormat)
					{
					case vk::Format::eR8G8B8A8Unorm: m_Format = TextureFormat::eRGBA8Unorm; break;
					case vk::Format::eB8G8R8A8Unorm: m_Format = TextureFormat::eBGRA8Unorm; break;
					case vk::Format::eB8G8R8A8Srgb:  m_Format = TextureFormat::eBGRA8UnormSrgb;  break;
					case vk::Format::eR8G8B8A8Srgb: m_Format = TextureFormat::eRGBA8UnormSrgb;  break;
					default:
						QGFX_UNEXPECTED("Unexpected replacement color");
						m_Format = TextureFormat::eRGBA8Unorm;
					}

					m_VkClearValue = VulkanConversion::GetVkClearValue(m_ClearColor, m_Format);
				}
				else
				{
					// QGFX_LOG_WARNING_MESSAGE("Requested color buffer format ", GetTextureFormatAttribs(m_SwapChainDesc.ColorBufferFormat).Name, "is not supported by the surface");
				}
			}
		}

		vk::SurfaceCapabilitiesKHR SurfCapabilities = {};

		try
		{
			SurfCapabilities = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, VkDispatch);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
		}

		std::vector<vk::PresentModeKHR> PresentModes = {};

		try
		{
			PresentModes = VkPhDevice.getSurfacePresentModesKHR(m_VkSurface, VkDispatch);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
		}

		vk::SurfaceTransformFlagBitsKHR VkPreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
		if (m_DesiredPreTransform != SurfaceTransform::eOptimal)
		{
			VkPreTransform = VulkanConversion::GetVkSurfaceTransformKHR(m_DesiredPreTransform);
			if (SurfCapabilities.supportedTransforms & VkPreTransform)
			{
				m_PreTransform = m_DesiredPreTransform;
			}
			else
			{
				//LOG_WARNING_MESSAGE(GetSurfaceTransformString(m_DesiredPreTransform),
				//    " is not supported by the presentation engine. Optimal surface transform will be used instead."
				//    " Query the swap chain description to get the actual surface transform.");
				m_DesiredPreTransform = SurfaceTransform::eOptimal;
			}
		}

		if (m_DesiredPreTransform == SurfaceTransform::eOptimal)
		{
			// Use current surface transform to avoid extra cost of presenting the image.
			// If preTransform does not match the currentTransform value returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
			// the presentation engine will transform the image content as part of the presentation operation.
			// https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
			// https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation
			VkPreTransform = SurfCapabilities.currentTransform;
			m_PreTransform = VulkanConversion::GetSurfaceTransform(VkPreTransform);
			// QGFX_LOG_INFO_MESSAGE("Using ", GetSurfaceTransformString(m_SwapChainDesc.PreTransform), " swap chain pretransform");
		}

		VkExtent2D SwapchainExtent = {};
		// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
		if (SurfCapabilities.currentExtent.width == 0xFFFFFFFF && m_Width != 0 && m_Height != 0)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			SwapchainExtent.width = std::min(std::max(m_Width, SurfCapabilities.minImageExtent.width), SurfCapabilities.maxImageExtent.width);
			SwapchainExtent.height = std::min(std::max(m_Height, SurfCapabilities.minImageExtent.height), SurfCapabilities.maxImageExtent.height);
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			SwapchainExtent = SurfCapabilities.currentExtent;
		}

#if 0 // For Andriod
		// On Android, vkGetPhysicalDeviceSurfaceCapabilitiesKHR is not reliable and starts reporting incorrect
		// dimensions after few rotations. To alleviate the problem, we store the surface extent corresponding to
		// identity rotation.
		// https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
		if (m_SurfaceIdentityExtent.width == 0 || m_SurfaceIdentityExtent.height == 0)
		{
			m_SurfaceIdentityExtent = SurfCapabilities.currentExtent;
			constexpr auto Rotate90TransformFlags =
				vk::SurfaceTransformFlagBitsKHR::eRotate90 |
				vk::SurfaceTransformFlagBitsKHR::eRotate270 |
				vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90 |
				vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;
			if (SurfCapabilities.currentTransform & Rotate90TransformFlags)
				std::swap(m_SurfaceIdentityExtent.width, m_SurfaceIdentityExtent.height);
		}

		if (m_DesiredPreTransform == SurfaceTransform::Optimal)
		{
			SwapchainExtent = m_SurfaceIdentityExtent;
		}
		m_CurrentSurfaceTransform = SurfCapabilities.currentTransform;
#endif

		SwapchainExtent.width = std::max(SwapchainExtent.width, 1u);
		SwapchainExtent.height = std::max(SwapchainExtent.height, 1u);
		m_Width = SwapchainExtent.width;
		m_Height = SwapchainExtent.height;

		// The FIFO present mode is guaranteed by the spec to always be supported.
		vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
		{
			std::vector<vk::PresentModeKHR> PreferredPresentModes;
			if (m_bVSyncEnabled)
			{
				// FIFO relaxed waits for the next VSync, but if the frame is late,
				// it still shows it even if VSync has already passed, which may
				// result in tearing.
				PreferredPresentModes.push_back(vk::PresentModeKHR::eFifoRelaxed);
				PreferredPresentModes.push_back(vk::PresentModeKHR::eFifo);
			}
			else
			{
				// Mailbox is the lowest latency non-tearing presentation mode.
				PreferredPresentModes.push_back(vk::PresentModeKHR::eMailbox);
				PreferredPresentModes.push_back(vk::PresentModeKHR::eImmediate);
				PreferredPresentModes.push_back(vk::PresentModeKHR::eFifo);
			}

			for (auto PreferredMode : PreferredPresentModes)
			{
				if (std::find(PresentModes.begin(), PresentModes.end(), PreferredMode) != PresentModes.end())
				{
					PresentMode = PreferredMode;
					break;
				}
			}

			QGFX_LOG_INFO_MESSAGE("Using ", vk::to_string(PresentMode), " swap chain present mode");
		}

		// Determine the number of VkImage's to use in the swap chain.
		// We need to acquire only 1 presentable image at at time.
		// Asking for minImageCount images ensures that we can acquire
		// 1 presentable image as long as we present it before attempting
		// to acquire another.
		if (m_DesiredTextureCount < SurfCapabilities.minImageCount)
		{
			QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredTextureCount, ") is smaller than the minimal image count supported for this surface (", SurfCapabilities.minImageCount, "). Resetting to ", SurfCapabilities.minImageCount);
			m_DesiredTextureCount = SurfCapabilities.minImageCount;
		}
		if (SurfCapabilities.maxImageCount != 0 && m_DesiredTextureCount > SurfCapabilities.maxImageCount)
		{
			QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredTextureCount, ") is greater than the maximal image count supported for this surface (", SurfCapabilities.maxImageCount, "). Resetting to ", SurfCapabilities.maxImageCount);
			m_DesiredTextureCount = SurfCapabilities.maxImageCount;
		}

		m_Usage = m_DesiredUsage & VulkanConversion::GetResourceUsage(SurfCapabilities.supportedUsageFlags);

		// We must use m_DesiredBufferCount instead of m_SwapChainDesc.BufferCount, because Vulkan on Android
		// may decide to always add extra buffers, causing infinite growth of the swap chain when it is recreated:
		//                          m_SwapChainDesc.BufferCount
		// CreateVulkanSwapChain()          2 -> 4
		// CreateVulkanSwapChain()          4 -> 6
		// CreateVulkanSwapChain()          6 -> 8
		uint32_t DesiredNumberOfSwapChainImages = m_DesiredTextureCount;

		// Find a supported composite alpha mode - one of these is guaranteed to be set
		vk::CompositeAlphaFlagBitsKHR CompositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		vk::CompositeAlphaFlagBitsKHR CompositeAlphaFlags[4] = //
		{
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
			vk::CompositeAlphaFlagBitsKHR::eInherit,
		};
		for (uint32_t i = 0; i < 4; i++)
		{
			if (SurfCapabilities.supportedCompositeAlpha & CompositeAlphaFlags[i])
			{
				CompositeAlpha = CompositeAlphaFlags[i];
				break;
			}
		}

		vk::SwapchainKHR OldSwapchain = m_VkSwapchain;
		m_VkSwapchain = nullptr;

		vk::SwapchainCreateInfoKHR SwapChainCI = {};

		SwapChainCI.pNext = nullptr;
		SwapChainCI.surface = m_VkSurface;
		SwapChainCI.minImageCount = DesiredNumberOfSwapChainImages;
		SwapChainCI.imageFormat = m_VkColorFormat;
		SwapChainCI.imageExtent.width = SwapchainExtent.width;
		SwapChainCI.imageExtent.height = SwapchainExtent.height;
		SwapChainCI.preTransform = VkPreTransform;
		SwapChainCI.compositeAlpha = CompositeAlpha;
		SwapChainCI.imageArrayLayers = 1;
		SwapChainCI.presentMode = PresentMode;
		SwapChainCI.oldSwapchain = OldSwapchain;
		SwapChainCI.clipped = true;
		SwapChainCI.imageColorSpace = ColorSpace;
		SwapChainCI.imageUsage = VulkanConversion::GetVkImageUsage(m_Usage);

		std::set<uint32_t> QueueFamilyIndiciesSet;
		QueueFamilyIndiciesSet.emplace(m_pVulkanQueue->GetVkQueueFamily());
		QueueFamilyIndiciesSet.emplace(m_PresentQueueFamilyIndex);
		
		std::vector<uint32_t> QueueFamilyIndicies(QueueFamilyIndiciesSet.begin(), QueueFamilyIndiciesSet.end());

		SwapChainCI.imageSharingMode = QueueFamilyIndicies.size() == 1 ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
		SwapChainCI.queueFamilyIndexCount = static_cast<uint32_t>(QueueFamilyIndicies.size());
		SwapChainCI.pQueueFamilyIndices = QueueFamilyIndicies.data();

		try
		{
			m_VkSwapchain = VkDevice.createSwapchainKHR(SwapChainCI, nullptr, VkDispatch);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to create Vulkan swapchain: ", Error.what());
		}

		if (OldSwapchain)
		{
			VkDevice.destroySwapchainKHR(OldSwapchain, nullptr, VkDispatch);
		}

		try
		{
			vk::throwResultException(VkDevice.getSwapchainImagesKHR(m_VkSwapchain, &m_TextureCount, nullptr, VkDispatch), "Failed to get buffer count");
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to request swap chain image count: ", Error.what());
		}

		QGFX_VERIFY_EXPR(m_TextureCount > 0);


		if (m_DesiredTextureCount != m_TextureCount)
		{
			QGFX_LOG_INFO_MESSAGE("Created swap chain with ", m_TextureCount, " images vs ", m_DesiredTextureCount, " requested.");
		}

		m_bImageAcquired.resize(m_TextureCount);
		m_ImageAcquiredFences.resize(m_TextureCount);
		m_ImageAcquiredSemaphores.resize(m_TextureCount);
		m_SubmitCompleteSemaphores.resize(m_TextureCount);

		/*m_FrameTextures.resize(m_TextureCount);

		TextureCreateInfo TexCI{};
		TexCI.Dimension = TextureDimension::e2D;
		TexCI.Width = m_Width;
		TexCI.Height = m_Height;
		TexCI.ArraySize = 1;
		TexCI.Format = m_TextureFormat;
		TexCI.MipLevels = 1;
		TexCI.SampleCount = TextureSampleCount::e1;

		if (m_Usage & SwapChainUsageFlagBits::eRenderAttachment)
			TexCI.Usage |= TextureUsageFlagBits::eRenderAttachment;
		if (m_Usage & SwapChainUsageFlagBits::eSampled)
			TexCI.Usage |= TextureUsageFlagBits::eSampled;
		if (m_Usage & SwapChainUsageFlagBits::eTransferSrc)
			TexCI.Usage |= TextureUsageFlagBits::eTransferSrc;
		if (m_Usage & SwapChainUsageFlagBits::eTransferDst)
			TexCI.Usage |= TextureUsageFlagBits::eTransferDst;

		TexCI.pInitialQueue = m_spCommandQueue;*/

		std::vector<vk::Image> m_VkImages(m_TextureCount);
		vk::throwResultException(VkDevice.getSwapchainImagesKHR(m_VkSwapchain, &m_TextureCount, m_VkImages.data(), VkDispatch), "Failed to retrieve vulkan swapchain images");

		vk::CommandBuffer InitialClearCommandBuffer;

		vk::CommandBufferAllocateInfo AllocInfo{};
		AllocInfo.pNext = nullptr;
		AllocInfo.commandBufferCount = 1;
		AllocInfo.level = vk::CommandBufferLevel::ePrimary;
		AllocInfo.commandPool = m_CmdPool;

		vk::throwResultException(VkDevice.allocateCommandBuffers(&AllocInfo, &InitialClearCommandBuffer, VkDispatch), "Failed to allocate initial clear command buffer for swapchain");

		if (m_Flags & SwapChainCreationFlagBits::eClearOnAcquire)
		{
			AllocInfo.commandBufferCount = m_TextureCount;

			m_ClearOnAcquireCommands.resize(m_TextureCount);
			m_ClearOnAcquireFramebuffers.resize(m_TextureCount);
			m_ClearOnAcquireRenderPasses.resize(m_TextureCount);

			vk::throwResultException(VkDevice.allocateCommandBuffers(&AllocInfo, m_ClearOnAcquireCommands.data(), VkDispatch), "Failed to allocate clear on acquire command buffer for swapchain");
		}

		vk::CommandBufferBeginInfo BeginInfo{};
		BeginInfo.flags = {};
		BeginInfo.pNext = nullptr;
		BeginInfo.pInheritanceInfo = nullptr;

		InitialClearCommandBuffer.begin(BeginInfo, VkDispatch);

		for (uint32_t i = 0; i < m_TextureCount; ++i)
		{
			m_bImageAcquired[i] = false;

			vk::FenceCreateInfo FenceCI = {};

			FenceCI.pNext = nullptr;
			FenceCI.flags = {};

			m_ImageAcquiredFences[i] = VkDevice.createFence(FenceCI, nullptr, VkDispatch);

			vk::SemaphoreCreateInfo SemaphoreCI = {};

			SemaphoreCI.pNext = nullptr;
			SemaphoreCI.flags = {}; // reserved for future use

			m_ImageAcquiredSemaphores[i] = VkDevice.createSemaphore(SemaphoreCI, nullptr, VkDispatch);

			m_SubmitCompleteSemaphores[i] = VkDevice.createSemaphore(SemaphoreCI, nullptr, VkDispatch);

			// Command Buffers

			vk::ImageSubresourceRange AllSubresources{};
			AllSubresources.aspectMask = vk::ImageAspectFlagBits::eColor;
			AllSubresources.baseArrayLayer = 0;
			AllSubresources.layerCount = VK_REMAINING_ARRAY_LAYERS;
			AllSubresources.baseMipLevel = 0;
			AllSubresources.levelCount = VK_REMAINING_MIP_LEVELS;

			{
				vk::ImageMemoryBarrier ImageBarrier{};
				ImageBarrier.pNext = nullptr;
				ImageBarrier.image = m_VkImages[i];
				ImageBarrier.subresourceRange = AllSubresources;
				ImageBarrier.oldLayout = vk::ImageLayout::eUndefined;
				ImageBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
				ImageBarrier.srcAccessMask = {};
				ImageBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				
				InitialClearCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr, nullptr, ImageBarrier, VkDispatch);

				//InitialClearCommandBuffer.clearColorImage(m_VkImages[i], vk::ImageLayout::eTransferDstOptimal, m_VkClearColor, AllSubresources, VkDispatch);

				//ImageBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				//ImageBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
				//ImageBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				//ImageBarrier.dstAccessMask = {}; // Fences will ensure memory is available

				//InitialClearCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, {}, nullptr, nullptr, ImageBarrier, VkDispatch);
			}

			if (m_Flags & SwapChainCreationFlagBits::eClearOnAcquire)
			{
				vk::FramebufferCreateInfo FramebufferCI{};
				FramebufferCI.pNext = nullptr;
				FramebufferCI.flags = {};
				FramebufferCI.attachmentCount = 1;
				FramebufferCI.layers = 1;
				FramebufferCI.width = m_Width;
				FramebufferCI.height = m_Height;
				
				m_ClearOnAcquireFramebuffers[i] = VkDevice.createFramebuffer(FramebufferCI, nullptr, VkDispatch);

				vk::AttachmentDescription AttachmentDesc{};
				AttachmentDesc.flags = {};
				AttachmentDesc.format = m_VkColorFormat;
				AttachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
				AttachmentDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;
				AttachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
				AttachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
				AttachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				AttachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				AttachmentDesc.samples = vk::SampleCountFlagBits::e1;

				vk::AttachmentReference AttachmentRef{};
				AttachmentRef.attachment = 0;
				AttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

				vk::SubpassDescription SubpassDesc{};
				SubpassDesc.flags = {};
				SubpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
				SubpassDesc.colorAttachmentCount = 1;
				SubpassDesc.pColorAttachments = &AttachmentRef;
				SubpassDesc.pResolveAttachments = nullptr;
				SubpassDesc.inputAttachmentCount = 0;
				SubpassDesc.pInputAttachments = nullptr;
				SubpassDesc.preserveAttachmentCount = 0;
				SubpassDesc.pPreserveAttachments = nullptr;
				SubpassDesc.pDepthStencilAttachment = nullptr;

				vk::SubpassDependency InitialDependency{};
				InitialDependency.dependencyFlags = {};
				InitialDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				InitialDependency.srcAccessMask = {};
				InitialDependency.srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
				InitialDependency.dstSubpass = 0;
				InitialDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
				InitialDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;;

				vk::SubpassDependency FinalDependency{};
				InitialDependency.dependencyFlags = {};
				InitialDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				InitialDependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
				InitialDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
				InitialDependency.dstSubpass = 0;
				InitialDependency.dstAccessMask = {}; // This may change depending on how present barriers work
				InitialDependency.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;  // This may change depending on how present barriers work

				vk::SubpassDependency Dependencies[] = { InitialDependency, FinalDependency };

				vk::RenderPassCreateInfo RenderPassCI{};
				RenderPassCI.pNext = nullptr;
				RenderPassCI.flags = {};
				RenderPassCI.attachmentCount = 1;
				RenderPassCI.pAttachments = &AttachmentDesc;
				RenderPassCI.subpassCount = 1;
				RenderPassCI.pSubpasses = &SubpassDesc;
				RenderPassCI.dependencyCount = 2;
				RenderPassCI.pDependencies = Dependencies;

				m_ClearOnAcquireRenderPasses[i] = VkDevice.createRenderPass(RenderPassCI, nullptr, VkDispatch);

				vk::CommandBuffer ClearOnAcquireCommand = m_ClearOnAcquireCommands[i];

				ClearOnAcquireCommand.begin(BeginInfo, VkDispatch);

				vk::RenderPassBeginInfo BeginInfo{};
				BeginInfo.pNext = nullptr;
				BeginInfo.framebuffer = m_ClearOnAcquireFramebuffers[i];
				BeginInfo.renderPass = m_ClearOnAcquireRenderPasses[i];
				BeginInfo.renderArea.offset.x = 0;
				BeginInfo.renderArea.offset.y = 0;
				BeginInfo.renderArea.extent.width = m_Width;
				BeginInfo.renderArea.extent.height = m_Height;
				BeginInfo.clearValueCount = 1;
				BeginInfo.pClearValues = &m_VkClearValue;

				ClearOnAcquireCommand.beginRenderPass(BeginInfo, vk::SubpassContents::eInline, VkDispatch);

				ClearOnAcquireCommand.endRenderPass();
				

				/*vk::ImageMemoryBarrier ImageBarrier{};
				ImageBarrier.pNext = nullptr;
				ImageBarrier.image = m_VkImages[i];
				ImageBarrier.subresourceRange = AllSubresources;
				ImageBarrier.oldLayout = vk::ImageLayout::eUndefined;
				ImageBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
				ImageBarrier.srcAccessMask = {};
				ImageBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
				ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				ClearOnAcquireCommand.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, ImageBarrier, VkDispatch);

				ClearOnAcquireCommand.clearColorImage(m_VkImages[i], vk::ImageLayout::eTransferDstOptimal, m_VkClearColor, AllSubresources, VkDispatch);

				ImageBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
				ImageBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
				ImageBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
				ImageBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;

				ClearOnAcquireCommand.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {}, nullptr, nullptr, ImageBarrier, VkDispatch);*/

				ClearOnAcquireCommand.end(VkDispatch);
			}

			//m_pRenderDevice->CreateTextureFromVkImage(TexCI, m_VkImages[m_TextureCount], &m_FrameTextures[m_TextureCount]);
		}

		InitialClearCommandBuffer.end();

		vk::SubmitInfo Submit{};
		Submit.pNext = nullptr;
		Submit.commandBufferCount = 1;
		Submit.pCommandBuffers = &InitialClearCommandBuffer;
		Submit.waitSemaphoreCount = 0;
		Submit.pWaitSemaphores = nullptr;
		Submit.pWaitDstStageMask = nullptr;
		Submit.signalSemaphoreCount = 0;
		Submit.pSignalSemaphores = nullptr;
		
		vk::FenceCreateInfo FenceCI{};
		FenceCI.flags = {};
		FenceCI.pNext = nullptr;

		vk::Fence Fence = VkDevice.createFence(FenceCI, nullptr, VkDispatch);

		m_pVulkanDevice->VkQueueSubmit(m_GraphicsQueue, Submit, Fence);

		VkDevice.waitForFences(Fence, true, UINT64_MAX, VkDispatch);

		VkDevice.destroyFence(Fence, nullptr, VkDispatch);

		VkDevice.freeCommandBuffers(m_CmdPool, InitialClearCommandBuffer, VkDispatch);

		m_SemaphoreIndex = m_TextureCount - 1;

	}

	void VulkanSwapChain::WaitForImageAcquiredFences()
	{
		auto& VkDevice = m_pVulkanDevice->GetVkDevice();
		auto& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		for (auto Fence : m_ImageAcquiredFences)
		{
			if (VkDevice.getFenceStatus(Fence, VkDispatch) == vk::Result::eNotReady)
				VkDevice.waitForFences(1, &Fence, true, UINT64_MAX, VkDispatch);
		}
	}

	void VulkanSwapChain::ReleaseSwapChainResources(bool bDestroySwapChain)
	{
		if (!m_VkSwapchain)
			return;

		auto& VkDevice = m_pVulkanDevice->GetVkDevice();
		auto& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

		m_pVulkanDevice->WaitIdle();

		WaitForImageAcquiredFences();

		/*for (auto Texture : m_FrameTextures)
		{
			Texture->Release();
		}*/

		if (m_Flags & SwapChainCreationFlagBits::eClearOnAcquire)
		{

			for (auto Cmd : m_ClearOnAcquireCommands)
			{
				VkDevice.freeCommandBuffers(m_CmdPool, 1, &Cmd, VkDispatch);
			}

			m_ClearOnAcquireCommands.clear();

			for (auto Framebuffer : m_ClearOnAcquireFramebuffers)
			{
				VkDevice.destroyFramebuffer(Framebuffer, nullptr, VkDispatch);
			}

			m_ClearOnAcquireFramebuffers.clear();

			for (auto Renderpass : m_ClearOnAcquireRenderPasses)
			{
				VkDevice.destroyRenderPass(Renderpass, nullptr, VkDispatch);
			}

			m_ClearOnAcquireRenderPasses.clear();

		}

		for (auto Fence : m_ImageAcquiredFences)
		{
			VkDevice.destroyFence(Fence, nullptr, VkDispatch);
		}

		for (auto Semaphore : m_ImageAcquiredSemaphores)
		{
			VkDevice.destroySemaphore(Semaphore, nullptr, VkDispatch);
		}

		for (auto Semaphore : m_SubmitCompleteSemaphores)
		{
			VkDevice.destroySemaphore(Semaphore, nullptr, VkDispatch);
		}

		// m_FrameTextures.clear();
		m_bImageAcquired.clear();
		m_ImageAcquiredFences.clear();
		m_ImageAcquiredSemaphores.clear();
		m_SubmitCompleteSemaphores.clear();

		m_SemaphoreIndex = 0;

		if (bDestroySwapChain)
		{
			VkDevice.destroySwapchainKHR(m_VkSwapchain, nullptr, VkDispatch);
		}
	}

	void VulkanSwapChain::RecreateSwapChain()
	{
		ReleaseSwapChainResources(false);

		// Check if the surface is lost
		{
			auto& VkPhDevice = m_pVulkanDevice->GetVkPhDevice();
			auto& VkDevice =   m_pVulkanDevice->GetVkDevice();
			auto& VkDispatch = m_pVulkanDevice->GetVkDeviceDispatch();

			try
			{
				vk::SurfaceCapabilitiesKHR SurfCapabilities = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, VkDispatch);
				(void)SurfCapabilities;
			}
			catch (const vk::SurfaceLostKHRError&)
			{
				if (m_VkSwapchain)
				{
					VkDevice.destroySwapchainKHR(m_VkSwapchain, nullptr, VkDispatch);
				}

				// Recreate the surface
				CreateSurface();
			}
			catch (const vk::SystemError& Error)
			{
				QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
			}
		}

		CreateSwapChain();
	}
}