#pragma once

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	struct TextureCreateInfo
	{
        TextureFormat Format = TextureFormat::RGBA8Unorm;

        ImageMemoryType MemoryType = ImageMemoryType::Immutable;

        CPUAccessFlags CPUAccess = CPUAccessFlagBits::None;

		uint32_t Width = 0;
		uint32_t Height = 0;

		union
		{
			uint32_t ArraySize = 1;
			uint32_t Depth = 1;
		};

		uint32_t MipLevels = 1;

		uint32_t SampleCount = 1;

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

	class ITexture
	{
	public:

	};
}