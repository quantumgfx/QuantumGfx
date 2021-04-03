#include "Qgfx/Graphics/Vulkan/InstanceVk.hpp"
#include "Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp"
#include "Qgfx/Common/Error.hpp"

#include <vector>

namespace Qgfx
{
	InstanceVk::InstanceVk(RefCounter* pRefCounter, const InstanceVkCreateInfo& CreateInfo)
		: IInstance(pRefCounter)
	{
		m_Loader = vkq::Loader::create(CreateInfo.pfnLoaderHandle);

		std::vector<const char*> EnabledExtensions{};
		std::vector<const char*> EnabledLayers{};

		EnabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		EnabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

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

		m_Instance = vkq::InstanceFactory(m_Loader)
			.setAppName(CreateInfo.AppName).setAppVersion(CreateInfo.AppVersion)
			.setEngineName(CreateInfo.EngineName).setEngineVersion(CreateInfo.EngineVersion)
			.enableExtensions(EnabledExtensions)
			.enableLayers(EnabledLayers)
			.requireApiVersion(1, 2, 0)
			.build();
	}
	
	InstanceVk::~InstanceVk()
	{
		m_Instance.destroy();
		m_Loader.destroy();
	}

	void InstanceVk::CreateRenderDevice(IRenderDevice** ppDevice)
	{
		RenderDeviceVk* pRenderDevice = MakeRefCountedObj<RenderDeviceVk>()(this);
		*ppDevice = pRenderDevice;
	}

	
	void CreateInstanceVk(const InstanceVkCreateInfo& CreateInfo, InstanceVk** ppInstance)
	{
		*ppInstance = MakeRefCountedObj<InstanceVk>()(CreateInfo);
	}

}