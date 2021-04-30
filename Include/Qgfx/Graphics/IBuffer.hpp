#pragma once

#include "Forward.hpp"
#include "GraphicsTypes.hpp"
#include "IObject.hpp"

#include "../Common/FlagsEnum.hpp"

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
        eVertex = 0,
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
        eFormattedUniform = 0x020,
        eFormattedStorage = 0x040,
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

	class IBuffer : public IObject
	{
	public:

		IBuffer(IRefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual uint32_t GetSize() const = 0;

		virtual BufferUsageFlags GetUsage() const = 0;
		//virtual BufferMemoryType GetMemoryType() const = 0;
	};
}