#pragma once

#include <cstdint>

namespace Qgfx
{
	class IFence
	{
	public:

		virtual uint64_t GetCounterValue() = 0;
	};
}