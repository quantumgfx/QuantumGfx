#pragma once

#include "../IRenderContext.hpp"

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"

namespace Qgfx
{
	class RenderContextVk final : public IRenderContext
	{
	public:

		RenderContextVk(RefCounter* pRefCounter, RenderDeviceVk* pRenderDevice, uint32_t QueueIndex);

		~RenderContextVk();

		virtual RenderContextType GetType() override { return m_Type; }
		
		virtual void InvalidateState() override;

		virtual void Flush() override;

		virtual void WaitIdle() override;

	private:

		RenderContextType m_Type;

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;

		uint32_t m_QueueIndex;
	};
}