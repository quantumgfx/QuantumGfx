#pragma once

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	struct BufferCreateInfo
	{
		uint32_t SizeInBytes = 0;

		BufferUsageFlags Usage = {};

		BufferMemoryType MemoryType = BufferMemoryType::Immutable;
	};

	class IBuffer
	{
	public:

		virtual uint32_t GetSizeInByte() const = 0;
		virtual BufferUsageFlags GetUsage() const = 0;
		virtual BufferMemoryType GetMemoryType() const = 0;
		virtual CPUAccessFlags GetCPUAccess() const = 0;
	};
}