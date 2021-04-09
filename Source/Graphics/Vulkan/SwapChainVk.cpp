#include "Qgfx/Graphics/Vulkan/SwapChainVk.hpp"
#include "Qgfx/Graphics/Vulkan/TypeConversionsVk.hpp"
#include "Qgfx/Common/Error.hpp"

namespace Qgfx
{
	SwapChainVk::SwapChainVk(RefCounter* pRefCounter, const SwapChainCreateInfo& CreateInfo, const NativeWindow& Window, RenderContextVk* pRenderContext, RenderDeviceVk* pRenderDevice)
		: ISwapChain(pRefCounter)
	{
		m_spRenderDevice = pRenderDevice;
	}

	SwapChainVk::~SwapChainVk()
	{

	}

	void SwapChainVk::CreateSurface()
	{
		try
		{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			if (m_Window.hWnd != NULL)
			{
				vk::Win32SurfaceCreateInfoKHR SurfaceCreateInfo = {};

				SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
				SurfaceCreateInfo.hwnd = (HWND)m_Window.hWnd;

				m_VkSurface = m_spRenderDevice->GetVkqInstance().createWin32SurfaceKHR(SurfaceCreateInfo);
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

		vkq::Device Dev = m_spRenderDevice->GetVkqDevice();
		vkq::PhysicalDevice PhDev = m_spRenderDevice->GetVkqPhysicalDevice();
		
		std::vector<vk::SurfaceFormatKHR> SupportedFormats{};

		try
		{
			SupportedFormats = PhDev.getSurfaceFormatsKHR(m_VkSurface);
		}
		catch (const vk::SystemError& Error)
		{
			QGFX_LOG_ERROR_AND_THROW("Failed to query avalaible surface formats: ", Error.what());
		}

		m_VkColorFormat = TexFormatToVkFormat(m_ColorBufferFormat);

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
                    m_ColorBufferFormat = VkFormatToTexFormat(VkReplacementColorFormat);
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
            SurfCapabilities = PhDev.getSurfaceCapabilitiesKHR(m_VkSurface);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
        }

        std::vector<vk::PresentModeKHR> PresentModes = {};

        try
        {
            PresentModes = PhDev.getSurfacePresentModes(m_VkSurface);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to query physical device surface capabilities: ", Error.what());
        }

        vk::SurfaceTransformFlagBitsKHR VkPreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        if (m_DesiredPreTransform != SurfaceTransform::Optimal)
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
                m_DesiredPreTransform = SurfaceTransform::Optimal;
            }
        }

        if (m_DesiredPreTransform == SurfaceTransform::Optimal)
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

#if 1 // For Andriod
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
        if (m_DesiredBufferCount < SurfCapabilities.minImageCount)
        {
            QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredBufferCount, ") is smaller than the minimal image count supported for this surface (", SurfCapabilities.minImageCount, "). Resetting to ", SurfCapabilities.minImageCount);
            m_DesiredBufferCount = SurfCapabilities.minImageCount;
        }
        if (SurfCapabilities.maxImageCount != 0 && m_DesiredBufferCount > SurfCapabilities.maxImageCount)
        {
            QGFX_LOG_INFO_MESSAGE("Desired back buffer count (", m_DesiredBufferCount, ") is greater than the maximal image count supported for this surface (", SurfCapabilities.maxImageCount, "). Resetting to ", SurfCapabilities.maxImageCount);
            m_DesiredBufferCount = SurfCapabilities.maxImageCount;
        }

        // We must use m_DesiredBufferCount instead of m_SwapChainDesc.BufferCount, because Vulkan on Android
        // may decide to always add extra buffers, causing infinite growth of the swap chain when it is recreated:
        //                          m_SwapChainDesc.BufferCount
        // CreateVulkanSwapChain()          2 -> 4
        // CreateVulkanSwapChain()          4 -> 6
        // CreateVulkanSwapChain()          6 -> 8
        uint32_t DesiredNumberOfSwapChainImages = m_DesiredBufferCount;

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

        vk::SwapchainKHR OldSwapchain = m_VkSwapChain;
        m_VkSwapChain = nullptr;

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
        SwapChainCI.clipped = VK_TRUE;
        SwapChainCI.imageColorSpace = ColorSpace;

        //DEV_CHECK_ERR(m_SwapChainDesc.Usage != 0, "No swap chain usage flags defined");
        //static_assert(SWAP_CHAIN_USAGE_LAST == SWAP_CHAIN_USAGE_COPY_SOURCE, "Please update this function to handle the new swapchain usage");
        if (m_Usage & SwapChainUsageFlagBits::RenderTarget)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
        if (m_Usage & SwapChainUsageFlagBits::ShaderInput)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eSampled;
        if (m_Usage & SwapChainUsageFlagBits::TransferSrc)
            SwapChainCI.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
        if (m_Usage & SwapChainUsageFlagBits::TransferDst)
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
            m_VkSwapChain = Dev.createSwapchainKHR(SwapChainCI);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to create Vulkan swapchain: ", Error.what());
        }

        if (OldSwapchain)
        {
            Dev.destroySwapchainKHR(OldSwapchain);
            OldSwapchain = nullptr;
        }

        try
        {
            m_BufferCount = Dev.getSwapchainImageCountKHR(m_VkSwapChain);
        }
        catch (const vk::SystemError& Error)
        {
            QGFX_LOG_ERROR_AND_THROW("Failed to request swap chain image count: ", Error.what());
        }

        QGFX_VERIFY_EXPR(m_BufferCount > 0);


        if (m_DesiredBufferCount != m_BufferCount)
        {
            QGFX_LOG_INFO_MESSAGE("Created swap chain with ", m_BufferCount, " images vs ", m_DesiredBufferCount, " requested.");
        }

        m_ImageAcquiredSemaphores.resize(m_BufferCount);
        m_DrawCompleteSemaphores.resize(m_BufferCount);
        m_ImageAcquiredFences.resize(m_BufferCount);
        for (uint32_t i = 0; i < m_BufferCount; ++i)
        {
            vk::SemaphoreCreateInfo SemaphoreCI = {};

            SemaphoreCI.pNext = nullptr;
            SemaphoreCI.flags = {}; // reserved for future use

            m_ImageAcquiredSemaphores[i] = Dev.createSemaphore(SemaphoreCI);
            m_DrawCompleteSemaphores[i] = Dev.createSemaphore(SemaphoreCI);

            vk::FenceCreateInfo FenceCI = {};

            FenceCI.pNext = nullptr;
            FenceCI.flags = {};

            m_ImageAcquiredFences[i] = Dev.createFence(FenceCI);
        }
	}
}