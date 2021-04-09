#pragma once

#include "GraphicsTypes.hpp"
#include "IObject.hpp"

namespace Qgfx
{
    struct SwapChainCreateInfo
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

        virtual uint32_t GetWidth() = 0;
        virtual uint32_t GetHeight() = 0;
        virtual uint32_t GetBufferCount() = 0;
        virtual SurfaceTransform GetSurfaceTransform() = 0;

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform = SurfaceTransform::Optimal) = 0;
	};
}