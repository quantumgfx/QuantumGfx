#pragma once

#include "GraphicsTypes.hpp"
#include "ICommandQueue.hpp"
#include "ITexture.hpp"
#include "ITextureView.hpp"

#include "../Common/FlagsEnum.hpp"
#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
    enum class SwapChainUsageFlagBits
    {
        eNone = 0x00,
        eRenderAttachment = 0x01,
        eSampled = 0x02,
        eTransferSrc = 0x04,
        eTransferDst = 0x08
    };

    template<>
    struct EnableEnumFlags<SwapChainUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using SwapChainUsageFlags = Flags<SwapChainUsageFlagBits>;

    /// The transform applied to the image content prior to presentation.
    enum class SurfaceTransform
    {
        /// Uset the most optimal surface transform.
        eOptimal = 0,

        /// The image content is presented without being transformed.
        eIdentity,

        /// The image content is rotated 90 degrees clockwise.
        eRotate90,

        /// The image content is rotated 180 degrees clockwise.
        eRotate180,

        /// The image content is rotated 270 degrees clockwise.
        eRotate270,

        /// The image content is mirrored horizontally.
        eHorizontalMirror,

        /// The image content is mirrored horizontally, then rotated 90 degrees clockwise.
        eHorizontalMirrorRotate90,

        /// The  image content is mirrored horizontally, then rotated 180 degrees clockwise.
        eHorizontalMirrorRotate180,

        /// The  image content is mirrored horizontally, then rotated 270 degrees clockwise.
        eHorizontalMirrorRotate270,
    };

    struct SwapChainCreateInfo
    {

        NativeWindow Window;

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
        uint32_t TextureCount = 3;
        /**
         * @brief Format of swapchain images
        */
        TextureFormat Format = TextureFormat::eRGBA8UnormSrgb;
        /**
         * @brief Desired pre transform to use for the swapchain
        */
        SurfaceTransform PreTransform = SurfaceTransform::eOptimal;
        
        SwapChainUsageFlags Usage = SwapChainUsageFlagBits::eRenderAttachment;

        /**
         * @brief Pointer to queue which will own the swapchain. Must be have type CommandQueueType::General.
        */
        ICommandQueue* pQueue = nullptr;
    };

	class ISwapChain
	{
	public:

        /**
         * @brief Gets the Swap Chain's current width.
         * @return The width of the swapchain.
        */
        inline uint32_t GetWidth() const { return m_Width; }

        /**
         * @brief Gets the Swap Chain's current height.
         * @return The height of the swapchain.
        */
        inline uint32_t GetHeight() const { return m_Height; }

        /**
         * @brief Gets the current number of buffers in the swapchain.
         * @return The number of buffer in the swapchain.
        */
        inline uint32_t GetTextureCount() const { return m_TextureCount; }

        /**
         * @brief Gets the current surface transform of the swapchain.
         * @return The current surface transform of the swapchain.
        */
        inline SurfaceTransform GetSurfaceTransform() const { return m_PreTransform; }

        inline TextureFormat GetTextureFormat() const { return m_TextureFormat; }

		virtual void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform = SurfaceTransform::eOptimal) = 0;

        virtual void AcquireNextTexture() = 0;

        /**
         * @brief This function retrieve's the swapchain's current texture. It will be in state TextureState::ePresent after AcquireNextTexture
         * And must be in TextureState::ePresent before Present is called.
         * @return
        */
        virtual ITexture* GetCurrentTexture() = 0;

        virtual ITextureView* GetCurrentTextureView() = 0;

        virtual void Present() = 0;

        virtual void Destroy() = 0;

    protected:

        TextureFormat m_TextureFormat;
        SwapChainUsageFlags m_Usage;
        uint32_t m_Width;
        uint32_t m_Height;
        bool     m_bIsMinimized = false;
        bool     m_bVSyncEnabled = true;
        uint32_t m_TextureCount;

        SurfaceTransform m_PreTransform;
	};
}