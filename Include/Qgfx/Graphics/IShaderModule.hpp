#pragma once

#include <cstdint>

namespace Qgfx
{

	struct ShaderModuleCreateInfo
	{
		size_t Size;
		const void* Code;
	};

	class IShaderModule
	{
	public:
	};
}