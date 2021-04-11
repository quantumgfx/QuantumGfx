#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"
#include "ICommandQueue.hpp"
#include "IFence.hpp"
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

		virtual void CreateFence(uint64_t InitialValue, IFence** ppFence) = 0;

		virtual const DeviceFeatures& GetFeatures() const = 0;

	};
}