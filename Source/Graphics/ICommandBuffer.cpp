#include "Qgfx/Graphics/ICommandBuffer.hpp"
#include "Qgfx/Graphics/ICommandQueue.hpp"

namespace Qgfx
{
	ICommandBuffer::~ICommandBuffer()
	{
	}

	ICommandBuffer::ICommandBuffer(ICommandQueue* pCommandQueue)
		: m_pCommandQueue(pCommandQueue), m_CurrentState(CommandBufferState::eRecording)
	{
		m_pCommandQueue->AddRef();
	}

	void CommandBufferDeleter::operator()(ICommandBuffer* pCommandBuffer)
	{
		ICommandQueue* pCommandQueue = pCommandBuffer->m_pCommandQueue;
		pCommandQueue->DeleteCommandBuffer(pCommandBuffer);
		pCommandQueue->Release();
	}
}