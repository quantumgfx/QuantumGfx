#pragma once

#include "GraphicsTypes.hpp"
#include "IObject.hpp"
#include "ICommandQueue.hpp"

namespace Qgfx
{
    struct SwapChainCreateInfo
    {
        /**
         * @brief Initial width of the swapchain
        */
        uint32_t Width = 0;
        /**
         * @brief Initial height of the swapchain
        */
        uint32_t Height = 0;
        /**
         * @brief Initial requested buffer count of the swapchain
        */
        uint32_t BufferCount = 3;
        /**
         * @brief Format of swapchain images
        */
        TextureFormat ColorBufferFormat = TextureFormat::RGBA8UnormSrgb;
        /**
         * @brief Format of the swapchain depth buffer
        */
        TextureFormat DepthBufferFormat = TextureFormat::D32Float;
        /**
         * @brief Desired pre transform to use for the swapchain
        */
        SurfaceTransform PreTransform = SurfaceTransform::Optimal;
        
        SwapChainUsageFlags Usage = SwapChainUsageFlagBits::RenderTarget;

        /**
         * @brief Pointer to queue which will own the swapchain. Must be have type CommandQueueType::General.
        */
        ICommandQueue* pQueue = nullptr;
    };

	class ISwapChain : public IObject
	{
	public:

        ISwapChain(RefCounter* pRefCounter)
            : IObject(pRefCounter)
        {
        }

        virtual ~ISwapChain() = default;

        /**
         * @brief Gets the Swap Chain's current width.
         * @return The width of the swapchain.
        */
        virtual uint32_t GetWidth() = 0;

        /**
         * @brief Gets the Swap Chain's current height.
         * @return The height of the swapchain.
        */
        virtual uint32_t GetHeight() = 0;

        /**
         * @brief Gets the current number of buffers in the swapchain.
         * @return The number of buffer in the swapchain.
        */
        virtual uint32_t GetBufferCount() = 0;

        /**
         * @brief Gets the current surface transform of the swapchain.
         * @return The current surface transform of the swapchain.
        */
        virtual SurfaceTransform GetSurfaceTransform() = 0;

        // virtual TextureFormat GetColorBufferFormat() = 0;

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform = SurfaceTransform::Optimal) = 0;

        // virtual void GetCurrentPresentableImage();

        virtual void GetCurrentColorTextureView() = 0;

        virtual void GetCurrentDepthTextureView() = 0;

        virtual void Present() = 0;
	};
}