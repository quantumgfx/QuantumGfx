#pragma once

#include "GraphicsTypes.hpp"
#include "IObject.hpp"

#include "IRenderDevice.hpp"

namespace Qgfx
{
	class IInstance : public IObject
	{
	public:

		IInstance(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IInstance() = default;

		virtual const APIInfo& GetAPIInfo() const = 0;

		virtual GraphicsInstanceType GetType() const = 0;

	};
}