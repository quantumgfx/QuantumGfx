#pragma once

#include <vector>
#include <set>

#include "../Common/Error.hpp"
#include "../Common/ValidatedCast.hpp"
#include "../Common/MemoryAllocator.hpp"
#include "../Common/FixedBlockMemoryAllocator.hpp"
#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	enum class QueueType
	{
		eGraphics,
		eCompute,
		eTransfer,
	};

	enum class CompareFunc
	{
		eNever = 0,
		eLess,
		eEqual,
		eLessEqual,
		eGreater,
		eGreaterEqual,
		eNotEqual,
		eAlways
	};

	enum class FilterMode
	{
		eNearest = 0,
		eLinear,
	};


	/**
	 * @brief Value to clear a texture to. Double precision is used (as it can percisely hold a uint32_t),
	 * but is cast to the appropriate type depending on the format of the texture.
	*/
	union ClearValue
	{
		struct
		{
			double R;
			double G;
			double B;
			double A;
		};
		struct
		{
			float   Depth;
			uint32_t Stencil;
		};
	};

	struct Extent2D
	{
		uint32_t Width;
		uint32_t Height;
	};

	struct Extent3D
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
	};
}