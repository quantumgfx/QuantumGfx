#pragma once

#include "BaseVk.hpp"

#include "../IShaderModule.hpp"

namespace Qgfx
{
	class ShaderModuleVk final : public IShaderModule
	{
	public:

		ShaderModuleVk(IRenderDevice* pRenderDevice);

		~ShaderModuleVk();

	private:

		vk::ShaderModule m_VkShaderModule;
	};
}