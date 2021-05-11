#include "Qgfx/Graphics/ICommandQueue.hpp"
#include "Qgfx/Graphics/IEngineFactory.hpp"

namespace Qgfx
{
	ICommandQueue::ICommandQueue(IEngineFactory* pEngineFactory, CommandQueueType Type)
		: m_pEngineFactory(pEngineFactory), m_Type(Type)
	{
		m_pEngineFactory->AddRef();
	}

	ICommandQueue::~ICommandQueue()
	{
		m_pEngineFactory->Release();
	}

}