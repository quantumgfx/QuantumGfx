#pragma once

#include "GraphicsTypes.hpp"
#include "IPipeline.hpp"
#include "ITexture.hpp"

namespace Qgfx
{
    enum class VertexFormat
    {
        eUint8x2 = 0,
        eUint8x4,
        eSint8x2,
        eSint8x4,
        eUnorm8x2,
        eUnorm8x4,
        eSnorm8x2,
        eSnorm8x4,
        eUint16x2,
        eUint16x4,
        eSint16x2,
        eSint16x4,
        eFloat16x2,
        eFloat16x4,
        eUint32,
        eUint32x2,
        eUint32x3,
        eUint32x4,
        eSint32,
        eSint32x2,
        eSint32x3,
        eSint32x4,
        eFloat32,
        eFloat32x2,
        eFloat32x3,
        eFloat32x4,
    };

    enum class InputStepMode
    {
        eVertex,
        eInstance,
    };

    enum class StencilOperation
    {
        eKeep = 0,
        eZero,
        eReplace,
        eIncrementClamp,
        eDecrementClamp,
        eInvert,
        eIncremenetWrap,
        eDecrementWrap
    };

    enum class BlendFactor
    {
        eZero = 0,
        eOne,
        eSrc,
        eOneMinusSrc,
        eSrcAlpha,
        eOneMinusSrcAlpha,
        eDst,
        eOneMinusDst,
        eDstAlpha,
        eOneMinusDstAlpha,
        eConstantColor,
        eOneMinusConstantColor,
        eConstantAlpha,
        eOneMinusConstantAlpha,
        eOneMinusConstant,
        eSrcAlphaSaturated
    };

    enum class BlendOperation
    {
        eAdd = 0,
        eSubtract,
        eReverseSubtract,
        eMin,
        eMax,
    };

    enum class ColorWriteFlagBits
    {
        eNone = 0x00,
        eRed = 0x01,
        eGreen = 0x02,
        eBlue = 0x04,
        eAlpha = 0x08,
        eAll = eRed | eGreen | eBlue | eAlpha,  
    };

    template<>
    struct EnableEnumFlags<ColorWriteFlagBits>
    {
        static const bool bEnabled = true;
    };

    using ColorWriteFlags = Flags<ColorWriteFlagBits>;


    struct VertexAttribute
    {
        uint32_t ShaderLocation;
        size_t Offset;
        VertexFormat Format;
    };

    struct VertexBufferLayout
    {
        size_t Stride;
        InputStepMode StepMode;
        uint32_t NumAttributes;
        VertexAttribute* pAttributes;
    };

    struct VertexState : public ProgrammableStage
    {
        uint32_t NumBuffers;
        VertexBufferLayout* pBufferLayouts;
    };

    struct PrimitiveState
    {

    };

    struct MultisampleState
    {
        TextureSampleCount Count = TextureSampleCount::e1;
        uint32_t Mask = 0xffffffff;
        bool bAlphaToConverageEnable = false;
    };

    struct StencilFaceState
    {
        CompareFunc Compare =     CompareFunc::eAlways;
        StencilOperation FailOp = StencilOperation::eKeep;
        StencilOperation DepthFailOp = StencilOperation::eKeep;
        StencilOperation PassOp = StencilOperation::eKeep;
        uint32_t CompareMask = 0xffffffff;
        uint32_t WriteMask =   0xffffffff;
        uint32_t Reference =   0xffffffff;
    };

    struct DepthStencilState
    {
        bool bDepthTestEnable = true;
        bool bDepthWriteEnabled = false;
        CompareFunc DepthCompare = CompareFunc::eAlways;
        
        bool bStencilTestEnable = false;
        StencilFaceState StencilFront = {};
        StencilFaceState StencilBack = {};

        float depthBias = 0;
        float depthBiasSlopeScale = 0;
        float depthBiasClamp = 0;
    };

    struct BlendState
    {
        bool bBlendEnable = true;
        BlendFactor SrcColorFactor = BlendFactor::eOne;
        BlendFactor DstColorFactor = BlendFactor::eZero;
        BlendOperation ColorOp = BlendOperation::eAdd;
        BlendFactor SrcAlphaFactor = BlendFactor::eOne;
        BlendFactor DstAlphaFactor = BlendFactor::eZero;
        BlendOperation AlphaOp = BlendOperation::eAdd;
    };

    struct ColorTargetState
    {
        TextureFormat Format;
        BlendState Blend;
        ColorWriteFlags WriteMask = ColorWriteFlagBits::eAll;
    };
    
    struct FragmentState : public ProgrammableStage
    {
        uint32_t NumTargets;
        ColorTargetState* pTargets;
    };

    struct GraphicsPipelineCreateInfo
    {
        VertexState Vertex = {};
        PrimitiveState Primitive = {};
        DepthStencilState DepthStencil = {};
        MultisampleState Multisample = {};
        FragmentState Fragment = {};
    };
}