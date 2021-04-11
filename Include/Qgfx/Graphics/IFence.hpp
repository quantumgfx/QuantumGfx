#pragma once

#include <cstdint>

namespace Qgfx
{
	class IFence
	{
	public:

		virtual uint64_t GetCompletedValue() = 0;

		virtual uint64_t GetPendingValue() = 0;

		/**
		 * @brief Signals fence from host (vs from GPU as on ICommandQueue::SignalFence())
		 * @param Value 
		*/
		virtual void Signal(uint64_t Value) = 0;

		/**
		 * @brief Waits on the fence from the host (as opposed to the GPU in ICommandQueue::WaitForFence()
		 * @param Value 
		*/
		virtual void Wait(uint64_t Value) = 0;
	};
}