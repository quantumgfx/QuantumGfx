#include "Qgfx/Graphics/Vulkan/EngineFactoryVk.hpp"
#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"
#include "Qgfx/Common/Error.hpp"

#include <vector>

namespace Qgfx
{
	void CreateEngineFactoryVk(const EngineFactoryCreateInfoVk& CreateInfo, EngineFactoryVk** ppEngineFactory)
	{
		*ppEngineFactory = MakeRefCountedObj<EngineFactoryVk>()(CreateInfo);
	}

	EngineFactoryVk::EngineFactoryVk(RefCounter* pRefCounter, const EngineFactoryCreateInfoVk& CreateInfo)
		: IEngineFactory(pRefCounter)
	{

		m_VkDispatch.init(CreateInfo.pfnLoaderHandle);

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


		if (CreateInfo.bEnableValidation)
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
		AppInfo.applicationVersion = CreateInfo.AppVersion;
		AppInfo.pApplicationName = CreateInfo.AppName;
		AppInfo.engineVersion = CreateInfo.EngineVersion;
		AppInfo.pEngineName = CreateInfo.EngineName;

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
	}
	
	EngineFactoryVk::~EngineFactoryVk()
	{
		m_VkInstance.destroy(nullptr, m_VkDispatch);
	}

	void EngineFactoryVk::CreateRenderDevice(const RenderDeviceCreateInfoVk& DeviceCreateInfo, uint32_t* pNumSupportedQueues, IRenderDevice** ppDevice)
	{
		RenderDeviceVk* pRenderDevice = MakeRefCountedObj<RenderDeviceVk>()(this, DeviceCreateInfo);
		if(pNumSupportedQueues)
			*pNumSupportedQueues = pRenderDevice->GetNumSupportedQueues();
		*ppDevice = pRenderDevice;
	}

	void EngineFactoryVk::CreateCommandQueue(IRenderDevice* pDevice, uint32_t QueueIndex, ICommandQueue** ppQueue)
	{
		CommandQueueVk* pCommandQueue = MakeRefCountedObj<CommandQueueVk>()(pDevice, QueueIndex);
		*ppQueue = pCommandQueue;
	}

}