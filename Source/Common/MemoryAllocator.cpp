#include "Qgfx/Common/MemoryAllocator.hpp"
#include "Qgfx/Common/Error.hpp"

#include <cstdint>
#include <memory>

namespace Qgfx
{
    DefaultRawMemoryAllocator::DefaultRawMemoryAllocator()
    {
    }

    void* DefaultRawMemoryAllocator::Allocate(size_t Size)
    {
        QGFX_VERIFY_EXPR(Size > 0);
        return new uint8_t[Size];
    }

    void DefaultRawMemoryAllocator::Free(void* Ptr)
    {
        delete[] reinterpret_cast<uint8_t*>(Ptr);
    }

    DefaultRawMemoryAllocator& DefaultRawMemoryAllocator::GetAllocator()
    {
        static DefaultRawMemoryAllocator Allocator;
        return Allocator;
    }
}