#pragma once

#include "BaseVk.hpp"
#include "RenderDeviceVk.hpp"

#include "../IFence.hpp"

#include "../../Common/RefAutoPtr.hpp"

namespace Qgfx
{
	enum class FenceVkType
	{
		Standard = 0,
		Timeline
	};

	class FenceVk : public IFence
	{
	public:

		FenceVk(RefCounter* pRefCounter)
			: IFence(pRefCounter)
		{
		}

		virtual FenceVkType GetFenceVkType() = 0;

	};


	class TimelineFenceVk final : public FenceVk
	{
	public:

		TimelineFenceVk(RefCounter* pRefCounter, RenderDeviceVk* pDevice, uint64_t InitialValue);

		~TimelineFenceVk();

		virtual FenceVkType GetFenceVkType() { return FenceVkType::Timeline; }

		virtual uint64_t GetCompletedValue() override;

		virtual void Signal(uint64_t Value) override;

		virtual void Wait(uint64_t Value) override;

	private:

		RefAutoPtr<RenderDeviceVk> m_spRenderDevice;
		vk::Semaphore m_VkSemaphore;
	};
}