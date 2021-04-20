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
        None = 0x000,
        Vertex = 0x001,
        Uniform = 0x002,
        Index = 0x004,
        Indirect = 0x008,
        ShaderResource = 0x010,
        UnorderedAccess = 0x020,
        TransferSrc = 0x040,
        TransferDst = 0x080,
        MapRead = 0x100,
        MapWrite = 0x100
    };

    template<>
    struct EnableEnumFlags<BufferUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using BufferUsageFlags = Flags<BufferUsageFlagBits>;

    enum class ImageMemoryType
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
        RenderAttachment,
        DepthReadOnlyAttachment,
        DepthAttachment,
        ShaderResource,
        UnorderedAccess,
        TransferDst,
        TransferSrc,
        Present,
    };

    enum class TextureFormat
    {
        // 8-bit formats
        R8Unorm = 1,
        R8Snorm,
        R8Uint,
        R8Sint,

        // 16-bit formats
        R16Float,
        R16Unorm,
        R16Snorm,
        R16Uint,
        R16Sint,

        RG8Unorm,
        RG8Snorm,
        RG8Uint,
        RG8Sint,

        // 32-bit formats
        R32Float,
        R32Uint,
        R32Sint,

        RG16Float,
        RG16Unorm,
        RG16Snorm,
        RG16Uint,
        RG16Sint,
        RGBA8Unorm,
        RGBA8Snorm,
        RGBA8Uint,
        RGBA8Sint,
        RGBA8UnormSrgb,

        BGRA8Unorm,
        BGRA8UnormSrgb,
        // Packed 32-bit formats
        RGB10A2Unorm,
        RGB10A2Uint,
        R11G11B10Float,

        // 64-bit formats
        RG32Float,
        RG32Uint,
        RG32Sint,

        RGBA16Float,
        RGBA16Uint,
        RGBA16Sint,

        // 128-bit formats
        RGBA32Float,
        RGBA32Uint,
        RGBA32Sint,
       
        // Depth and stencil formats
        Stencil8Uint,            // Might be implemented as real Stencil8Uint or Depth24PlusStencil8Uint if that is not available.
        Depth16Unorm, 
        Depth24Plus,             // Might be implemented as Depth24Unorm or Depth32Float
        Depth24PlusStencil8Uint, // Might be implemented as Depth24UnormStencil8Uint or Depth32FloatStencil8Uint32
        Depth32Float,
        // Optional depth and stencil formats
        Depth16UnormStencil8Uint,
        Depth32FloatStencil8Uint,
    };

    enum class SwapChainUsageFlagBits
    {
        None = 0x00,
        RenderTarget = 0x01,
        ShaderInput = 0x02,
        TransferSrc = 0x04,
        TransferDst = 0x08
    };

    using SwapChainUsageFlags = Flags<SwapChainUsageFlagBits>;

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

    enum class CommandQueueType
    {
        /**
         * @brief General queues support presentation, graphics commands, compute commands, and transfer commands.
        */
        General = 0,
        /**
         * @brief Compute queues support compute commands and transfer commands
        */
        Compute,
        /**
         * @brief Transfer queues only support transfer commands
        */
        Transfer,
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