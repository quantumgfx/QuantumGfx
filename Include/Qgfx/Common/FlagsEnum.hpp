#pragma once

#include <cstdint>
#include <type_traits>


namespace Qgfx
{
    template<typename BitType>
    struct EnableEnumFlags
    {
        static const bool bEnabled = false;
    };

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

#define ENABLE_IF_FLAG_OPERATOR(Type) typename std::enable_if<EnableEnumFlags<BitType>::bEnable, Type>::type

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator<(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator>(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator<=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator>=(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator>(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator<(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator>=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator<=(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator==(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator==(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(bool) operator!=(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator!=(bit);
    }

    // bitwise operators
    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator&(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator&(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator|(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator|(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator^(BitType bit, Flags<BitType> const& flags) noexcept
    {
        return flags.operator^(bit);
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator|(BitType bit0, BitType bit1) noexcept
    {
        return Flags<BitType>(bit0) | bit1;
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator&(BitType bit0, BitType bit1) noexcept
    {
        return Flags<BitType>(bit0) & bit1;
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator^(BitType bit0, BitType bit1) noexcept
    {
        return Flags<BitType>(bit0) ^ bit1;
    }

    template <typename BitType>
    constexpr ENABLE_IF_FLAG_OPERATOR(Flags<BitType>) operator~(BitType bits) noexcept
    {
        return ~(Flags<BitType>(bits));
    }

#undef ENABLE_IF_FLAG_OPERATOR
}