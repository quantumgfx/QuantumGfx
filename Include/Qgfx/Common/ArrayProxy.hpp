#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include "Error.hpp"

namespace Qgfx
{
    template <typename T>
    class ArrayProxy
    {
    public:
        constexpr ArrayProxy() noexcept
            : m_count(0)
            , m_ptr(nullptr)
        {}

        constexpr ArrayProxy(std::nullptr_t) noexcept
            : m_count(0)
            , m_ptr(nullptr)
        {}

        ArrayProxy(T& value) noexcept
            : m_count(1)
            , m_ptr(&value)
        {}

        template <typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(typename std::remove_const<T>::type& value) noexcept
            : m_count(1)
            , m_ptr(&value)
        {}

        ArrayProxy(uint32_t count, T* ptr) noexcept
            : m_count(count)
            , m_ptr(ptr)
        {}

        template <typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(uint32_t count, typename std::remove_const<T>::type* ptr) noexcept
            : m_count(count)
            , m_ptr(ptr)
        {}

#if __GNUC__ >= 9
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winit-list-lifetime"
#endif

        ArrayProxy(std::initializer_list<T> const& list) noexcept
            : m_count(static_cast<uint32_t>(list.size()))
            , m_ptr(list.begin())
        {}

        template <typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(std::initializer_list<typename std::remove_const<T>::type> const& list) noexcept
            : m_count(static_cast<uint32_t>(list.size()))
            , m_ptr(list.begin())
        {}

        ArrayProxy(std::initializer_list<T>& list) noexcept
            : m_count(static_cast<uint32_t>(list.size()))
            , m_ptr(list.begin())
        {}

        template <typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(std::initializer_list<typename std::remove_const<T>::type>& list) noexcept
            : m_count(static_cast<uint32_t>(list.size()))
            , m_ptr(list.begin())
        {}

#if __GNUC__ >= 9
#pragma GCC diagnostic pop
#endif

        template <size_t N>
        ArrayProxy(std::array<T, N> const& data) noexcept
            : m_count(N)
            , m_ptr(data.data())
        {}

        template <size_t N, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(std::array<typename std::remove_const<T>::type, N> const& data) noexcept
            : m_count(N)
            , m_ptr(data.data())
        {}

        template <size_t N>
        ArrayProxy(std::array<T, N>& data) noexcept
            : m_count(N)
            , m_ptr(data.data())
        {}

        template <size_t N, typename B = T, typename std::enable_if<std::is_const<B>::value, int>::type = 0>
        ArrayProxy(std::array<typename std::remove_const<T>::type, N>& data) noexcept
            : m_count(N)
            , m_ptr(data.data())
        {}

        template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
        ArrayProxy(std::vector<T, Allocator> const& data) noexcept
            : m_count(static_cast<uint32_t>(data.size()))
            , m_ptr(data.data())
        {}

        template <class Allocator = std::allocator<typename std::remove_const<T>::type>,
            typename B = T,
            typename std::enable_if<std::is_const<B>::value, int>::type = 0>
            ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator> const& data) noexcept
            : m_count(static_cast<uint32_t>(data.size()))
            , m_ptr(data.data())
        {}

        template <class Allocator = std::allocator<typename std::remove_const<T>::type>>
        ArrayProxy(std::vector<T, Allocator>& data) noexcept
            : m_count(static_cast<uint32_t>(data.size()))
            , m_ptr(data.data())
        {}

        template <class Allocator = std::allocator<typename std::remove_const<T>::type>,
            typename B = T,
            typename std::enable_if<std::is_const<B>::value, int>::type = 0>
            ArrayProxy(std::vector<typename std::remove_const<T>::type, Allocator>& data) noexcept
            : m_count(static_cast<uint32_t>(data.size()))
            , m_ptr(data.data())
        {}

        const T* begin() const noexcept
        {
            return m_ptr;
        }

        const T* end() const noexcept
        {
            return m_ptr + m_count;
        }

        const T& Front() const noexcept
        {
            QGFX_VERIFY_EXPR(m_count && m_ptr);
            return *m_ptr;
        }

        const T& Back() const noexcept
        {
            QGFX_VERIFY_EXPR(m_count && m_ptr);
            return *(m_ptr + m_count - 1);
        }

        bool Empty() const noexcept
        {
            return (m_count == 0);
        }

        uint32_t Size() const noexcept
        {
            return m_count;
        }

        T* Data() const noexcept
        {
            return m_ptr;
        }

    private:
        uint32_t m_count;
        T* m_ptr;
    };
}