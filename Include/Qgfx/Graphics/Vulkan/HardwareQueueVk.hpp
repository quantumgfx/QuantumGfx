#pragma once

//#include "RenderDeviceVk.hpp"
//
//namespace Qgfx
//{
//	struct HardwareQueueVkCreateInfo
//	{
//		uint32_t QueueFamilyIndex;
//		uint32_t QueueIndex;
//		HardwareQueueInfoVk Info;
//	};
//
//	class HardwareQueueVk
//	{
//	public:
//
//		void WaitIdle();
//
//		void Submit(const vk::ArrayProxy<const vk::SubmitInfo>& Submits, vk::Fence Fence);
//
//		void Present(const vk::PresentInfoKHR& PresentInfo);
//
//		inline uint32_t GetVkQueueFamilyIndex() const { return m_VkQueueFamilyIndex; }
//		inline uint32_t GetVkQueueIndex() const { return m_VkQueueIndex; }
//		inline vk::Queue GetVkQueue() const { return m_VkQueue; }
//
//		inline float GetPriority() const { return m_Priority; }
//
//		inline CommandQueueType GetQueueType() const { return m_QueueType; }
//
//	private:
//
//		friend class RenderDeviceVk;
//
//		HardwareQueueVk(RenderDeviceVk* pRenderDevice, const HardwareQueueVkCreateInfo& CreateInfo);
//
//		~HardwareQueueVk();
//
//		RenderDeviceVk* m_pRenderDevice;
//
//		std::mutex m_Mutex;
//
//		uint32_t m_VkQueueFamilyIndex;
//		uint32_t m_VkQueueIndex;
//		vk::Queue m_VkQueue;
//
//		float m_Priority;
//		CommandQueueType m_QueueType;
//
//	};
//}