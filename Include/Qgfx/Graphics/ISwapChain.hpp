#pragma once

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	class ISwapChain
	{
	public:

		virtual void Present() = 0;

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform = SurfaceTransform::Optimal) = 0;
	};
}