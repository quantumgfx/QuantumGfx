#pragma once

#include "GraphicsTypes.hpp"
#include "IObject.hpp"

namespace Qgfx
{
    struct SwapChainDesc
    {
        uint32_t Width = 0;
        uint32_t Height = 0;

        uint32_t BufferCount = 3;

        TextureFormat ColorBufferFormat = TextureFormat::RGBA8UnormSrgb;
        TextureFormat DepthBufferFormat = TextureFormat::D32Float;

        SurfaceTransform PreTransform = SurfaceTransform::Optimal;
    };

	class ISwapChain : public IObject
	{
	public:

        ISwapChain(RefCounter* pRefCounter)
            : IObject(pRefCounter)
        {
        }

        virtual ~ISwapChain() = default;

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform = SurfaceTransform::Optimal) = 0;
	};
}