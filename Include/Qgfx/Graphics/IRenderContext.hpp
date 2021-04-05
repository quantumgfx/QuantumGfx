#pragma once

#include "GraphicsTypes.hpp"

#include "IObject.hpp"

#include "ISwapChain.hpp"

namespace Qgfx
{
	class IRenderContext : public IObject
	{
	public:

		enum class Type
		{
			Universal,
			AsyncCompute,
			AsyncTransfer,
		};

		IRenderContext(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IRenderContext() = default;

		/**
		 * @brief Gets what type this render context is (ie, what sort of operations it supports).
		 * @return The type of the render context.
		*/
		virtual Type GetType() = 0;

		virtual void InvalidateState() = 0;

		virtual void Flush() = 0;

		virtual void WaitIdle() = 0;

		virtual void Present(ISwapChain* pSwapChain) = 0;

	};
}