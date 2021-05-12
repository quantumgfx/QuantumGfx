#define VMA_IMPLEMENTATION

#include "Qgfx/Graphics/Vulkan/VulkanBase.hpp"

namespace Qgfx
{
	vk::BufferUsageFlags VulkanConversion::GetVkBufferUsage(ResourceUsageFlags Usage)
	{
		vk::BufferUsageFlags BufferUsage = {};
		if (Usage & ResourceUsageFlagBits::eVertexBuffer)
			BufferUsage |= vk::BufferUsageFlagBits::eVertexBuffer;
		if (Usage & ResourceUsageFlagBits::eIndexBuffer)
			BufferUsage |= vk::BufferUsageFlagBits::eIndexBuffer;
		if (Usage & ResourceUsageFlagBits::eIndirectBuffer)
			BufferUsage |= vk::BufferUsageFlagBits::eIndirectBuffer;
		if (Usage & ResourceUsageFlagBits::eUniformBuffer)
			BufferUsage |= vk::BufferUsageFlagBits::eUniformBuffer;
		if (Usage & ResourceUsageFlagBits::eStorageBuffer)
			BufferUsage |= vk::BufferUsageFlagBits::eStorageBuffer;
		if (Usage & ResourceUsageFlagBits::eSampledImage)
			BufferUsage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
		if (Usage & ResourceUsageFlagBits::eStorageImage)
			BufferUsage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
		if (Usage & ResourceUsageFlagBits::eTransferSrc)
			BufferUsage |= vk::BufferUsageFlagBits::eTransferSrc;
		if (Usage & ResourceUsageFlagBits::eTransferDst)
			BufferUsage |= vk::BufferUsageFlagBits::eTransferDst;

		return BufferUsage;
	}

	vk::ImageUsageFlags VulkanConversion::GetVkImageUsage(ResourceUsageFlags Usage)
	{
		vk::ImageUsageFlags ImageUsage = {};
		if (Usage & ResourceUsageFlagBits::eRenderAttachment)
			ImageUsage |= vk::ImageUsageFlagBits::eColorAttachment;
		if (Usage & ResourceUsageFlagBits::eDepthStencilAttachment)
			ImageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		if (Usage & ResourceUsageFlagBits::eSampledImage)
			ImageUsage |= vk::ImageUsageFlagBits::eSampled;
		if (Usage & ResourceUsageFlagBits::eStorageImage)
			ImageUsage |= vk::ImageUsageFlagBits::eStorage;
		if (Usage & ResourceUsageFlagBits::eTransferSrc)
			ImageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
		if (Usage & ResourceUsageFlagBits::eTransferDst)
			ImageUsage |= vk::ImageUsageFlagBits::eTransferDst;

		return ImageUsage;
	}

	ResourceUsageFlags VulkanConversion::GetResourceUsage(vk::BufferUsageFlags BufferUsage)
	{
		ResourceUsageFlags Usage = {};
		if (BufferUsage & vk::BufferUsageFlagBits::eVertexBuffer)
			Usage |= ResourceUsageFlagBits::eVertexBuffer;
		if (BufferUsage & vk::BufferUsageFlagBits::eIndexBuffer)
			Usage |= ResourceUsageFlagBits::eIndexBuffer;
		if (BufferUsage & vk::BufferUsageFlagBits::eIndirectBuffer)
			Usage |= ResourceUsageFlagBits::eIndirectBuffer;
		if (BufferUsage & vk::BufferUsageFlagBits::eUniformBuffer)
			Usage |= ResourceUsageFlagBits::eUniformBuffer;
		if (BufferUsage & vk::BufferUsageFlagBits::eStorageBuffer)
			Usage |= ResourceUsageFlagBits::eStorageBuffer;
		if (BufferUsage & vk::BufferUsageFlagBits::eUniformTexelBuffer)
			Usage |= ResourceUsageFlagBits::eSampledImage;
		if (BufferUsage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
			Usage |= ResourceUsageFlagBits::eStorageImage;
		if (BufferUsage & vk::BufferUsageFlagBits::eTransferSrc)
			Usage |= ResourceUsageFlagBits::eTransferSrc;
		if (BufferUsage & vk::BufferUsageFlagBits::eTransferDst)
			Usage |= ResourceUsageFlagBits::eTransferDst;

		return Usage;
	}

	ResourceUsageFlags VulkanConversion::GetResourceUsage(vk::ImageUsageFlags ImageUsage)
	{
		ResourceUsageFlags Usage = {};
		if (ImageUsage & vk::ImageUsageFlagBits::eColorAttachment)
			Usage |= ResourceUsageFlagBits::eRenderAttachment;
		if (ImageUsage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
			Usage |= ResourceUsageFlagBits::eDepthStencilAttachment;
		if (ImageUsage & vk::ImageUsageFlagBits::eSampled)
			Usage |= ResourceUsageFlagBits::eSampledImage;
		if (ImageUsage & vk::ImageUsageFlagBits::eStorage)
			Usage |= ResourceUsageFlagBits::eStorageImage;
		if (ImageUsage & vk::ImageUsageFlagBits::eTransferSrc)
			Usage |= ResourceUsageFlagBits::eTransferSrc;
		if (ImageUsage & vk::ImageUsageFlagBits::eTransferDst)
			Usage |= ResourceUsageFlagBits::eTransferDst;

		return Usage;
	}

	vk::ClearValue VulkanConversion::GetVkClearValue(ClearValue Clear, TextureFormat Fmt)
	{
		TextureFormatParams Params = TextureFormatUtils::GetParams(Fmt);
		
		if (Params.ColorDataType == TextureFormatComponentType::eUnorm)
		{

		}

		vk::ClearValue VkValue;

		if (Params.ColorDataType != TextureFormatComponentType::eNone)
		{
			switch (Params.ColorDataType)
			{
			case TextureFormatComponentType::eUnorm:
			case TextureFormatComponentType::eSnorm:
			case TextureFormatComponentType::eFloat:
			{
				VkValue.color.float32[0] = static_cast<float>(Clear.R);
				VkValue.color.float32[1] = static_cast<float>(Clear.G);
				VkValue.color.float32[2] = static_cast<float>(Clear.B);
				VkValue.color.float32[3] = static_cast<float>(Clear.A);
				break;
			}
			case TextureFormatComponentType::eUint:
			{
				VkValue.color.uint32[0] = static_cast<uint32_t>(Clear.R);
				VkValue.color.uint32[1] = static_cast<uint32_t>(Clear.G);
				VkValue.color.uint32[2] = static_cast<uint32_t>(Clear.B);
				VkValue.color.uint32[3] = static_cast<uint32_t>(Clear.A);
				break;
			}
			case TextureFormatComponentType::eSint:
			{
				VkValue.color.int32[0] = static_cast<int32_t>(Clear.R);
				VkValue.color.int32[1] = static_cast<int32_t>(Clear.G);
				VkValue.color.int32[2] = static_cast<int32_t>(Clear.B);
				VkValue.color.int32[3] = static_cast<int32_t>(Clear.A);
				break;
			}

			default:
				QGFX_UNEXPECTED("Exepected format");
				break;
			}

			return VkValue;
		}
		else if (Params.DepthDataType != TextureFormatComponentType::eNone || Params.StencilDataType != TextureFormatComponentType::eNone)
		{
			VkValue.depthStencil.depth = Clear.Depth;
			VkValue.depthStencil.stencil = Clear.Stencil;
			return VkValue;
		}
		else
		{
			QGFX_UNEXPECTED("Exepected format");
			return VkValue;
		}
	}

	vk::Format VulkanConversion::GetColorVkFormat(TextureFormat ColorFmt)
	{
		switch (ColorFmt)
		{
		case Qgfx::TextureFormat::eR8Unorm:   return vk::Format::eR8Unorm;
		case Qgfx::TextureFormat::eR8Snorm:   return vk::Format::eR8Snorm;
		case Qgfx::TextureFormat::eR8Uint:    return vk::Format::eR8Uint;
		case Qgfx::TextureFormat::eR8Sint:    return vk::Format::eR8Sint;
		case Qgfx::TextureFormat::eR16Float:  return vk::Format::eR16Sfloat;
		case Qgfx::TextureFormat::eR16Unorm:  return vk::Format::eR16Unorm;
		case Qgfx::TextureFormat::eR16Snorm:  return vk::Format::eR16Snorm;
		case Qgfx::TextureFormat::eR16Uint:   return vk::Format::eR16Uint;
		case Qgfx::TextureFormat::eR16Sint:   return vk::Format::eR16Sint;
		case Qgfx::TextureFormat::eRG8Unorm:  return vk::Format::eR8G8Unorm;
		case Qgfx::TextureFormat::eRG8Snorm:  return vk::Format::eR8G8Snorm;
		case Qgfx::TextureFormat::eRG8Uint:   return vk::Format::eR8G8Uint;
		case Qgfx::TextureFormat::eRG8Sint:   return vk::Format::eR8G8Sint;
		case Qgfx::TextureFormat::eR32Float:  return vk::Format::eR32Sfloat;
		case Qgfx::TextureFormat::eR32Uint:   return vk::Format::eR32Uint;
		case Qgfx::TextureFormat::eR32Sint:   return vk::Format::eR32Sint;
		case Qgfx::TextureFormat::eRG16Float: return vk::Format::eR16G16Sfloat;
		case Qgfx::TextureFormat::eRG16Unorm: return vk::Format::eR16G16Unorm;
		case Qgfx::TextureFormat::eRG16Snorm: return vk::Format::eR16G16Snorm;
		case Qgfx::TextureFormat::eRG16Uint:  return vk::Format::eR16G16Uint;
		case Qgfx::TextureFormat::eRG16Sint:  return vk::Format::eR16G16Sint;
		case Qgfx::TextureFormat::eRGBA8Unorm: return vk::Format::eR8G8B8A8Unorm;
		case Qgfx::TextureFormat::eRGBA8Snorm: return vk::Format::eR8G8B8A8Snorm;
		case Qgfx::TextureFormat::eRGBA8Uint:  return vk::Format::eR8G8B8A8Uint;
		case Qgfx::TextureFormat::eRGBA8Sint:  return vk::Format::eR8G8B8A8Sint;
		case Qgfx::TextureFormat::eRGBA8UnormSrgb: return vk::Format::eR8G8B8A8Srgb;
		case Qgfx::TextureFormat::eBGRA8Unorm:     return vk::Format::eB8G8R8A8Unorm;
		case Qgfx::TextureFormat::eBGRA8UnormSrgb: return vk::Format::eB8G8R8A8Srgb;
		case Qgfx::TextureFormat::eRGB10A2Unorm:   return vk::Format::eA2R10G10B10UnormPack32;
		case Qgfx::TextureFormat::eRGB10A2Uint:    return vk::Format::eA2R10G10B10UintPack32;
		case Qgfx::TextureFormat::eR11G11B10Float: break;
		case Qgfx::TextureFormat::eRG32Float:      return vk::Format::eR32G32Sfloat;
		case Qgfx::TextureFormat::eRG32Uint:       return vk::Format::eR32G32Uint;
		case Qgfx::TextureFormat::eRG32Sint:       return vk::Format::eR32G32Sint;
		case Qgfx::TextureFormat::eRGBA16Float:    return vk::Format::eR16G16B16A16Sfloat;
		case Qgfx::TextureFormat::eRGBA16Uint:     return vk::Format::eR16G16B16A16Uint;
		case Qgfx::TextureFormat::eRGBA16Sint:     return vk::Format::eR16G16B16A16Sint;
		case Qgfx::TextureFormat::eRGBA32Float:    return vk::Format::eR32G32B32A32Sfloat;
		case Qgfx::TextureFormat::eRGBA32Uint:     return vk::Format::eR32G32B32A32Uint;
		case Qgfx::TextureFormat::eRGBA32Sint:     return vk::Format::eR32G32B32A32Sint;

		case Qgfx::TextureFormat::eStencil8Uint:
		case Qgfx::TextureFormat::eDepth16Unorm:
		case Qgfx::TextureFormat::eDepth24Plus:
		case Qgfx::TextureFormat::eDepth24PlusStencil8Uint:
		case Qgfx::TextureFormat::eDepth32Float:
		case Qgfx::TextureFormat::eDepth16UnormStencil8Uint:
		case Qgfx::TextureFormat::eDepth32FloatStencil8Uint:
			QGFX_UNSUPPORTED("DepthStencil formats not convertable with GetColorVkFormat()");
			return vk::Format::eUndefined;
		default:
			QGFX_UNEXPECTED("Unrecognized format");
			return vk::Format::eUndefined;
		}
	}

	vk::SurfaceTransformFlagBitsKHR VulkanConversion::GetVkSurfaceTransformKHR(SurfaceTransform Transform)
	{
		switch (Transform)
		{
		case SurfaceTransform::eOptimal:
			QGFX_UNEXPECTED("Optimal transform does not have corresponding Vulkan flag");
			return vk::SurfaceTransformFlagBitsKHR::eIdentity;

		case SurfaceTransform::eIdentity:                   return vk::SurfaceTransformFlagBitsKHR::eIdentity;
		case SurfaceTransform::eRotate90:                   return vk::SurfaceTransformFlagBitsKHR::eRotate90;
		case SurfaceTransform::eRotate180:                  return vk::SurfaceTransformFlagBitsKHR::eRotate180;
		case SurfaceTransform::eRotate270:                  return vk::SurfaceTransformFlagBitsKHR::eRotate270;
		case SurfaceTransform::eHorizontalMirror:           return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror;
		case SurfaceTransform::eHorizontalMirrorRotate90:   return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90;
		case SurfaceTransform::eHorizontalMirrorRotate180:  return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180;
		case SurfaceTransform::eHorizontalMirrorRotate270:  return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;

		default:
			QGFX_UNEXPECTED("Unexpected surface transform");
			return vk::SurfaceTransformFlagBitsKHR::eIdentity;
		}
	}

	SurfaceTransform VulkanConversion::GetSurfaceTransform(vk::SurfaceTransformFlagBitsKHR Transform)
	{
		//QGFX_VERIFY(IsPowerOfTwo(static_cast<Uint32>(vkTransformFlag)), "Only single transform bit is expected");

		switch (Transform)
		{
		case vk::SurfaceTransformFlagBitsKHR::eIdentity:                  return SurfaceTransform::eIdentity;
		case vk::SurfaceTransformFlagBitsKHR::eRotate90:                  return SurfaceTransform::eRotate90;
		case vk::SurfaceTransformFlagBitsKHR::eRotate180:                 return SurfaceTransform::eRotate180;
		case vk::SurfaceTransformFlagBitsKHR::eRotate270:                 return SurfaceTransform::eRotate270;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror:          return SurfaceTransform::eHorizontalMirror;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90:  return SurfaceTransform::eHorizontalMirrorRotate90;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180: return SurfaceTransform::eHorizontalMirrorRotate180;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270: return SurfaceTransform::eHorizontalMirrorRotate270;

		default:
			QGFX_UNEXPECTED("Unexpected surface transform");
			return SurfaceTransform::eIdentity;
		}
	}
}
