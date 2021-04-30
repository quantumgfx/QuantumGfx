#pragma once

#include "../Common/RefCountedObject.hpp"
#include "../Command/RefPtr.hpp"

namespace Qgfx
{
	class IObject : public RefCountedObject
	{
	public:

		IObject(IRefCounter* pRefCounter)
			: RefCountedObject{ pRefCounter }
		{
		}

	};
}