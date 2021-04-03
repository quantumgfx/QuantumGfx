#pragma once

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	class IBuffer
	{
	public:

		virtual const BufferDesc& GetDesc() const = 0;
		virtual uint32_t GetSizeInByte() const = 0;
		virtual BufferUsageFlags GetUsage() const = 0;
		virtual BufferMemoryType GetMemoryType() const = 0;
		virtual CPUAccessFlags GetCPUAccess() const = 0;
	};
}