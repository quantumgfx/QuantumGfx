#pragma once

#include "GraphicsTypes.hpp"

#include "IObject.hpp"

#include "ISwapChain.hpp"

namespace Qgfx
{
	class IRenderContext : public IObject
	{
	public:

		IRenderContext(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IRenderContext() = default;

		/**
		 * @brief Gets what type this render context is (ie, what sort of operations it supports).
		 * @return The type of the render context.
		*/
		virtual RenderContextType GetType() = 0;

		virtual void InvalidateState() = 0;

		virtual void Flush() = 0;

		virtual void WaitIdle() = 0;

		virtual void Present(uint32_t NumSwapChains, ISwapChain** ppSwapChains) = 0;

	};
}