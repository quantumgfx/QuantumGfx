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
        eVulkan = 1,
    };

    //enum class ImageMemoryType
    //{
    //    /**
    //     * @brief A resource that can only be read by the GPU. It cannot be written by the GPU, and can not be accessed at all by the CPU. \n
    //     * Static buffers do not allow CPU access and must use CPUAccess::None.
    //    */
    //    Immutable = 0,
    //    Default,
    //    Dynamic,
    //    Staging,
    //    Unified,
    //};

    enum class CompareFunc
    {
        eNever = 0,
        eLess,
        eEqual,
        eLessEqual,
        eGreater,
        eGreaterEqual,
        eNotEqual,
        eAlways
    };

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

    enum class Filter
    {
        eNearest = 0,
        eLinear,
    };

    enum class MipMode
    {
        eNearest = 0,
        eLinear,
    };

    /**
     * @brief Defines a comparison operation.
    */
    enum class CompareOp
    {
        eNever = 0,
        eLess,
        eEqual,
        eLessOrEqual,
        eGreater,
        eNotEqual,
        eGreaterOrEqual,
        eAlways,
    };

    /**
     * @brief Defines the technique used to resolve texture coordinates that outside of the boundaries of a texture.
    */
    enum class TextureAddressMode
    {
        eWrap,
        eMirror,
        eClamp,
        eBorder,
    };

    enum class CommandQueueType
    {
        /**
         * @brief General queues support presentation, graphics commands, compute commands, and transfer commands.
        */
        eGeneral = 0,
        /**
         * @brief Compute queues support compute commands and transfer commands
        */
        eCompute,
        /**
         * @brief Transfer queues only support transfer commands
        */
        eTransfer,
    };

    /////////////////////////
    // Device structs ///////
    /////////////////////////

    enum class DeviceFeatureState
    {
        eDisabled = 0,
        eEnabled,
        eOptional
    };

    struct DeviceFeatures
    {
        DeviceFeatureState ComputeShaders = DeviceFeatureState::eDisabled;
        DeviceFeatureState TesselationShaders = DeviceFeatureState::eDisabled;
        DeviceFeatureState GeometryShaders = DeviceFeatureState::eDisabled;
        DeviceFeatureState IndirectRendering = DeviceFeatureState::eDisabled;
        DeviceFeatureState WireFrameFill = DeviceFeatureState::eDisabled;
    };

    struct APIInfo
    {
        uint32_t word;
    };

    struct RenderDeviceCreateInfo
    {
        DeviceFeatures Features;
    };

    struct Extent3D
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t Depth;
    };
}