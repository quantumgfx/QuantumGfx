#pragma once

#include "Error.hpp"

namespace Qgfx
{
	template <typename DstType, typename SrcType>
	void CheckDynamicType(SrcType* pSrcPtr)
	{
		QGFX_VERIFY(pSrcPtr == nullptr || dynamic_cast<DstType*>(pSrcPtr) != nullptr, "Dynamic type cast failed. Src typeid: \'", typeid(*pSrcPtr).name(), "\' Dst typeid: \'", typeid(DstType).name(), '\'');
	}

	template <typename DstType, typename SrcType>
	DstType* ValidatedCast(SrcType* Ptr)
	{
#ifdef QGFX_DEBUG
		if (Ptr != nullptr)
		{
			CheckDynamicType<DstType>(Ptr);
		}
#endif
		return static_cast<DstType*>(Ptr);
	}
}