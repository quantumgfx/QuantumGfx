#include "Qgfx/Graphics/Vulkan/SwapChainVk.hpp"
#include "Qgfx/Graphics/Vulkan/CommandQueueVk.hpp"
#include "Qgfx/Graphics/Vulkan/TypeConversionsVk.hpp"
#include "Qgfx/Common/Error.hpp"
#include "Qgfx/Common/ValidatedCast.hpp"

namespace Qgfx
{
	SwapChainVk::SwapChainVk(RenderDeviceVk* pRenderDevice, const SwapChainCreateInfo& CreateInfo)
        : m_pRenderDevice(pRenderDevice)
	{
        QGFX_VERIFY_EXPR(CreateInfo.pQueue);

        m_spCommandQueue = ValidatedCast<CommandQueueVk>(CreateInfo.pQueue);
        m_Window = CreateInfo.Window;

        m_TextureFormat = CreateInfo.Format;
        m_DesiredTextureCount = CreateInfo.TextureCount;
        m_DesiredPreTransform = CreateInfo.PreTransform;
        m_Usage = CreateInfo.Usage;

        m_Width = CreateInfo.Width;
        m_Height = CreateInfo.Height;

        CreateSurface();
        CreateSwapChain();
	}

	SwapChainVk::~SwapChainVk()
	{
        vk::Instance VkInstance = m_pRenderDevice->GetVkInstance();
        const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();

        ReleaseSwapChainResources(true);

        if (m_VkSurface)
        {
            VkInstance.destroySurfaceKHR(m_VkSurface, nullptr, VkDispatch);
            m_VkSurface = nullptr;
        }
	}

    void SwapChainVk::AcquireNextTexture()
    {

        QGFX_VERIFY(!m_bAcquired, "Texture is already acquired");

        vk::Device VkDevice = m_pRenderDevice->GetVkDevice();
        const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();

        // Applications should not rely on vkAcquireNextImageKHR blocking in order to
        // meter their rendering speed. The implementation may return from this function
        // immediately regardless of how many presentation requests are queued, and regardless
        // of when queued presentation requests will complete relative to the call. Instead,
        // applications can use fence to meter their frame generation work to match the
        // presentation rate.

        // Explicitly make sure that there are no more pending frames in the command queue
        // than the number of the swap chain images.
        //
        // Nsc = 3 - number of the swap chain images
        //
        //   N-Ns          N-2           N-1            N (Current frame)
        //    |             |             |             |
        //                  |
        //          Wait for this fence
        //
        // When acquiring swap chain image for frame N, we need to make sure that
        // frame N-Nsc has completed. To achieve that, we wait for the image acquire
        // fence for frame N-Nsc-1. Thus we will have no more than Nsc frames in the queue.

        m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_TextureCount;

        uint32_t m_OldestSemaphoreIndex = (m_SemaphoreIndex + 1) % m_TextureCount;

        if (m_bImageAcquired[m_OldestSemaphoreIndex])
        {
            vk::Result Res = VkDevice.getFenceStatus(m_ImageAcquiredFences[m_OldestSemaphoreIndex], VkDispatch);
            if (Res == vk::Result::eErrorDeviceLost)
                QGFX_LOG_ERROR_AND_THROW("Unexpected device loss.");

            if (Res == vk::Result::eNotReady)
                VkDevice.waitForFences(m_ImageAcquiredFences[m_OldestSemaphoreIndex], true, UINT64_MAX, VkDispatch);

            VkDevice.resetFences(m_ImageAcquiredFences[m_OldestSemaphoreIndex], VkDispatch);

            m_bImageAcquired[m_OldestSemaphoreIndex] = false;
        }

        vk::Semaphore ImageAcquiredSemaphore = m_pRenderDevice->CreateVkBinarySemaphore();

        vk::Result Res = VkDevice.acquireNextImageKHR(m_VkSwapchain, UINT64_MAX, ImageAcquiredSemaphore, m_ImageAcquiredFences[m_SemaphoreIndex], &m_TextureIndex, VkDispatch);

        if (Res == vk::Result::eSuboptimalKHR || Res == vk::Result::eErrorOutOfDateKHR)
        {
            RecreateSwapChain();

            m_pRenderDevice->DestroyVkSemaphore(ImageAcquiredSemaphore);

            m_SemaphoreIndex = 0; // To start with 0 index when acquire next image

            ImageAcquiredSemaphore = m_pRenderDevice->CreateVkBinarySemaphore();

            Res = VkDevice.acquireNextImageKHR(m_VkSwapchain, UINT64_MAX, ImageAcquiredSemaphore, m_ImageAcquiredFences[m_SemaphoreIndex], &m_TextureIndex, VkDispatch);
        }
        QGFX_VERIFY(Res == vk::Result::eSuccess, "Failed to acquire next swap chain image");

        m_bImageAcquired[m_SemaphoreIndex] = (Res == vk::Result::eSuccess);

        m_spCommandQueue->AddWaitSemaphore(ImageAcquiredSemaphore);

        m_spCommandQueue->DeleteSemaphoreWhenUnused(ImageAcquiredSemaphore);

        m_bAcquired = true;
    }

    ITexture* SwapChainVk::GetCurrentTexture()
    {
        return m_FrameTextures[m_TextureIndex];
    }

    ITextureView* SwapChainVk::GetCurrentTextureView()
    {
        return m_FrameTextures[m_TextureIndex]->GetDefaultView();
    }

    void SwapChainVk::Present()
    {
        QGFX_VERIFY(m_bAcquired, "Swapchain must be acquired in order to present its containtss");

        m_spCommandQueue->AddSignalSemaphore(m_SubmitCompleteSemaphores[m_SemaphoreIndex]);
        m_spCommandQueue->SubmitCommandBuffers(0, nullptr);

        vk::PresentInfoKHR PresentInfo{};
        PresentInfo.pNext = nullptr;
        PresentInfo.waitSemaphoreCount = 1;
        PresentInfo.pWaitSemaphores = &m_SubmitCompleteSemaphores[m_SemaphoreIndex];
        PresentInfo.swapchainCount = 1;
        PresentInfo.pSwapchains = &m_VkSwapchain;
        PresentInfo.pImageIndices = &m_TextureIndex;
        PresentInfo.pResults = nullptr;
    }

    void SwapChainVk::Destroy()
    {
        m_pRenderDevice->DestroySwapChain(this);
    }

    void SwapChainVk::Resize(uint32_t NewWidth, uint32_t NewHeight, SurfaceTransform NewPreTransform)
    {
        bool bRecreateSwapChain = false;

#if 0
        if (m_VkSurface)
        {
            // Check orientation
            vk::PhysicalDevice VkPhDevice = m_spRenderDevice->GetVkPhysicalDevice();
            vk::Device VkDevice = m_spRenderDevice->GetVkDevice();
            const vk::DispatchLoaderDynamic& VkDispatch = m_spRenderDevice->GetVkDispatch();

            vk::SurfaceCapabilitiesKHR surfCapabilities = {};

            vk::Result err = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, &surfCapabilities, VkDispatch);
            if (err == vk::Result::eSuccess)
            {
                if (m_CurrentSurfaceTransform != surfCapabilities.currentTransform)
                {
                    // Surface orientation has changed - we need to recreate the swap chain
                    bRecreateSwapChain = true;
                }

                constexpr auto Rotate90TransformFlags =
                    vk::SurfaceTransformFlagBitsKHR::eRotate90 |
                    vk::SurfaceTransformFlagBitsKHR::eRotate270 |
                    vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90 |
                    vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;

                if (NewWidth == 0 || NewHeight == 0)
                {
                    NewWidth = m_SurfaceIdentityExtent.width;
                    NewHeight = m_SurfaceIdentityExtent.height;

                    if (surfCapabilities.currentTransform & Rotate90TransformFlags)
                    {
                        // Swap to get logical dimensions as input NewWidth and NewHeight are
                        // expected to be logical sizes.
                        std::swap(NewWidth, NewHeight);
                    }
                }

                if (NewPreTransform == SurfaceTransform::Optimal)
                {
                    if (surfCapabilities.currentTransform & Rotate90TransformFlags)
                    {
                        // Swap to get physical dimensions
                        std::swap(NewWidth, NewHeight);
                    }
                }
                else
                {
                    // Swap if necessary to get desired sizes after pre-transform
                    if (NewPreTransform == SurfaceTransform::Rotate90 ||
                        NewPreTransform == SurfaceTransform::Rotate270 ||
                        NewPreTransform == SurfaceTransform::HorizontalMirrorRotate90 ||
                        NewPreTransform == SurfaceTransform::HorizontalMirrorRotate270)
                    {
                        std::swap(NewWidth, NewHeight);
                    }
                }
            }
            else
            {
                QGFX_LOG_ERROR_MESSAGE(err, "Failed to query physical device surface capabilities");
            }
        }
#endif

        if (NewWidth != m_Width || NewHeight != m_Height || NewPreTransform != m_PreTransform)
        {
            m_Width = NewWidth;
            m_Height = NewHeight;
            m_DesiredPreTransform = NewPreTransform;

            bRecreateSwapChain = true;
        }

        if (bRecreateSwapChain)
        {
            QGFX_LOG_INFO_MESSAGE("Resizing swapchain to (", NewWidth, ",", NewHeight, ")");

            RecreateSwapChain();
        }
    }

    void SwapChainVk::CreateSurface()
	{
        if (m_VkSurface)
        {
            m_pRenderDevice->GetVkInstance().destroySurfaceKHR(m_VkSurface, nullptr, m_pRenderDevice->GetVkDispatch());
            m_VkSurface = nullptr;
        }

		try
		{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			if (m_Window.hWnd != NULL)
			{
				vk::Win32SurfaceCreateInfoKHR SurfaceCreateInfo = {};

				SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
				SurfaceCreateInfo.hwnd = (HWND)m_Window.hWnd;

				m_VkSurface = m_pRenderDevice->GetVkInstance().createWin32SurfaceKHR(SurfaceCreateInfo, nullptr, m_pRenderDevice->GetVkDispatch());
			}
#endif
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("vk::SurfaceKHR creation failed with error: ", Error.what());
		}
	}

	void SwapChainVk::CreateSwapChain()
	{

		vk::Device VkDevice = m_pRenderDevice->GetVkDevice();
		vk::PhysicalDevice VkPhDevice = m_pRenderDevice->GetVkPhysicalDevice();
        const vk::DispatchLoaderDynamic& VkDispatch = m_pRenderDevice->GetVkDispatch();
		
		std::vector<vk::SurfaceFormatKHR> SupportedFormats{};

		try
		{
			SupportedFormats = VkPhDevice.getSurfaceFormatsKHR(m_VkSurface, VkDispatch);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to query avalaible surface formats: ", Error.what());
		}

		m_VkColorFormat = TexFormatToVkFormat(m_TextureFormat);

        vk::ColorSpaceKHR ColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        if (SupportedFormats.size() == 1 && SupportedFormats[0].format == vk::Format::eUndefined)
        {
            // If the format list includes just one entry of vk::Format::eUndefined,
            // the surface has no preferred format.  Otherwise, at least one
            // supported format will be returned.

            // Do nothing
        }
        else
        {
            bool FmtFound = false;
            for (const auto& SrfFmt : SupportedFormats)
            {
                if (SrfFmt.format == m_VkColorFormat)
                {
                    FmtFound = true;
                    ColorSpace = SrfFmt.colorSpace;
                    break;
                }
            }
            if (!FmtFound)
            {
                vk::Format VkReplacementColorFormat = vk::Format::eUndefined;
                switch (m_VkColorFormat)
                {
                case vk::Format::eR8G8B8A8Unorm: VkReplacementColorFormat = vk::Format::eB8G8R8A8Unorm; break;
                case vk::Format::eB8G8R8A8Unorm: VkReplacementColorFormat = vk::Format::eR8G8B8A8Unorm; break;
                case vk::Format::eB8G8R8A8Srgb:  VkReplacementColorFormat = vk::Format::eR8G8B8A8Srgb;  break;
                case vk::Format::eR8G8B8A8Srgb:  VkReplacementColorFormat = vk::Format::eB8G8R8A8Srgb;  break;
                default: VkReplacementColorFormat = vk::Format::eUndefined;
                }

                bool ReplacementFmtFound = false;
                for (const auto& SrfFmt : SupportedFormats)
                {
                    if (SrfFmt.format == VkReplacementColorFormat)
                    {
                        ReplacementFmtFound = true;
                        ColorSpace = SrfFmt.colorSpace;
                        break;
                    }
                }

                if (ReplacementFmtFound)
                {
                    QGFX_LOG_INFO_MESSAGE("Requested color buffer format ", vk::to_string(m_VkColorFormat), " is not supported by the surface and will be replaced with ", vk::to_string(VkReplacementColorFormat));
                    m_VkColorFormat = VkReplacementColorFormat;
                    m_TextureFormat = VkFormatToTexFormat(VkReplacementColorFormat);
                }
                else
                {
                    //QGFX_LOG_WARNING_MESSAGE("Requested color buffer format ", GetTextureFormatAttribs(m_SwapChainDesc.ColorBufferFormat).Name, "is not supported by the surface");
                }
            }
        }

        vk::SurfaceCapabilitiesKHR SurfCapabilities = {};

        try
        {
            SurfCapabilities = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, VkDispatch);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
        }

        std::vector<vk::PresentModeKHR> PresentModes = {};

        try
        {
            PresentModes = VkPhDevice.getSurfacePresentModesKHR(m_VkSurface, VkDispatch);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
        }

        vk::SurfaceTransformFlagBitsKHR VkPreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        if (m_DesiredPreTransform != SurfaceTransform::eOptimal)
        {
            VkPreTransform = SurfaceTransformToVkSurfaceTransformFlag(m_DesiredPreTransform);
            if (SurfCapabilities.supportedTransforms & VkPreTransform)
            {
                m_PreTransform = m_DesiredPreTransform;
            }
            else
            {
                //LOG_WARNING_MESSAGE(GetSurfaceTransformString(m_DesiredPreTransform),
                //    " is not supported by the presentation engine. Optimal surface transform will be used instead."
                //    " Query the swap chain description to get the actual surface transform.");
                m_DesiredPreTransform = SurfaceTransform::eOptimal;
            }
        }

        if (m_DesiredPreTransform == SurfaceTransform::eOptimal)
        {
            // Use current surface transform to avoid extra cost of presenting the image.
            // If preTransform does not match the currentTransform value returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
            // the presentation engine will transform the image content as part of the presentation operation.
            // https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
            // https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation
            VkPreTransform = SurfCapabilities.currentTransform;
            m_PreTransform = VkSurfaceTransformFlagToSurfaceTransform(VkPreTransform);
            // QGFX_LOG_INFO_MESSAGE("Using ", GetSurfaceTransformString(m_SwapChainDesc.PreTransform), " swap chain pretransform");
        }

        VkExtent2D SwapchainExtent = {};
        // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
        if (SurfCapabilities.currentExtent.width == 0xFFFFFFFF && m_Width != 0 && m_Height != 0)
        {
            // If the surface size is undefined, the size is set to
            // the size of the images requested.
            SwapchainExtent.width = std::min(std::max(m_Width, SurfCapabilities.minImageExtent.width), SurfCapabilities.maxImageExtent.width);
            SwapchainExtent.height = std::min(std::max(m_Height, SurfCapabilities.minImageExtent.height), SurfCapabilities.maxImageExtent.height);
        }
        else
        {
            // If the surface size is defined, the swap chain size must match
            SwapchainExtent = SurfCapabilities.currentExtent;
        }

#if 0 // For Andriod
        // On Android, vkGetPhysicalDeviceSurfaceCapabilitiesKHR is not reliable and starts reporting incorrect
        // dimensions after few rotations. To alleviate the problem, we store the surface extent corresponding to
        // identity rotation.
        // https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
        if (m_SurfaceIdentityExtent.width == 0 || m_SurfaceIdentityExtent.height == 0)
        {
            m_SurfaceIdentityExtent = SurfCapabilities.currentExtent;
            constexpr auto Rotate90TransformFlags =
                vk::SurfaceTransformFlagBitsKHR::eRotate90 |
                vk::SurfaceTransformFlagBitsKHR::eRotate270 |
                vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90 |
                vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;
            if (SurfCapabilities.currentTransform & Rotate90TransformFlags)
                std::swap(m_SurfaceIdentityExtent.width, m_SurfaceIdentityExtent.height);
        }

        if (m_DesiredPreTransform == SurfaceTransform::Optimal)
        {
            SwapchainExtent = m_SurfaceIdentityExtent;
        }
        m_CurrentSurfaceTransform = SurfCapabilities.currentTransform;
#endif

        SwapchainExtent.width = std::max(SwapchainExtent.width, 1u);
        SwapchainExtent.height = std::max(SwapchainExtent.height, 1u);
        m_Width = SwapchainExtent.width;
        m_Height = SwapchainExtent.height;

        // The FIFO present mode is guaranteed by the spec to always be supported.
        vk::PresentModeKHR PresentMode = vk::PresentModeKHR::eFifo;
        {
            std::vector<vk::PresentModeKHR> PreferredPresentModes;
            if (m_bVSyncEnabled)
            {
                // FIFO relaxed waits for the next VSync, but if the frame is late,
                // it still shows it even if VSync has already passed, which may
                // result in tearing.
                PreferredPresentModes.push_back(vk::PresentModeKHR::eFifoRelaxed);
                PreferredPresentModes.push_back(vk::PresentModeKHR::eFifo);
            }
            else
            {
                // Mailbox is the lowest latency non-tearing presentation mode.
                PreferredPresentModes.push_back(vk::PresentModeKHR::eMailbox);
                PreferredPresentModes.push_back(vk::PresentModeKHR::eImmediate);
                PreferredPresentModes.push_back(vk::PresentModeKHR::eFifo);
            }

            for (auto PreferredMode : PreferredPresentModes)
            {
                if (std::find(PresentModes.begin(), PresentModes.end(), PreferredMode) != PresentModes.end())
                {
                    PresentMode = PreferredMode;
                    break;
                }
            }

            QGFX_LOG_INFO_MESSAGE("Using ", vk::to_string(PresentMode), " swap chain present mode");
        }

        // Determine the number of VkImage's to use in the swap chain.
        // We need to acquire only 1 presentable image at at time.
        // Asking for minImageCount images ensures that we can acquire
        // 1 presentable image as long as we present it before attempting
        // to acquire another.
        if (m_DesiredTextureCount < SurfCapabilities.minImageCount)
        {
            QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredTextureCount, ") is smaller than the minimal image count supported for this surface (", SurfCapabilities.minImageCount, "). Resetting to ", SurfCapabilities.minImageCount);
            m_DesiredTextureCount = SurfCapabilities.minImageCount;
        }
        if (SurfCapabilities.maxImageCount != 0 && m_DesiredTextureCount > SurfCapabilities.maxImageCount)
        {
            QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredTextureCount, ") is greater than the maximal image count supported for this surface (", SurfCapabilities.maxImageCount, "). Resetting to ", SurfCapabilities.maxImageCount);
            m_DesiredTextureCount = SurfCapabilities.maxImageCount;
        }

        // We must use m_DesiredBufferCount instead of m_SwapChainDesc.BufferCount, because Vulkan on Android
        // may decide to always add extra buffers, causing infinite growth of the swap chain when it is recreated:
        //                          m_SwapChainDesc.BufferCount
        // CreateVulkanSwapChain()          2 -> 4
        // CreateVulkanSwapChain()          4 -> 6
        // CreateVulkanSwapChain()          6 -> 8
        uint32_t DesiredNumberOfSwapChainImages = m_DesiredTextureCount;

        // Find a supported composite alpha mode - one of these is guaranteed to be set
        vk::CompositeAlphaFlagBitsKHR CompositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        vk::CompositeAlphaFlagBitsKHR CompositeAlphaFlags[4] = //
        {
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
            vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
            vk::CompositeAlphaFlagBitsKHR::eInherit,
        };
        for (uint32_t i = 0; i < 4; i++)
        {
            if (SurfCapabilities.supportedCompositeAlpha & CompositeAlphaFlags[i])
            {
                CompositeAlpha = CompositeAlphaFlags[i];
                break;
            }
        }

        vk::SwapchainKHR OldSwapchain = m_VkSwapchain;
        m_VkSwapchain = nullptr;

        vk::SwapchainCreateInfoKHR SwapChainCI = {};

        SwapChainCI.pNext = nullptr;
        SwapChainCI.surface = m_VkSurface;
        SwapChainCI.minImageCount = DesiredNumberOfSwapChainImages;
        SwapChainCI.imageFormat = m_VkColorFormat;
        SwapChainCI.imageExtent.width = SwapchainExtent.width;
        SwapChainCI.imageExtent.height = SwapchainExtent.height;
        SwapChainCI.preTransform = VkPreTransform;
        SwapChainCI.compositeAlpha = CompositeAlpha;
        SwapChainCI.imageArrayLayers = 1;
        SwapChainCI.presentMode = PresentMode;
        SwapChainCI.oldSwapchain = OldSwapchain;
        SwapChainCI.clipped = true;
        SwapChainCI.imageColorSpace = ColorSpace;

        //DEV_CHECK_ERR(m_SwapChainDesc.Usage != 0, "No swap chain usage flags defined");
        //static_assert(SWAP_CHAIN_USAGE_LAST == SWAP_CHAIN_USAGE_COPY_SOURCE, "Please update this function to handle the new swapchain usage");
        if (m_Usage & SwapChainUsageFlagBits::eRenderAttachment)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
        if (m_Usage & SwapChainUsageFlagBits::eSampled)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eSampled;
        if (m_Usage & SwapChainUsageFlagBits::eTransferSrc)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
        if (m_Usage & SwapChainUsageFlagBits::eTransferDst)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;

        // vkCmdClearColorImage() command requires the image to use VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL layout
        // that requires  VK_IMAGE_USAGE_TRANSFER_DST_BIT to be set
        SwapChainCI.imageSharingMode = vk::SharingMode::eExclusive;
        SwapChainCI.queueFamilyIndexCount = 0;
        SwapChainCI.pQueueFamilyIndices = nullptr;
        //uint32_t queueFamilyIndices[] = { (uint32_t)info.graphics_queue_family_index, (uint32_t)info.present_queue_family_index };
        //if (info.graphics_queue_family_index != info.present_queue_family_index) {
        //    // If the graphics and present queues are from different queue families,
        //    // we either have to explicitly transfer ownership of images between
        //    // the queues, or we have to create the swapchain with imageSharingMode
        //    // as VK_SHARING_MODE_CONCURRENT
        //    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        //    swapchain_ci.queueFamilyIndexCount = 2;
        //    swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
        //}

        try
        {
            m_VkSwapchain = VkDevice.createSwapchainKHR(SwapChainCI, nullptr, VkDispatch);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to create Vulkan swapchain: ", Error.what());
        }

        if (OldSwapchain)
        {
            VkDevice.destroySwapchainKHR(OldSwapchain, nullptr, VkDispatch);
        }

        try
        {
            vk::throwResultException(VkDevice.getSwapchainImagesKHR(m_VkSwapchain, &m_TextureCount, nullptr, VkDispatch), "Failed to get buffer count");
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to request swap chain image count: ", Error.what());
        }

        QGFX_VERIFY_EXPR(m_TextureCount > 0);


        if (m_DesiredTextureCount != m_TextureCount)
        {
            QGFX_LOG_INFO_MESSAGE("Created swap chain with ", m_TextureCount, " images vs ", m_DesiredTextureCount, " requested.");
        }

        m_bImageAcquired.resize(m_TextureCount);
        m_ImageAcquiredFences.resize(m_TextureCount);
        m_SubmitCompleteSemaphores.resize(m_TextureCount);
        m_FrameTextures.resize(m_TextureCount);

        TextureCreateInfo TexCI{};
        TexCI.Dimension = TextureDimension::e2D;
        TexCI.Width = m_Width;
        TexCI.Height = m_Height;
        TexCI.ArraySize = 1;
        TexCI.Format = m_TextureFormat;
        TexCI.MipLevels = 1;
        TexCI.SampleCount = TextureSampleCount::e1;

        if (m_Usage & SwapChainUsageFlagBits::eRenderAttachment)
            TexCI.Usage |= TextureUsageFlagBits::eRenderAttachment;
        if (m_Usage & SwapChainUsageFlagBits::eSampled)
            TexCI.Usage |= TextureUsageFlagBits::eSampled;
        if (m_Usage & SwapChainUsageFlagBits::eTransferSrc)
            TexCI.Usage |= TextureUsageFlagBits::eTransferSrc;
        if (m_Usage & SwapChainUsageFlagBits::eTransferDst)
            TexCI.Usage |= TextureUsageFlagBits::eTransferDst;

        TexCI.pInitialQueue = m_spCommandQueue;

        std::vector<vk::Image> m_VkImages(m_TextureCount);
        vk::throwResultException(VkDevice.getSwapchainImagesKHR(m_VkSwapchain, &m_TextureCount, m_VkImages.data(), VkDispatch), "Failed to retrieve vulkan swapchain images");

        for (uint32_t i = 0; i < m_TextureCount; ++i)
        {
            m_bImageAcquired[i] = false;

            vk::FenceCreateInfo FenceCI = {};

            FenceCI.pNext = nullptr;
            FenceCI.flags = {};

            m_ImageAcquiredFences[i] = VkDevice.createFence(FenceCI, nullptr, VkDispatch);

            vk::SemaphoreCreateInfo SemaphoreCI = {};

            SemaphoreCI.pNext = nullptr;
            SemaphoreCI.flags = {}; // reserved for future use

            m_SubmitCompleteSemaphores[i] = VkDevice.createSemaphore(SemaphoreCI, nullptr, VkDispatch);
            
            m_pRenderDevice->CreateTextureFromVkImage(TexCI, m_VkImages[m_TextureCount], &m_FrameTextures[m_TextureCount]);
        }
        m_SemaphoreIndex = m_TextureCount - 1;
	}

    //vk::Result SwapChainVk::AcquireNextImage()
    //{
    //    // Applications should not rely on vkAcquireNextImageKHR blocking in order to
    //    // meter their rendering speed. The implementation may return from this function
    //    // immediately regardless of how many presentation requests are queued, and regardless
    //    // of when queued presentation requests will complete relative to the call. Instead,
    //    // applications can use fence to meter their frame generation work to match the
    //    // presentation rate.

    //    // Explicitly make sure that there are no more pending frames in the command queue
    //    // than the number of the swap chain images.
    //    //
    //    // Nsc = 3 - number of the swap chain images
    //    //
    //    //   N-Ns          N-2           N-1            N (Current frame)
    //    //    |             |             |             |
    //    //                  |
    //    //          Wait for this fence
    //    //
    //    // When acquiring swap chain image for frame N, we need to make sure that
    //    // frame N-Nsc has completed. To achieve that, we wait for the image acquire
    //    // fence for frame N-Nsc-1. Thus we will have no more than Nsc frames in the queue.
    //    auto& OldestSubmittedSemaphoreChain = m_SemaphoreChains[(m_SemaphoreIndex + 1u) % m_BufferCount];
    //    if (OldestSubmittedSemaphoreChain.bAcquired)
    //    {
    //        vk::Fence OldestSubmittedFence = OldestSubmittedSemaphoreChain.AcquireFence;

    //        if (m_spRenderDevice->GetVkqDevice().getFenceStatus(OldestSubmittedFence) == vk::Result::eNotReady)
    //        {
    //            vk::Result Res = m_spRenderDevice->GetVkqDevice().waitForFences(OldestSubmittedFence, true, UINT64_MAX);
    //            QGFX_VERIFY_EXPR(Res = vk::Result::eSuccess);
    //            (void)Res;
    //        }

    //        m_spRenderDevice->GetVkqDevice().resetFence(OldestSubmittedFence);


    //        OldestSubmittedSemaphoreChain.bAcquired = false;
    //    }

    //    vk::Fence ImageAcquiredFence = m_SemaphoreChains[m_SemaphoreIndex].AcquireFence;
    //    vk::Semaphore ImageAcquiredSemaphore = m_SemaphoreChains[m_SemaphoreIndex].AcquireSemaphore;

    //    vk::Result Res = m_Swapchain.acquireNextImage(UINT64_MAX, ImageAcquiredSemaphore, ImageAcquiredFence, &m_TextureIndex);

    //    m_SemaphoreChains[m_SemaphoreIndex].bAcquired = (Res == vk::Result::eSuccess);
    //    m_SemaphoreChains[m_SemaphoreIndex].CurrentWaitSemaphore = m_SemaphoreChains[m_SemaphoreIndex].AcquireSemaphore;
    //    //if (res == VK_SUCCESS)
    //    //{
    //    //    // Next command in the device context must wait for the next image to be acquired.
    //    //    // Unlike fences or events, the act of waiting for a semaphore also unsignals that semaphore (6.4.2).
    //    //    // Swapchain image may be used as render target or as destination for copy command.
    //    //    pDeviceCtxVk->AddWaitSemaphore(m_ImageAcquiredSemaphores[m_SemaphoreIndex], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT);
    //    //    if (!m_SwapChainImagesInitialized[m_TextureIndex])
    //    //    {
    //    //        // Vulkan validation layers do not like uninitialized memory.
    //    //        // Clear back buffer first time we acquire it.

    //    //        ITextureView* pRTV = GetCurrentBackBufferRTV();
    //    //        ITextureView* pDSV = GetDepthBufferDSV();
    //    //        pDeviceCtxVk->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    //    //        pDeviceCtxVk->ClearRenderTarget(GetCurrentBackBufferRTV(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    //    //        m_SwapChainImagesInitialized[m_TextureIndex] = true;
    //    //    }
    //    //    pDeviceCtxVk->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_NONE);
    //    //}

    //    return Res;
    //}

    //void SwapChainVk::Acquire(bool bVsync)
    //{
    //    if (!m_bIsMinimized)
    //    {

    //        vk::Result Res = (bVsync == m_bVSyncEnabled) ? AcquireNextImage() : vk::Result::eErrorOutOfDateKHR;

    //        if (Res == vk::Result::eSuboptimalKHR || Res == vk::Result::eErrorOutOfDateKHR)
    //        {
    //            m_bVSyncEnabled = bVsync;
    //            RecreateSwapChain();
    //            m_SemaphoreIndex = m_BufferCount - 1; // To start with 0 index when acquire next image

    //            Res = AcquireNextImage();
    //        }
    //        QGFX_VERIFY(Res == vk::Result::eSuccess, "Failed to acquire next swap chain image");
    //    }
    //}

    void SwapChainVk::WaitForImageAcquiredFences()
    {
        auto& VkDevice = m_pRenderDevice->GetVkDevice();
        auto& VkDispatch = m_pRenderDevice->GetVkDispatch();

        for (auto Fence : m_ImageAcquiredFences)
        {
            if (VkDevice.getFenceStatus(Fence, VkDispatch) == vk::Result::eNotReady)
                VkDevice.waitForFences(1, &Fence, true, UINT64_MAX, VkDispatch);
        }
    }


    void SwapChainVk::ReleaseSwapChainResources(bool bDestroySwapChain)
    {
        if (!m_VkSwapchain)
            return;

        auto& VkDevice = m_pRenderDevice->GetVkDevice();
        auto& VkDispatch = m_pRenderDevice->GetVkDispatch();

        m_pRenderDevice->WaitIdle();

        WaitForImageAcquiredFences();

        for (auto Texture : m_FrameTextures)
        {
            Texture->Release();
        }

        for (auto Fence : m_ImageAcquiredFences)
        {
            VkDevice.destroyFence(Fence, nullptr, VkDispatch);
        }

        for (auto Semaphore : m_SubmitCompleteSemaphores)
        {
            VkDevice.destroySemaphore(Semaphore, nullptr, VkDispatch);
        }

        m_FrameTextures.clear();
        m_bImageAcquired.clear();
        m_ImageAcquiredFences.clear();
        m_SubmitCompleteSemaphores.clear();

        m_SemaphoreIndex = 0;

        if (bDestroySwapChain)
        {
            VkDevice.destroySwapchainKHR(m_VkSwapchain, nullptr, VkDispatch);
        }
    }

    void SwapChainVk::RecreateSwapChain()
    {
        ReleaseSwapChainResources(false);

        // Check if the surface is lost
        {
            auto& VkPhDevice = m_pRenderDevice->GetVkPhysicalDevice();
            auto& VkDevice = m_pRenderDevice->GetVkDevice();
            auto& VkDispatch = m_pRenderDevice->GetVkDispatch();

            try
            {
                vk::SurfaceCapabilitiesKHR SurfCapabilities = VkPhDevice.getSurfaceCapabilitiesKHR(m_VkSurface, VkDispatch);
                (void)SurfCapabilities;
            }
            catch (const vk::SurfaceLostKHRError&)
            {
                if (m_VkSwapchain)
                {
                    VkDevice.destroySwapchainKHR(m_VkSwapchain, nullptr, VkDispatch);
                }

                // Recreate the surface
                CreateSurface();
            }
            catch (const vk::SystemError& Error)
            {
                QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
            }
        }

        CreateSwapChain();
    }
}