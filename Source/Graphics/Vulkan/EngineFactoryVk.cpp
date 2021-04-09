#include "Qgfx/Graphics/Vulkan/EngineFactoryVk.hpp"
#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"
#include "Qgfx/Graphics/Vulkan/RenderContextVk.hpp"
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
		m_Loader = vkq::Loader::create(CreateInfo.pfnLoaderHandle);

		std::vector<const char*> EnabledExtensions{};
		std::vector<const char*> EnabledLayers{};

		EnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef QGFX_PLATFORM_WIN32
		EnabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		if (CreateInfo.bEnableValidation)
		{
			if (m_Loader.isLayerSupported("VK_LAYER_KHRONOS_validation"))
			{
				EnabledLayers.push_back("VK_LAYER_KHRONOS_validation");
			}
			else
			{
				QGFX_LOG_ERROR("Validation enabled but VK_LAYER_KHRONOS_validation is not available. Make sure the proper Vulkan SDK is installed, and the appropriate pfnLoaderHandle provided.");
			}

			if (m_Loader.isInstanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
			{
				EnabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			else
			{
				QGFX_LOG_ERROR("Validation enabled but VK_EXT_DEBUG_UTILS_EXTENSION not supported.");
			}
		}

		try
		{
			m_Instance = vkq::InstanceFactory(m_Loader)
				.setAppName(CreateInfo.AppName).setAppVersion(CreateInfo.AppVersion)
				.setEngineName(CreateInfo.EngineName).setEngineVersion(CreateInfo.EngineVersion)
				.enableExtensions(EnabledExtensions)
				.enableLayers(EnabledLayers)
				.requireApiVersion(1, 2, 0)
				.build();
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vkCreateInstance failed with error: ", Error.what());
		}
	}
	
	EngineFactoryVk::~EngineFactoryVk()
	{
		m_Instance.destroy();
		m_Loader.destroy();
	}

	void EngineFactoryVk::CreateRenderDevice(const RenderDeviceCreateInfoVk& DeviceCreateInfo, uint32_t* pNumSupportedQueues, IRenderDevice** ppDevice)
	{
		RenderDeviceVk* pRenderDevice = MakeRefCountedObj<RenderDeviceVk>()(this, DeviceCreateInfo);
		if(pNumSupportedQueues)
			*pNumSupportedQueues = pRenderDevice->GetNumSupportedQueues();
		*ppDevice = pRenderDevice;
	}

	void EngineFactoryVk::CreateRenderContext(IRenderDevice* pDevice, uint32_t QueueIndex, IRenderContext** ppContext)
	{
		RenderContextVk* pRenderContext = MakeRefCountedObj<RenderContextVk>()(pDevice, QueueIndex);
		*ppContext = pRenderContext;
	}

}