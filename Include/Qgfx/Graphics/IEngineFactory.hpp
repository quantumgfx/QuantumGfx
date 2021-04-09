#pragma once

#include "GraphicsTypes.hpp"
#include "IObject.hpp"

#include "IRenderDevice.hpp"

namespace Qgfx
{
	class IEngineFactory : public IObject
	{
	public:

		IEngineFactory(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IEngineFactory() = default;

		virtual const APIInfo& GetAPIInfo() const = 0;

		virtual GraphicsInstanceType GetType() const = 0;

	};
}