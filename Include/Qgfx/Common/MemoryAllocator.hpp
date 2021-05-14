#pragma once

#include <type_traits>

namespace Qgfx
{
	class IMemoryAllocator
	{
	public:

		/// Allocates block of memory
		virtual void* Allocate(size_t Size) = 0;

		/// Releases memory
		virtual void Free(void* Ptr) = 0;
	};

	class DefaultRawMemoryAllocator final : public IMemoryAllocator
	{
	public:

		DefaultRawMemoryAllocator();

		virtual void* Allocate(size_t Size) override;

		virtual void Free(void* Ptr) override;

		static DefaultRawMemoryAllocator& GetAllocator();

	private:
		DefaultRawMemoryAllocator(const DefaultRawMemoryAllocator&) = delete;
		DefaultRawMemoryAllocator(DefaultRawMemoryAllocator&&) = delete;
		DefaultRawMemoryAllocator& operator=(const DefaultRawMemoryAllocator&) = delete;
		DefaultRawMemoryAllocator& operator=(DefaultRawMemoryAllocator&&) = delete;
	};

    template <typename T, typename AllocatorType>
    struct STDAllocator
    {
        using value_type = T;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        STDAllocator(AllocatorType& Allocator) noexcept :
            m_Allocator{ Allocator }
        {
        }

        template <class U>
        STDAllocator(const STDAllocator<U, AllocatorType>& other) noexcept :
            m_Allocator{ other.m_Allocator }
        {
        }

        template <class U>
        STDAllocator(STDAllocator<U, AllocatorType>&& other) noexcept :
            m_Allocator{ other.m_Allocator }
        {
        }

        template <class U> struct rebind
        {
            typedef STDAllocator<U, AllocatorType> other;
        };

        T* allocate(std::size_t count)
        {
            return reinterpret_cast<T*>(m_Allocator.Allocate(count * sizeof(T)));
        }

        pointer       address(reference r) { return &r; }
        const_pointer address(const_reference r) { return &r; }

        void deallocate(T* p, std::size_t count)
        {
            m_Allocator.Free(p);
        }

        inline size_type max_size() const
        {
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }

        //    construction/destruction
        template <class U, class... Args>
        void construct(U* p, Args&&... args)
        {
            ::new (p) U(std::forward<Args>(args)...);
        }

        inline void destroy(pointer p)
        {
            p->~T();
        }

        AllocatorType& m_Allocator;
    };

    template <class T, class U, class A>
    bool operator==(const STDAllocator<T, A>& left, const STDAllocator<U, A>& right)
    {
        return &left.m_Allocator == &right.m_Allocator;
    }

    template <class T, class U, class A>
    bool operator!=(const STDAllocator<T, A>& left, const STDAllocator<U, A>& right)
    {
        return !(left == right);
    }

    template <class T> using STDAllocatorRawMem = STDAllocator<T, IMemoryAllocator>;

    template <class T, typename AllocatorType>
    struct STDDeleter
    {
        STDDeleter() noexcept {}

        STDDeleter(AllocatorType& Allocator) noexcept :
            m_Allocator{ &Allocator }
        {}

        STDDeleter(const STDDeleter&) = default;
        STDDeleter& operator=(const STDDeleter&) = default;

        STDDeleter(STDDeleter&& rhs) noexcept :
            m_Allocator{ rhs.m_Allocator }
        {
            rhs.m_Allocator = nullptr;
        }

        STDDeleter& operator=(STDDeleter&& rhs) noexcept
        {
            m_Allocator = rhs.m_Allocator;
            rhs.m_Allocator = nullptr;
            return *this;
        }

        void operator()(T* ptr) noexcept
        {
            QGFX_VERIFY(m_Allocator != nullptr, "The deleter has been moved away or never initialized, and can't be used");
            Destruct(ptr);
            m_Allocator->Free(ptr);
        }

    private:
        AllocatorType* m_Allocator = nullptr;

        typename std::enable_if<std::is_destructible<T>::value, void>::type Destruct(T* ptr)
        {
            ptr->~T();
        }

        typename std::enable_if<!std::is_destructible<T>::value, void>::type Destruct(T* ptr)
        {
        }
    };
    template <class T> using STDDeleterRawMem = STDDeleter<T, IMemoryAllocator>;
}