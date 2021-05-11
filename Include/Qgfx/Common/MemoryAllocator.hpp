#pragma once

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
}