#pragma once

#include "Forward.hpp"
#include "GraphicsTypes.hpp"
#include "IObject.hpp"
#include "ISwapChain.hpp"
#include "ICommandBuffer.hpp"

namespace Qgfx
{

	class ICommandQueue : public IObject
	{
	public:

		ICommandQueue(IRefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~ICommandQueue() = default;

		virtual CommandQueueType GetType() const = 0;

		virtual void CreateCommandBuffer(ICommandBuffer** ppCommandBuffer) = 0;

		/**
		 * @brief Once all current pending work on the queue is completed, it will signal this fence to value.
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

		virtual void SubmitCommandBuffers(uint32_t NumCommandBuffers, ICommandBuffer** ppCommandBuffers) = 0;
	};
}