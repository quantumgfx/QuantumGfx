#include "Qgfx/Graphics/Vulkan/InstanceVk.hpp"

#include <vector>

namespace Qgfx
{
	InstanceVk::InstanceVk(const InstanceVkCreateInfo& CreateInfo)
	{
		m_Loader = vkq::Loader::create(CreateInfo.LoaderHandle);

		std::vector<const char*> EnabledExtensions{};

#if QGFX_PLATFORM_WIN32
		EnabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		m_Instance = vkq::InstanceFactory(m_Loader)
			.setAppName(CreateInfo.AppName).setAppVersion(CreateInfo.AppVersion)
			.setEngineName(CreateInfo.EngineName).setEngineVersion(CreateInfo.EngineVersion)
			.enableExtensions(EnabledExtensions)
			.requireApiVersion(1,2,0)
			.build();
	}
	
	InstanceVk::~InstanceVk()
	{
		m_Instance.destroy();
		m_Loader.destroy();
	}
}