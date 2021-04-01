#pragma once

#include "BaseVk.hpp"

#include "../IRenderDevice.hpp"

#include "InstanceVk.hpp"

namespace Qgfx
{
	class RenderDeviceVk final : public IRenderDevice
	{
	public:

	private:

		RenderDeviceVk(InstanceVk Instance);

		~RenderDeviceVk();

	private:

		vkq::PhysicalDevice m_PhysicalDevice;
		vkq::Device m_LogicalDevice;


		friend InstanceVk;
	};
}