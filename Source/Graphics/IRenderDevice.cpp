#include "Qgfx/Graphics/IRenderDevice.hpp"
#include "Qgfx/Graphics/IEngineFactory.hpp"

namespace Qgfx
{

	IRenderDevice::IRenderDevice(IEngineFactory* pEngineFactory)
		: m_pEngineFactory(pEngineFactory)
	{
		m_pEngineFactory->AddRef();
	}

	IRenderDevice::~IRenderDevice()
	{
		m_pEngineFactory->Release();
	}

}