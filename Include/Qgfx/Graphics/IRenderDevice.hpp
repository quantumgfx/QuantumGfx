#pragma once

#include "GraphicsTypes.hpp"

#include "IObject.hpp"
#include "IInstance.hpp"

namespace Qgfx
{
	class IRenderDevice : public IObject
	{

	public:

		IRenderDevice(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IRenderDevice() = default;

		virtual void WaitIdle() = 0;

		virtual const DeviceFeatures& GetFeatures() = 0;

	};
}