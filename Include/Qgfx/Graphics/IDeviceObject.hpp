#pragma once

#include "Forward.hpp"

#include "../Common/MemoryAllocator.hpp"
#include "../Common/RefCountedObject.hpp"

namespace Qgfx
{
	class IDeviceObject : public IRefCountedObject
	{
	protected:

		IDeviceObject(IMemoryAllocator* pAlloc, IRenderDevice* pRenderDevice, bool bIsDeviceInternal);

		virtual ~IDeviceObject();

		virtual void DeleteThis() override;

		IMemoryAllocator* m_pAlloc;
		IRenderDevice* m_pRenderDevice;
		const bool m_bIsDeviceInternal;

	};
}