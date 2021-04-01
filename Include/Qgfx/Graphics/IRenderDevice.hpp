#pragma once

#include "IInstance.hpp"

namespace Qgfx
{

	class IRenderDevice
	{
	public:

		virtual ~IRenderDevice() = default;

		virtual void WaitIdle() = 0;

	};
}