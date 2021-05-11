#pragma once

#include "Forward.hpp"

#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	enum class CommandBufferState
	{
		eRecording = 0,
		eReady,
		eExecuting,
	};

	struct CommandBufferDeleter
	{
		void operator()(ICommandBuffer* pCommandBuffer);
	};

	class ICommandBuffer : public IRefCountedObject<ICommandBuffer, CommandBufferDeleter>
	{
	public:

		friend struct CommandBufferDeleter;

		virtual ~ICommandBuffer();

		virtual void Finish() = 0;

		virtual void Destroy() = 0;

		inline CommandBufferState GetCurrentState() { return m_CurrentState; }

	protected:

		ICommandBuffer(ICommandQueue* pCommandQueue);

		ICommandQueue* m_pCommandQueue;

		CommandBufferState m_CurrentState = CommandBufferState::eRecording;
	};
}