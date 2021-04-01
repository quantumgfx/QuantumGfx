#pragma once

#include <cstdint>
#include <type_traits>

#include "../Platform/Atomics.hpp"
#include "../Platform/NativeWindow.hpp"

#include "APIInfo.hpp"

namespace Qgfx
{

    template <typename BitType>
    class Flags
    {
    public:
        using MaskType = typename std::underlying_type<BitType>::type;

        // constructors
        constexpr Flags() noexcept
            : m_mask(0)
        {}

        constexpr Flags(BitType bit) noexcept
            : m_mask(static_cast<MaskType>(bit))
        {}

        constexpr Flags(Flags<BitType> const& rhs) noexcept = default;

        constexpr explicit Flags(MaskType flags) noexcept
            : m_mask(flags)
        {}

        // relational operators
        //auto operator<=>(Flags<BitType> const&) const = default;
        constexpr bool operator<(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask < rhs.m_mask;
        }

        constexpr bool operator<=(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask <= rhs.m_mask;
        }

        constexpr bool operator>(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask > rhs.m_mask;
        }

        constexpr bool operator>=(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask >= rhs.m_mask;
        }

        constexpr bool operator==(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask == rhs.m_mask;
        }

        constexpr bool operator!=(Flags<BitType> const& rhs) const noexcept
        {
            return m_mask != rhs.m_mask;
        }

        // logical operator
        constexpr bool operator!() const noexcept
        {
            return !m_mask;
        }

        // bitwise operators
        constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept
        {
            return Flags<BitType>(m_mask & rhs.m_mask);
        }

        constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept
        {
            return Flags<BitType>(m_mask | rhs.m_mask);
        }

        constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept
        {
            return Flags<BitType>(m_mask ^ rhs.m_mask);
        }

        constexpr Flags<BitType> operator~() const noexcept
        {
            return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags);
        }

        // assignment operators
        constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept = default;

        constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept
        {
            m_mask |= rhs.m_mask;
            return *this;
        }

        constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept
        {
            m_mask &= rhs.m_mask;
            return *this;
        }

        constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept
        {
            m_mask ^= rhs.m_mask;
            return *this;
        }

        // cast operators
        explicit constexpr operator bool() const noexcept
        {
            return !!m_mask;
        }

        explicit constexpr operator MaskType() const noexcept
        {
            return m_mask;
        }
    private:
        MaskType  m_mask;
    };

    template <typename BitType>
    constexpr bool operator<(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator>(bit);
    }

    template <typename BitType>
    constexpr bool operator<=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator>=(bit);
    }

    template <typename BitType>
    constexpr bool operator>(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator<(bit);
    }

    template <typename BitType>
    constexpr bool operator>=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator<=(bit);
    }

    template <typename BitType>
    constexpr bool operator==(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator==(bit);
    }

    template <typename BitType>
    constexpr bool operator!=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator!=(bit);
    }

    // bitwise operators
    template <typename BitType>
    constexpr Flags<BitType> operator&(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator&(bit);
    }

    template <typename BitType>
    constexpr Flags<BitType> operator|(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator|(bit);
    }

    template <typename BitType>
    constexpr Flags<BitType> operator^(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator^(bit);
    }

    enum class GraphicsInstanceType
    {
        Vulkan = 1,
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

    struct SwapChainDesc
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
        
        uint32_t BufferCount = 3;
        
        TextureFormat eColorBufferFormat = TextureFormat::RGBA8UnormSrgb;
        TextureFormat eDepthBufferFormat = TextureFormat::D32Float;

        SurfaceTransform PreTransform = SurfaceTransform::Optimal;
    };

}