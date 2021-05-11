#pragma once

#include "Forward.hpp"
#include "GraphicsTypes.hpp"
#include "ISwapChain.hpp"
#include "ICommandBuffer.hpp"

#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	enum class CommandQueueType
	{
		/**
		 * @brief General queues support presentation, graphics commands, compute commands, and transfer commands.
		*/
		eGeneral = 0,
		/**
		 * @brief Compute queues support compute commands and transfer commands
		*/
		eCompute,
		/**
		 * @brief Transfer queues only support transfer commands
		*/
		eTransfer,
	};

	class ICommandQueue : public IRefCountedObject<ICommandQueue>
	{
		friend struct CommandBufferDeleter;

	public:

		inline CommandQueueType GetType() const { return m_Type; }

		virtual ICommandBuffer* CreateCommandBuffer() = 0;

		virtual void SubmitCommandBuffers(uint32_t NumCommandBuffers, ICommandBuffer** ppCommandBuffers) = 0;

		virtual void WaitIdle() = 0;

		virtual void Destroy() = 0;

	protected:

		ICommandQueue(IEngineFactory* pEngineFactory, CommandQueueType Type);

		virtual ~ICommandQueue();

		virtual void DeleteCommandBuffer(ICommandBuffer* pCommandBuffer) = 0;

	protected:

		IEngineFactory* m_pEngineFactory;

		CommandQueueType m_Type;
	};
}