#pragma once

#include "VulkanBase.hpp"

#include "../IRenderer.hpp"

namespace Qgfx
{

	class VulkanRenderer;
	class VulkanDevice;
	class VulkanQueue;
	class VulkanCommandBuffer;

	struct VulkanRendererDesc
	{
		const char* pAppName = nullptr;
		uint32_t AppVersion = VK_MAKE_VERSION(1, 0, 0);
		const char* pEngineName = nullptr;
		uint32_t EngineVersion = VK_MAKE_VERSION(1, 0, 0);

		/**
		 * @brief Optional pointer to the function that loads all vulkan related function pointers.
		*/
		PFN_vkGetInstanceProcAddr pfnLoaderHandle = nullptr;

		/**
		 * @brief Setting this to true will cause the instance to try to find and loader vulkan layers
		*/
		bool bEnableValidation = false;
	};

	class VulkanRenderer final : public IRenderer
	{
	public:

		virtual RendererApi GetApi() const override { return RendererApi::eVulkan; }

		virtual uint32_t GetAdapterCount() override { return static_cast<uint32_t>(m_PhysicalDevices.size()); }

		virtual void EnumerateAdapters(uint32_t Index, IAdapter** ppAdapter) override;

		virtual void CreateDevice(IAdapter* pAdapter, const DeviceDesc& Descriptor, IDevice** ppDevice) override;

		virtual void CreateSwapChain(IQueue* pQueue, const SwapChainDesc& Descriptor, ISwapChain** ppSwapChain) override;

		inline vk::Instance GetVkInstance() const { return m_VkInstance; }

		inline const vk::DispatchLoaderDynamic& GetVkInstanceDispatch() const { return m_VkDispatch; }

	private:

		friend IRenderer;

		VulkanRenderer(RendererDesc Descriptor);
		~VulkanRenderer();

		virtual void DeleteThis() override;

		vk::DispatchLoaderDynamic m_VkDispatch;
		vk::Instance m_VkInstance;

		std::vector<vk::PhysicalDevice> m_PhysicalDevices;
	};

	class VulkanAdapter final : public IAdapter
	{
	public:

		vk::PhysicalDevice GetVkPhysicalDevice() const { return m_VkPhysicalDevice; }

	private:

		VulkanAdapter(VulkanRenderer* pRenderer, vk::PhysicalDevice VkPhysicalDevice);
		~VulkanAdapter();

		virtual void DeleteThis() override;

		VulkanRenderer* m_pRenderer;

		vk::PhysicalDevice m_VkPhysicalDevice;

	};

	class VulkanDevice final : public IDevice
	{
	public:

		virtual IQueue* CreateQueue(const QueueDesc& Descriptor) override;

		virtual void WaitIdle() override;

		/**
		 * @brief Returns the appropriate vk format for a given texture format.
		 * @param Format 
		 * @return 
		*/
		vk::Format GetVkFormat(TextureFormat Format);

		/**
		 * @brief Creates a new vulkan binary semaphore.
		 * @return Binary semaphore.
		*/
		vk::Semaphore CreateVkBinarySemaphore() const;

		/**
		 * @brief Destroys a vulkan binary semaphore.
		 * @param Sem The semaphore to destroy.
		*/
		void DestroyVkSemaphore(vk::Semaphore Sem) const;

		/**
		 * @brief Gets the vulkan queue for a specific queue family (the current implementation only exposes one queue per queue family
		 * @param QueueFamilyIndex Index of the Queue Family
		 * @return Vulkan Handle for the retrieved queue.
		*/
		vk::Queue GetVkQueue(uint32_t QueueFamilyIndex) const;

		/**
		 * @brief Submits batches to a queue for execution in a thread safe manner.
		 * @param VkQueue Queue to submit to.
		 * @param Submits Corresponds to the same arg in vk::Queue::submit().
		 * @param VkFence Corresponds to the same arg in vk::Queue::submit().
		*/
		void VkQueueSubmit(vk::Queue VkQueue, const vk::ArrayProxy<const vk::SubmitInfo>& Submits, vk::Fence VkFence);

		/**
		 * @brief Issues a present command to a queue for execution in a thread safe manner.
		 * @param VkQueue Queue to present on.
		 * @param Present Corresponds to the same arg in vk::Queue::presentKHR().
		*/
		vk::Result VkQueuePresent(vk::Queue VkQueue, const vk::PresentInfoKHR& Present);

		/**
		 * @brief Retrieves the queue family for a certain queue type;
		 * @param Type 
		 * @return 
		*/
		uint32_t GetQueueFamily(QueueType Type) const;

		/**
		 * @brief Gets the vk::PhysicalDevice corresponding to the render device.
		 * @return 
		*/
		inline vk::PhysicalDevice GetVkPhDevice() { return m_VkPhDevice; }

		/**
		 * @brief Gets the underlying logical device.
		 * @return Underlying logical device.
		*/
		inline vk::Device GetVkDevice() { return m_VkDevice; }

		/**
		 * @brief Gets the underlying device dispatch table.
		 * @return Underlying device dispatch table.
		*/
		inline const vk::DispatchLoaderDynamic& GetVkDeviceDispatch() const { return m_VkDispatch; }


	private:

		friend VulkanRenderer;

		VulkanDevice(VulkanRenderer* pRenderer, VulkanAdapter* pAdapter, const DeviceDesc& Descriptor);
		~VulkanDevice();

		virtual void DeleteThis() override;

		VulkanRenderer* m_pVulkanRenderer;

		vk::PhysicalDevice m_VkPhDevice;
		vk::Device m_VkDevice;
		vk::DispatchLoaderDynamic m_VkDispatch;
		VmaAllocator m_VmaAllocator;

		struct Queue
		{
			std::unique_ptr<std::mutex> pMutex;
			vk::Queue VkHandle;
		};

		struct QueueFamily
		{
			vk::QueueFlags QueueFlags;
			std::vector<Queue> Queues;
		};

		std::mutex m_SubmitMutex;

		uint32_t m_GraphicsQueueFamilyIndex;
		uint32_t m_TransferQueueFamilyIndex;
		uint32_t m_ComputeQueueFamilyIndex;
	};

	class VulkanQueue final : public IQueue
	{
	public:

		virtual void CreateCommandBuffer(ICommandBuffer** ppCommandBuffer) override;

		vk::Queue GetVkQueue() const { return m_VkQueue; }

		uint32_t GetVkQueueFamily() const { return m_QueueFamilyIndex; }

		void DestroyVulkanCommandBuffer(VulkanCommandBuffer* pCommandBuffer);

		void GetVulkanDevice(VulkanDevice** ppDevice);

	private:

		friend VulkanDevice;

		VulkanQueue(VulkanDevice* pDevice, const QueueDesc& Descriptor);
		~VulkanQueue();

		virtual void DeleteThis() override;

	private:

		std::mutex m_AllocMutex;

		FixedBlockMemoryAllocator m_CommandBufferObjAllocator;

		std::mutex m_Mutex;

		uint32_t m_QueueFamilyIndex;
		uint32_t m_QueueIndex;

		vk::Queue m_VkQueue;
	};

	class VulkanCommandBuffer final : public ICommandBuffer
	{
	public:

		virtual void Finish() override;

	private:

		friend VulkanQueue;

		VulkanCommandBuffer(VulkanQueue* pQueue);
		~VulkanCommandBuffer();

		virtual void DeleteThis() override;

	private:

		VulkanQueue* m_pVulkanQueue;

	};

	class VulkanSwapChain final : public ISwapChain
	{
	private:

		friend VulkanRenderer;

		VulkanSwapChain(VulkanRenderer* pRenderer, VulkanQueue* pQueue, const SwapChainDesc& Descriptor);
		~VulkanSwapChain();

		virtual SwapChainOpResult AcquireNextTextureImpl() override;

		virtual SwapChainOpResult PresentImpl() override;

		virtual void ResizeImpl(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewTransform) override;

		virtual void DeleteThis() override;

		void CreateSurface();

		void CreateSwapChain();

		void WaitForImageAcquiredFences();

		void ReleaseSwapChainResources(bool bDestroySwapChain);

		void RecreateSwapChain();

	private:

		uint32_t m_PresentQueueFamilyIndex;
		vk::Queue m_PresentQueue;
		vk::Queue m_GraphicsQueue;

		VulkanRenderer* m_pVulkanRenderer;
		VulkanQueue* m_pVulkanQueue;
		VulkanDevice* m_pVulkanDevice;

		SurfaceTransform    m_DesiredPreTransform;
		uint32_t            m_DesiredTextureCount;

		vk::SurfaceKHR m_VkSurface;
		vk::SwapchainKHR m_VkSwapchain;

		uint32_t m_SemaphoreIndex;

		vk::CommandPool m_CmdPool;

		std::vector<bool> m_bImageAcquired;
		std::vector<vk::Fence> m_ImageAcquiredFences;
		std::vector<vk::Semaphore> m_ImageAcquiredSemaphores;
		std::vector<vk::Semaphore> m_SubmitCompleteSemaphores;
		std::vector<vk::CommandBuffer> m_ClearOnAcquireCommands;

		uint32_t m_TextureIndex;

		vk::Format m_VkColorFormat;
		vk::ClearColorValue m_VkClearColor;

		// std::vector<ITexture*> m_FrameTextures;
	};
}