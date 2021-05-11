#pragma once

#include "../Common/FlagsEnum.hpp"
#include "../Platform/NativeWindow.hpp"

#include "IBase.hpp"
#include "IResource.hpp"

namespace Qgfx
{

	class IQueue;
	class IDevice;
	class IAdapter;
	class IRenderer;

	//////////////////////////////
	// Command Buffer ////////////
	//////////////////////////////

	enum class CommandBufferState
	{
		eRecording = 0,
		eReady,
		eExecuting,
	};

	class ICommandBuffer : public IRefCountedObject
	{
	public:

		virtual void Finish() = 0;

		/**
		 * @brief Retreives the queue that was owns the command buffer.
		 * @param ppQueue Pointer to be filled with pointer to the queue (increments references to pQueue).
		*/
		void GetQueue(IQueue** ppQueue);

		inline CommandBufferState GetState() { return m_State; }

	protected:

		ICommandBuffer(IQueue* pQueue);
		~ICommandBuffer();

		IQueue* m_pQueue;

		CommandBufferState m_State = CommandBufferState::eRecording;
	};

	//////////////////////////////
	// Queues ////////////////////
	//////////////////////////////

	enum class QueueFlagBits
	{
		eNone = 0x00,
	};

	template<>
	struct EnableEnumFlags<QueueFlagBits>
	{
		static const bool bEnabled = true;
	};

	using QueueFlags = Flags<QueueFlagBits>;

	struct QueueDesc
	{
		QueueType Type = QueueType::eGraphics;
		QueueFlags Flags = QueueFlagBits::eNone;
	};

	class IQueue : public IRefCountedObject
	{
	public:

		virtual void CreateCommandBuffer(ICommandBuffer** ppCommandBuffer) = 0;

		/**
		 * @brief This returns a uint64 value that represents all work done on the queue up until this point.
		 * @return A value representing all work done on the queue up until this point.
		*/
		virtual uint64_t Signal() = 0;

		/**
		 * @brief This function blocks the CPU until the GPU finishes all work represented by the value. This value must have been retrieved by IQueue::Signal().
		 * @param Value Represents all work up to a specified point.
		*/
		virtual void Wait(uint64_t Value) = 0;

		/**
		 * @brief This function blocks the CPU until all submitted work on the GPU is finished. It functions indentically to the following:
		 * IQueue* pQueue;
		 * pQueue->Wait(pQueue->Signal());
		*/
		virtual void WaitIdle() = 0;

		/**
		 * @brief This function inserts a "fence" or work dependency onto this queue. It requires that all work up to the specified point must be finished 
		 * completing on the other queue before this queue can run any more commands. This function is non blocking, and inserts a wait on the GPU, rather
		 * than the CPU (for the other way round, use IQueue::Wait()).
		 * @param pQueue Queue to wait upon
		 * @param Value Represents all work up to a specified point.
		*/
		virtual void Fence(IQueue* pQueue, uint64_t Value) = 0;

		inline QueueType GetType() { return m_Type; }

		void GetDevice(IDevice** ppDevice);

	protected:
		
		IQueue(IDevice* pDevice);
		~IQueue();

		IDevice* m_pDevice;

		QueueType m_Type;
	};

	////////////////////////////////
	// Swap Chain //////////////////
	////////////////////////////////

	enum class SwapChainCreationFlagBits
	{
		/**
		 * @brief No-op flag
		*/
		eNone = 0x00,
		/**
		 * @brief This flag, when set, indicates that the texture should be cleared at the start of every frame, rather than just when the swapchain is resized.
		*/
		eClearOnAcquire = 0x01,
	};

	template<>
	struct EnableEnumFlags<SwapChainCreationFlagBits>
	{
		static const bool bEnabled = true;
	};

	using SwapChainCreationFlags = Flags<SwapChainCreationFlagBits>;

	/// The transform applied to the image content prior to presentation.
	enum class SurfaceTransform
	{
		/// Uset the most optimal surface transform.
		eOptimal = 0,

		/// The image content is presented without being transformed.
		eIdentity,

		/// The image content is rotated 90 degrees clockwise.
		eRotate90,

		/// The image content is rotated 180 degrees clockwise.
		eRotate180,

		/// The image content is rotated 270 degrees clockwise.
		eRotate270,

		/// The image content is mirrored horizontally.
		eHorizontalMirror,

		/// The image content is mirrored horizontally, then rotated 90 degrees clockwise.
		eHorizontalMirrorRotate90,

		/// The  image content is mirrored horizontally, then rotated 180 degrees clockwise.
		eHorizontalMirrorRotate180,

		/// The  image content is mirrored horizontally, then rotated 270 degrees clockwise.
		eHorizontalMirrorRotate270,
	};

	struct SwapChainDesc
	{
		SwapChainCreationFlags Flags = SwapChainCreationFlagBits::eNone;

		NativeWindow Window;

		/**
		 * @brief This specifies the initial color the swapchain is cleared to (on creation/recreation), as well as the optimized clear color of the render attachment.
		 * Furthermore, if the eClearOnAcquire flag is set, then this is color the swapchain is cleared to on every AcquireNextTexture.
		*/
		ClearValue ClearColor;

		/**
		 * @brief Initial width of the swapchain
		*/
		uint32_t Width = 0;
		/**
		 * @brief Initial height of the swapchain
		*/
		uint32_t Height = 0;
		/**
		 * @brief Initial requested texture count of the swapchain
		*/
		uint32_t TextureCount = 3;
		/**
		 * @brief Format of swapchain textures
		*/
		TextureFormat Format = TextureFormat::eRGBA8UnormSrgb;
		/**
		 * @brief Desired pre transform to use for the swapchain
		*/
		SurfaceTransform PreTransform = SurfaceTransform::eOptimal;

		/**
		 * @brief Usages of the swapchain textures
		*/
		ResourceUsageFlags Usage = ResourceUsageFlagBits::eRenderAttachment;
	};

	enum class SwapChainOpResult
	{
		eSuccess = 0,
		eSuboptimal,
		eOutOfDate,
		eSurfaceLost,
		eMinimized,
	};

	class ISwapChain : public IRefCountedObject
	{
	public:

		/**
		 * @brief Returns the current width of the swapchain.
		*/
		inline uint32_t GetWidth() { return m_Width; }
		
		/**
		 * @brief Returns the current height of the swapchain.
		*/
		inline uint32_t GetHeight() { return m_Height; }

		/**
		 * @brief Returns the current pre transform of the swapchain.
		 * @return The current pre transform
		*/
		inline SurfaceTransform GetPreTransform() { return m_PreTransform; }

		/**
		 * @brief Returns the number of textures in the swapchain.
		*/
		inline uint32_t GetTextureCount() { return m_TextureCount; }

		/**
		 * @brief Returns whether or not the swapchain is currently minimized.
		*/
		inline bool IsMinimized() { return m_bIsMinimized; }

		/**
		 * @brief Returns whether of not VSync is enabled for the swapchain.
		 * @return 
		*/
		inline bool IsVSyncEnabled() { return m_bVSyncEnabled; }

		/**
		 * @brief Acquires the next texture in the swapchain. The swapchain must not currently be acquired.
		 * If this function detects that the window must be resized (because the present surface is out of date,
		 * or suboptimal), it does so (and updates Width, Height, and IsMinimized accordingly). If the swapchain's
		 * connected surface is currently minimized, acquire succeeds, but GetCurrentTexture() and 
		 * GetCurrentTextureView() return nullptr. 
		 * AcquireNextTexture() can fail if any of the following occur:
		 * - The window/surface this swapchain is connected to has been destroyed
		 * - The window/surface this swapchain is connected to is no longer visible (ie, it has been minimized or the like).
		 * @returns Whether or not the acquire was successful.  
		*/
		SwapChainOpResult AcquireNextTexture();
		
		/**
		 * @brief Presents the current swapchain texture. The current texture must be acquired (ie, Resize() must not
		 * have been called between this call and AcquireNextTexture()).
		*/
		SwapChainOpResult Present();

		/**
		 * @brief Resizes the swapchain. If the texture is currently acquired, this call 
		 * invalidates it and means that you must call AcquireNextTexture() again before Present().
		*/
		void Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewTransform);

		void GetQueue(IQueue** ppQueue);

		void GetRenderer(IRenderer** ppRenderer);

	protected:

		ISwapChain(IRenderer* pRenderer, IQueue* pQueue, const SwapChainDesc& Descriptor);
		~ISwapChain();

		virtual SwapChainOpResult AcquireNextTextureImpl() = 0;

		virtual SwapChainOpResult PresentImpl() = 0;

		virtual void ResizeImpl(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewTransform) = 0;

		IRenderer* m_pRenderer;
		IQueue* m_pQueue;

		NativeWindow m_Window;

		TextureFormat m_Format;
		SwapChainCreationFlags m_Flags;
		ResourceUsageFlags m_Usage;
		uint32_t m_Width;
		uint32_t m_Height;
		bool     m_bIsMinimized;
		bool     m_bVSyncEnabled;
		uint32_t m_TextureCount;

		ClearValue m_ClearColor;

		SurfaceTransform m_PreTransform;

		bool m_bCurrentTextureAcquired = false;
	};

	////////////////////////////////
	// Render Device ///////////////
	////////////////////////////////

	enum class FeatureState
	{
		eDisabled = 0,
		eEnabled,
		eOptional
	};

	struct DeviceFeatures
	{
		FeatureState ComputeShaders =     FeatureState::eDisabled;
		FeatureState TesselationShaders = FeatureState::eDisabled;
		FeatureState GeometryShaders =    FeatureState::eDisabled;
		FeatureState IndirectRendering =  FeatureState::eDisabled;
		FeatureState PolygonModeLine =    FeatureState::eDisabled;
		FeatureState PolygonModePoint =   FeatureState::eDisabled;
	};

	struct DeviceDesc
	{
		DeviceFeatures Features = {};
	};

	class IDevice : public IRefCountedObject
	{
	public:

		virtual IQueue* CreateQueue(const QueueDesc& Descriptor) = 0;

		virtual void WaitIdle() = 0;

		virtual bool IsTextureFormatSupported(TextureFormat Fmt, ResourceUsageFlags Usage) = 0;

		const DeviceFeatures& GetFeatures() const { return m_SupportedFeatures; }

		void GetRenderer(IRenderer** ppRenderer);

		void GetAdapter(IAdapter** ppAdapter);

		IMemoryAllocator& GetRawMemAllocator();

	protected:

		IDevice(IRenderer* pRenderer, IAdapter* pAdapter);
		~IDevice();

		IRenderer* m_pRenderer;
		IAdapter* m_pAdapter;

		DeviceFeatures m_SupportedFeatures;
	};

	enum class AdapterType
	{
		eIntegrated,
		eDedicated,
	};

	class IAdapter : public IRefCountedObject
	{
	public:

		inline AdapterType GetType() { return m_Type; }

		void GetRenderer(IRenderer** ppRenderer);

	protected:

		IAdapter(IRenderer* pRenderer);
		~IAdapter();

		AdapterType m_Type;

		IRenderer* m_pRenderer;
		
	};

	enum class RendererApi
	{
		eVulkan = 0,
	};

	struct RendererDesc
	{
		RendererApi Api;
		void* pNativeDesc = nullptr;
	};

	class IRenderer : public IRefCountedObject
	{
	public:

		static void Create(const RendererDesc& Descriptor, IRenderer** ppRenderer);

		virtual RendererApi GetApi() const = 0;

		virtual uint32_t GetAdapterCount() = 0;

		virtual void EnumerateAdapters(uint32_t Index, IAdapter** ppAdapter) = 0;

		virtual void CreateDevice(IAdapter* pAdapter, const DeviceDesc& Descriptor, IDevice** ppDevice) = 0;

		virtual void CreateSwapChain(IQueue* pQueue, const SwapChainDesc& Descriptor, ISwapChain** ppSwapChain) = 0;

		IMemoryAllocator& GetRawMemAllocator() { return m_RawMemAllocator; }

	protected:

		IRenderer();
		~IRenderer();

		IMemoryAllocator& m_RawMemAllocator;

	};

}