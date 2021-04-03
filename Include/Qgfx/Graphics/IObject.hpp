#pragma once

#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	class IObject : public RefCountedObject
	{
	public:

		IObject(RefCounter* pRefCounter)
			: RefCountedObject{ pRefCounter }
		{
		}

	};
}