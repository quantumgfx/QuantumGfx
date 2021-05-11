#pragma once

#include "../Common/FlagsEnum.hpp"

namespace Qgfx
{
    enum class ResourceMemoryUsage
    {
        eGpuOnly = 0,
        eCpuOnly,
        eGpuToCpu,
        eCpuToGpu
    };

    enum class ResourceStateFlagBits
    {
        eUndefined = 0x0000,

        // Buffer only resource states

        // Read only, allows buffer to be bound as vertex buffer.
        eVertexBuffer = 0x0001,
        // Read only, allows buffer to be bound as uniform buffer.
        eUniformBuffer = 0x0002,
        // Read only, allows buffer to be bound as index buffer.
        eIndexBuffer = 0x0004,
        // Read only, allows buffer to be bound to indirect functions.
        eIndirectBuffer = 0x0008,

        // Texture only resource states

        // Read/Write, allows texture to be drawn to as render target
        eRenderAttachment = 0x0010,
        // Read/Write, allows texture to be used for depth tests with bDepthWrite = true
        eDepthAttachment = 0x0020,
        // Read, allows texture to be used for depth tests with bDepthWrite = false
        eDepthReadOnlyAttachment = 0x0040,
        // Read/Write allows texture to be presented by swapchain.
        ePresent = 0x0080,

        // Resource states either can use

        // Read/Write allows resource to be used in unordered write operations (such as storage textures and buffers).
        eUnorderedWrite = 0x0100,
        eNonFragmentShaderResource = 0x0200,
        eFragmentShaderResource = 0x0400,
        eShaderResource = eNonFragmentShaderResource | eFragmentShaderResource,
        eTransferSrc = 0x0800,
        eTransferDst = 0x1000,
        eCommon = 0x2000,
    };

    template<>
    struct EnableEnumFlags<ResourceStateFlagBits>
    {
        static const bool bEnabled = true;
    };

    using ResourceStateFlags = Flags<ResourceStateFlagBits>;

    enum class ResourceUsageFlagBits
    {
        eNone = 0x00,

        // Buffer only usages

        eVertexBuffer = 0x01,
        eUniformBuffer = 0x02,
        eIndexBuffer = 0x04,
        eIndirectBuffer = 0x08,
        eStorageBuffer = 0x10,

        // Texture only usages

        eRenderAttachment = 0x20,
        eDepthStencilAttachment = 0x40,

        // Resource usages either can have

        eSampledImage = 0x80,
        eStorageImage = 0x100,
        eTransferSrc = 0x200,
        eTransferDst = 0x400,
    };

    template<>
    struct EnableEnumFlags<ResourceUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using ResourceUsageFlags = Flags<ResourceUsageFlagBits>;

    enum class TextureFormat
    {
        // 8-bit formats
        eR8Unorm = 1,
        eR8Snorm,
        eR8Uint,
        eR8Sint,

        // 16-bit formats
        eR16Float,
        eR16Unorm,
        eR16Snorm,
        eR16Uint,
        eR16Sint,

        eRG8Unorm,
        eRG8Snorm,
        eRG8Uint,
        eRG8Sint,

        // 32-bit formats
        eR32Float,
        eR32Uint,
        eR32Sint,

        eRG16Float,
        eRG16Unorm,
        eRG16Snorm,
        eRG16Uint,
        eRG16Sint,
        eRGBA8Unorm,
        eRGBA8Snorm,
        eRGBA8Uint,
        eRGBA8Sint,
        eRGBA8UnormSrgb,

        eBGRA8Unorm,
        eBGRA8UnormSrgb,
        // Packed 32-bit formats
        eRGB10A2Unorm,
        eRGB10A2Uint,
        eR11G11B10Float,

        // 64-bit formats
        eRG32Float,
        eRG32Uint,
        eRG32Sint,

        eRGBA16Float,
        eRGBA16Uint,
        eRGBA16Sint,

        // 128-bit formats
        eRGBA32Float,
        eRGBA32Uint,
        eRGBA32Sint,

        // Depth and stencil formats
        eStencil8Uint,            // Might be implemented as real Stencil8Uint or Depth24PlusStencil8Uint if that is not available.
        eDepth16Unorm,
        eDepth24Plus,             // Might be implemented as Depth24Unorm or Depth32Float
        eDepth24PlusStencil8Uint, // Might be implemented as Depth24UnormStencil8Uint or Depth32FloatStencil8Uint32
        eDepth32Float,
        // Optional depth and stencil formats
        eDepth16UnormStencil8Uint,
        eDepth32FloatStencil8Uint,
    };

    enum class TextureFormatComponentType
    {
        eNone,
        eUnorm,
        // eUnormSrgb,
        eSnorm,
        eUint,
        eSint,
        eFloat,
    };

    struct TextureFormatParams
    {
        uint32_t BlockWidth = 1;
        uint32_t BlockHeight = 1;
        uint32_t BlockStride;

        uint32_t ColorComponentCount = 0;
        TextureFormatComponentType ColorDataType;
        TextureFormatComponentType DepthDataType;
        TextureFormatComponentType StencilDataType;
    };

    enum class TextureDimension
    {
        e1D = 0,
        e2D,
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

    struct TextureDesc
    {
        TextureDimension Dimension = TextureDimension::e2D;
        uint32_t Width = 1;
        uint32_t Height = 1;
        uint32_t DepthOrArraySize = 1;
        ResourceStateFlags InitialState;
        TextureSampleCount SampleCount = TextureSampleCount::e1;
        ResourceMemoryUsage MemoryUsage = ResourceMemoryUsage::eGpuOnly;
        ResourceUsageFlags Usage = ResourceUsageFlagBits::eNone;
        /// Pointer to native texture handle if the texture does not own underlying resource, 
        /// this will be treated as a different object depending on the Api in use and other settings of the texture.
        const void* pNativeHandle = nullptr;
    };

    class ITexture
    {
    public:

    protected:

        ITexture() = default;
        ~ITexture() = default;
    };

    class TextureFormatUtils
    {
    public:

        static bool HasColorAspect(TextureFormat Fmt);

        static bool HasDepthAspect(TextureFormat Fmt);

        static bool HasStencilAspect(TextureFormat Fmt);

        static bool HasDepthStencilAspect(TextureFormat Fmt);

        static TextureFormatParams GetParams(TextureFormat Fmt);

    };
}