#include "Qgfx/Graphics/ISwapChain.hpp"
#include "Qgfx/Graphics/IRenderDevice.hpp"

namespace Qgfx
{
	ISwapChain::ISwapChain(IRenderDevice* pRenderDevice)
		: m_pRenderDevice(pRenderDevice)
	{
		m_pRenderDevice->AddRef();
	}

	ISwapChain::~ISwapChain()
	{
		m_pRenderDevice->Release();
	}
}