#pragma once

#include "BaseVk.hpp"
#include "InstanceVk.hpp"

#include "../IInstance.hpp"
#include "../IRenderDevice.hpp"

#include "../../Common/RefAutoPtr.hpp"

namespace Qgfx
{
	class RenderDeviceVk final : public IRenderDevice
	{
	public:

		struct VkFeatureSupport
		{
			bool bTimelineSemaphore = false;
		};

	private:

		RenderDeviceVk(RefCounter* pRefCounter, InstanceVk* pInstance);

		~RenderDeviceVk();

	private:

		RefAutoPtr<InstanceVk> m_spInstance;

		vkq::PhysicalDevice m_PhysicalDevice;
		vkq::Device m_LogicalDevice;


		friend InstanceVk;
	};
}