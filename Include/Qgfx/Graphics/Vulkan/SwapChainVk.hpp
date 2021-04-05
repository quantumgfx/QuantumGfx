#pragma once

#include "../ISwapChain.hpp"

#include "../../Platform/NativeWindow.hpp"

#include "RenderDeviceVk.hpp"
#include "RenderContextVk.hpp"

namespace Qgfx
{
	class SwapChainVk final : public ISwapChain
	{
	public:

		SwapChainVk(RefCounter* pRefCounter, const SwapChainDesc& Desc, const NativeWindow& Window, RenderContextVk* pRenderContext, RenderDeviceVk* pRenderDevice);

		~SwapChainVk();

	private:

		vk::SurfaceKHR m_Surface;
	};
}