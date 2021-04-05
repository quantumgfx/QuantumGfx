#pragma once

#include <cstdint>
#include <type_traits>

#include "Forward.hpp"

#include "../Common/FlagsEnum.hpp"
#include "../Platform/Atomics.hpp"
#include "../Platform/NativeWindow.hpp"

namespace Qgfx
{
    ////////////////////////////
    // Enums ///////////////////
    ////////////////////////////

    enum class GraphicsInstanceType
    {
        Vulkan = 1,
    };

    enum class CPUAccessFlagBits
    {
        None = 0,
        Read = 0x1,
        Write = 0x2
    };

    template<>
    struct EnableEnumFlags<CPUAccessFlagBits>
    {
        static const bool bEnabled = true;
    };

    using CPUAccessFlags = Flags<CPUAccessFlagBits>;

    enum class BufferState
    {
        Vertex = 0,
        Uniform,
        Index,
        Indirect,
        ShaderResource,
        UnorderedAccess,
        TransferDst,
        TransferSrc
    };

    enum class BufferUsageFlagBits
    {
        None = 0x00,
        Vertex = 0x01,
        Uniform = 0x02,
        Index = 0x04,
        Indirect = 0x08,
        ShaderResource = 0x10,
        UnorderedAccess = 0x20,
    };

    template<>
    struct EnableEnumFlags<BufferUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using BufferUsageFlags = Flags<BufferUsageFlagBits>;


    enum class BufferMemoryType
    {
        /**
         * @brief A resource that can only be read by the GPU. It cannot be written by the GPU, and can not be accessed at all by the CPU. \n
         * Static buffers do not allow CPU access and must use CPUAccess::None.
        */
        Immutable = 0,
        Default,
        Dynamic,
        Staging,
        Unified,
    };

    enum class ImageState
    {
        RenderTarget,
        DepthReadOnly,
        Depth,
        ShaderResource,
        UnorderedAccess,
        TransferDst,
        TransferSrc,
        Present,
    };

    enum class TextureFormat
    {
        RGBA32Float = 1,
        RGBA32Uint,
        RGBA32Sint,

        RGBA16Float,
        RGBA16Uint,
        RGBA16Sint,

        RG32Float,
        RG32Uint,
        RG32Sint,

        RGB10A2Unorm,
        RGB10A2Uint,

        R11G11B10Float,

        RGBA8Unorm,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        /**
         * @brief Essentially identical to RGBA8Unorm, 
         * except this format has more percision near 0.
        */
        RGBA8UnormSrgb,

        RG16Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,

        R32Float,
        R32Uint,
        R32Sint,

        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,

        R16Float,
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,

        R8Unorm,
        R8Snorm,
        R8Uint,
        R8Sint,

        D32Float,
        D32FloatS8Uint,
        D24UnormS8Uint,
        D16Unorm,
        D16UnormS8Uint,
    };

    enum class SwapChainUsageFlagBits
    {
        None = 0x00,
        RenderTarget = 0x01,
    };

    /// The transform applied to the image content prior to presentation.
    enum class SurfaceTransform
    {
        /// Uset the most optimal surface transform.
        Optimal = 0,

        /// The image content is presented without being transformed.
        Identity,

        /// The image content is rotated 90 degrees clockwise.
        Rotate90,

        /// The image content is rotated 180 degrees clockwise.
        Rotate180,

        /// The image content is rotated 270 degrees clockwise.
        Rotate270,

        /// The image content is mirrored horizontally.
        HorizontalMirror,

        /// The image content is mirrored horizontally, then rotated 90 degrees clockwise.
        HorizontalMirrorRotate90,

        /// The  image content is mirrored horizontally, then rotated 180 degrees clockwise.
        HorizontalMirrorRotate180,

        /// The  image content is mirrored horizontally, then rotated 270 degrees clockwise.
        HorizontalMirrorRotate270,
    };

    enum class Filter
    {
        Nearest = 0,
        Linear,
    };

    enum class MipMode
    {
        Nearest = 0,
        Linear,
    };

    /**
     * @brief Defines a comparison operation.
    */
    enum class CompareOp
    {
        Never = 0,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always,
    };

    /**
     * @brief Defines the technique used to resolve texture coordinates that outside of the boundaries of a texture.
    */
    enum class TextureAddressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
    };

    /////////////////////////
    // Device structs ///////
    /////////////////////////

    enum class DeviceFeatureState
    {
        Disabled = 0,
        Enabled,
        Optional
    };

    struct DeviceFeatures
    {
        DeviceFeatureState ComputeShaders = DeviceFeatureState::Disabled;
        DeviceFeatureState TesselationShaders = DeviceFeatureState::Disabled;
        DeviceFeatureState GeometryShaders = DeviceFeatureState::Disabled;
        DeviceFeatureState IndirectRendering = DeviceFeatureState::Disabled;
        DeviceFeatureState WireFrameFill = DeviceFeatureState::Disabled;
    };

    struct APIInfo
    {
        uint32_t word;
    };

    struct RenderDeviceCreateInfo
    {
        DeviceFeatures Features;
    };
}