#pragma once

#include <cstdint>

namespace Qgfx
{
	class IFence
	{
	public:

		virtual uint64_t GetCounterValue() = 0;

		/**
		 * @brief Signals fence from host (vs from
		 * @param Value 
		*/
		virtual void Signal(uint64_t Value) = 0;

		// virtual void Wait(uint64_t Value) = 0;
	};
}