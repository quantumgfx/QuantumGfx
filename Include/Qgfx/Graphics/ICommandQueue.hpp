#pragma once

#include "GraphicsTypes.hpp"

#include "IObject.hpp"

#include "ISwapChain.hpp"

namespace Qgfx
{

	class ICommandQueue : public IObject
	{
	public:

		ICommandQueue(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~ICommandQueue() = default;

		virtual CommandQueueType GetType() const = 0;

		/**
		 * @brief Once all current pending work on the gpu is completed, it will signal this fence to value.
		 * @param pFence 
		 * @param Value 
		*/
		virtual void SignalFence(IFence* pFence, uint64_t Value) = 0;

		/**
		 * @brief This call ensures that before any other commands are submitted on this queue, the given fence must reach the specified value.
		 * @param pFence 
		 * @param Value 
		*/
		virtual void WaitForFence(IFence* pFence, uint64_t Value) = 0;

		virtual void WaitIdle() = 0;

		virtual void Present(uint32_t NumSwapChains, ISwapChain** ppSwapChains) = 0;
	};
}