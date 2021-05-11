#include "Qgfx/Graphics/IRenderer.hpp"

#ifdef QGFX_VULKAN_SUPPORTED
#include "Qgfx/Graphics/Vulkan/VulkanRenderer.hpp"
#endif

namespace Qgfx
{
	void IRenderer::Create(const RendererDesc& Descriptor, IRenderer** ppRenderer)
	{
		switch (Descriptor.Api)
		{
		case RendererApi::eVulkan:
		{
#ifdef QGFX_VULKAN_SUPPORTED
			*ppRenderer = new VulkanRenderer(Descriptor);
			return;
#else
#error Vulkan unsupported on current platform.
#endif
		}

		default:
		{
			QGFX_UNEXPECTED("Unexpected value of RendererDesc::Api");
			*ppRenderer = nullptr;
			return;
		}
		}
	}

	ICommandBuffer::ICommandBuffer(IQueue* pQueue)
		: m_pQueue(pQueue)
	{
		m_pQueue->AddRef();
	}

	ICommandBuffer::~ICommandBuffer()
	{
		m_pQueue->Release();
	}

	void ICommandBuffer::GetQueue(IQueue** ppQueue)
	{
		m_pQueue->AddRef();
		*ppQueue = m_pQueue;
	}

	IQueue::IQueue(IDevice* pDevice)
		: m_pDevice(pDevice)
	{
		m_pDevice->AddRef();
	}

	IQueue::~IQueue()
	{
		m_pDevice->Release();
	}

	void IQueue::GetDevice(IDevice** ppDevice)
	{
		m_pDevice->AddRef();
		*ppDevice = m_pDevice;
	}

	ISwapChain::ISwapChain(IRenderer* pRenderer, IQueue* pQueue, const SwapChainDesc& Descriptor)
		: m_pRenderer(pRenderer), m_pQueue(pQueue)
	{
		m_pRenderer->AddRef();
		m_pQueue->AddRef();

		m_Format =       Descriptor.Format;
		m_Flags =        Descriptor.Flags;
		m_Window =       Descriptor.Window;
		m_Usage =        Descriptor.Usage;
		m_ClearColor =   Descriptor.ClearColor;
		m_Width =        Descriptor.Width;
		m_Height =       Descriptor.Height;
		m_PreTransform = Descriptor.PreTransform;
		m_TextureCount = Descriptor.TextureCount;
		m_bVSyncEnabled = true;
		m_bIsMinimized = (m_Width == 0 && m_Height == 0);
	}

	ISwapChain::~ISwapChain()
	{
		m_pQueue->Release();
		m_pRenderer->Release();
	}

	SwapChainOpResult ISwapChain::AcquireNextTexture()
	{
		QGFX_VERIFY(!m_bCurrentTextureAcquired, "Texture is already acquired");

		if (m_bIsMinimized)
		{
			m_bCurrentTextureAcquired = false;
			return SwapChainOpResult::eMinimized;
		}

		SwapChainOpResult Result = AcquireNextTextureImpl();

		if (Result == SwapChainOpResult::eOutOfDate || Result == SwapChainOpResult::eSurfaceLost)
		{
			m_bCurrentTextureAcquired = false;
			return Result;
		}

		m_bCurrentTextureAcquired = true;

		return Result;
	}

	SwapChainOpResult ISwapChain::Present()
	{
		QGFX_VERIFY(m_bCurrentTextureAcquired, "Texture is must be acquired to be presented");

		m_bCurrentTextureAcquired = false;

		SwapChainOpResult Result = PresentImpl();

		if (Result == SwapChainOpResult::eOutOfDate || Result == SwapChainOpResult::eSurfaceLost)
		{
			return Result;
		}

		return Result;
	}

	void ISwapChain::Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewTransform)
	{
		m_bCurrentTextureAcquired = false;

		ResizeImpl(NewWidth, NewHeight, NewTransform);

		m_bIsMinimized = m_Width == 0 && m_Height == 0;
	}

	void ISwapChain::GetQueue(IQueue** ppQueue)
	{
		m_pQueue->AddRef();
		*ppQueue = m_pQueue;
	}

	void ISwapChain::GetRenderer(IRenderer** ppRenderer)
	{
		m_pRenderer->AddRef();
		*ppRenderer = m_pRenderer;
	}

	IDevice::IDevice(IRenderer* pRenderer, IAdapter* pAdapter)
		: m_pRenderer(pRenderer), m_pAdapter(pAdapter)
	{
		m_pRenderer->AddRef();
		m_pAdapter->AddRef();
	}

	IDevice::~IDevice()
	{
		m_pAdapter->Release();
		m_pRenderer->Release();
	}

	void IDevice::GetRenderer(IRenderer** ppRenderer)
	{
		m_pRenderer->AddRef();
		*ppRenderer = m_pRenderer;
	}

	void IDevice::GetAdapter(IAdapter** ppAdapter)
	{
		m_pAdapter->AddRef();
		*ppAdapter = m_pAdapter;
	}

	IMemoryAllocator& IDevice::GetRawMemAllocator()
	{
		return m_pRenderer->GetRawMemAllocator();
	}

	IAdapter::IAdapter(IRenderer* pRenderer)
		: m_pRenderer(pRenderer)
	{
		m_pRenderer->AddRef();
	}

	IAdapter::~IAdapter()
	{
		m_pRenderer->Release();
	}

	void IAdapter::GetRenderer(IRenderer** ppRenderer)
	{
		m_pRenderer->AddRef();
		*ppRenderer = m_pRenderer;
	}

	IRenderer::IRenderer()
		: m_RawMemAllocator(DefaultRawMemoryAllocator::GetAllocator())
	{
	}

	IRenderer::~IRenderer()
	{
	}
}