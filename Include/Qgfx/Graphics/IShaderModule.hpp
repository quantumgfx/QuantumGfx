#pragma once

#include <cstdint>

#include "Forward.hpp"

#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	struct ShaderModuleDeleter
	{
		void operator()(IShaderModule* pShaderModule);
	};

	struct ShaderModuleCreateInfo
	{
		size_t Size;
		const void* Code;
	};

	class IShaderModule : public IRefCountedObject<IShaderModule, ShaderModuleDeleter>
	{
	public:

		virtual ~IShaderModule();

	protected:

		IShaderModule(IRenderDevice* pRenderDevice);

	protected:

		friend ShaderModuleDeleter;

		IRenderDevice* m_pRenderDevice;
	};
}