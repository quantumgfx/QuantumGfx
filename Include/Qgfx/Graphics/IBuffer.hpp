#pragma once

#include "Forward.hpp"
#include "GraphicsTypes.hpp"

#include "../Common/FlagsEnum.hpp"
#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
    enum class BufferMemoryType
    {
        /**
         * @brief A resource that can only be read by the GPU. It cannot be written by the GPU, and can not be accessed at all by the CPU. \n
         * Static buffers do not allow CPU access and must use CPUAccess::None.
        */
        Immutable = 0,
        Default,
        Dynamic,
        Staging,
        Unified,
    };

    enum class BufferState
    {
        eUndefined = 0,
        eVertex,
        eUniform,
        eIndex,
        eIndirect,
        eShaderResource,
        eUnorderedAccess,
        eTransferSrc,
        eTransferDst,
    };

    enum class BufferUsageFlagBits
    {
        eNone = 0x000,
        eVertex = 0x001,
        eUniform = 0x002,
        eIndex = 0x004,
        eIndirect = 0x008,
        eStorage = 0x010,
        eTransferSrc = 0x200,
        eTransferDst = 0x080,
        eMapRead = 0x100,
        eMapWrite = 0x100
    };

    template<>
    struct EnableEnumFlags<BufferUsageFlagBits>
    {
        static const bool bEnabled = true;
    };

    using BufferUsageFlags = Flags<BufferUsageFlagBits>;

	struct BufferCreateInfo
	{
		uint32_t Size = 0;

		BufferUsageFlags Usage = {};

		// BufferMemoryType MemoryType = BufferMemoryType::Immutable;

		ICommandQueue* pInitialQueue = nullptr;
	};

	class IBuffer : public IRefCountedObject<IBuffer>
	{
	public:

		virtual uint32_t GetSize() const = 0;

		virtual BufferUsageFlags GetUsage() const = 0;
		//virtual BufferMemoryType GetMemoryType() const = 0;
	};
}