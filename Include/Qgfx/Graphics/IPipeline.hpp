#pragma once

#include "IShaderModule.hpp"

namespace Qgfx
{
	struct ProgrammableStage
	{
		const char* pEntryPoint = "main";
		IShaderModule* pModule = nullptr;
	};
}