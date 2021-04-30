#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"
#include "ITextureView.hpp"

#include "ICommandQueue.hpp"

#include "../Common/FlagsEnum.hpp"

namespace Qgfx
{
    enum class TextureState
    {
        eUndefined = 0,
        eRenderAttachment,
        eDepthReadOnlyAttachment,
        eDepthAttachment,
        eSampled,
        eStorage,
        eTransferSrc,
        eTransferDst,
        ePresent,
    };

    enum class TextureUsageFlagBits
    {
        eNone = 0x00,
        eSampled,
        eStorage,
        eRenderAttachment,
        eDepthStencilAttachment,
        eTransferSrc,
        eTransferDst,
    };

    template<>
    struct EnableEnumFlags<TextureUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using TextureUsageFlags = Flags<TextureUsageFlagBits>;

    enum class TextureDimension
    {
        e1D = 0,
        e2D,
        e3D
    };

    enum class TextureAspectFlagBits
    {
        eNone = 0x00,
        eColor = 0x01,
        eDepth = 0x02,
        eStencil = 0x04,
        eDepthStencil = eDepth | eStencil,
        eAll = eColor | eDepth | eStencil,
    };

    template<>
    struct EnableEnumFlags<TextureAspectFlagBits>
    {
        static const bool bEnabled = true;
    };

    using TextureAspectFlags = Flags<TextureAspectFlagBits>;

    enum class TextureViewDimension
    {
        e1D = 0,
        e1DArray,
        e2D,
        e2DArray,
        eCube,
        eCubeArray,
        e3D
    };

    enum class TextureSampleCount
    {
        e1 = 0,
        e2,
        e4,
        e8,
        e16
    };

	struct TextureCreateInfo
	{
        TextureFormat Format = TextureFormat::eRGBA8Unorm;

        TextureUsageFlags Usage = TextureUsageFlagBits::eNone;

        TextureDimension Dimension = TextureDimension::e1D;

        TextureSampleCount SampleCount = TextureSampleCount::e1;

		uint32_t Width = 0;
		uint32_t Height = 0;

		union
		{
			uint32_t ArraySize;
			uint32_t Depth;
		};

		uint32_t MipLevels = 1;

        ICommandQueue* pInitialQueue = nullptr;

	};

    struct TextureSubResData
    {
        /// Pointer to the subresource data in CPU memory.
        /// If provided, pSrcBuffer must be null
        const void* pData = nullptr;

        /// Pointer to the GPU buffer that contains subresource data.
        /// If provided, pData must be null
        struct IBuffer* pSrcBuffer = nullptr;

        /// When updating data from the buffer (pSrcBuffer is not null),
        /// offset from the beginning of the buffer to the data start
        uint32_t SrcOffset = 0;

        /// For 2D and 3D textures, row stride in bytes
        uint32_t Stride = 0;

        /// For 3D textures, depth slice stride in bytes
        uint32_t DepthStride = 0;
    };

    /// Describes the initial data to store in the texture
    struct TextureData
    {
        /// Pointer to the array of the TextureSubResData elements containing
        /// information about each subresource.
        TextureSubResData* pSubResources = nullptr;

        /// Number of elements in pSubResources array.
        /// NumSubresources must exactly match the number
        /// of subresources in the texture.
        uint32_t NumSubresources = 0;
    };

	class ITexture : public IObject
	{
	public:

        ITexture(IRefCounter* pRefCounter, const TextureCreateInfo& CreateInfo);

        inline TextureFormat GetFormat() const { return m_Format; }

        inline TextureAspectFlags GetAspect()const { return m_Aspect; }

        inline const Extent3D& GetExtent() const { return m_Extent; }

        inline uint32_t GetWidth() const { return m_Extent.Width; }

        inline uint32_t GetHeight() const { return m_Extent.Height; }

        inline uint32_t GetDepth() const { return m_Extent.Depth; }

        inline uint32_t GetNumArrayLayers() const { return m_NumArrayLayers; }

        inline uint32_t GetNumMipLevels() const { return m_NumMipLevels; }

        inline ITextureView* GetDefaultView();

        virtual void CreateView(const TextureViewCreateInfo& CreateInfo, ITextureView** ppView) = 0;

    protected:

        TextureDimension m_Dimension;
        TextureUsageFlags m_Usage;
        TextureAspectFlags m_Aspect;
        TextureFormat m_Format;
        TextureSampleCount m_SampleCount;

        Extent3D m_Extent;
        uint32_t m_NumMipLevels;
        uint32_t m_NumArrayLayers;

	};
}