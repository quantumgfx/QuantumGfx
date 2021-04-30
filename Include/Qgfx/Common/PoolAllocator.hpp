#pragma once

#include "MemoryAllocator.hpp"

#include <cstdint>

namespace Qgfx
{
	class PoolAllocator final : public IMemoryAllocator
	{
	public:

		PoolAllocator()
		{
		}

		~PoolAllocator()
		{
		}

		void* Allocate(size_t Size) override
		{
			return new uint8_t[Size];
		}

		void Deallocate(void* Ptr, size_t Size) override
		{
			delete[] reinterpret_cast<uint8_t*>(Ptr);
		}

	private:

	};
}