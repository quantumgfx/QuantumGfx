#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"
#include "IRenderContext.hpp"
#include "ISwapChain.hpp"

#include "../Platform/NativeWindow.hpp"

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

		virtual const DeviceFeatures& GetFeatures() const = 0;

	};
}