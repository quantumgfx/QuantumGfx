#include "Qgfx/Graphics/Vulkan/TypeConversionsVk.hpp"
#include "Qgfx/Common/Error.hpp"

namespace Qgfx
{
	vk::Format TexFormatToVkFormat(TextureFormat TexFmt)
	{
		switch (TexFmt)
		{
		case Qgfx::TextureFormat::RGBA32Float: return vk::Format::eR32G32B32A32Sfloat;
		case Qgfx::TextureFormat::RGBA32Uint:  return vk::Format::eR32G32B32A32Uint;
		case Qgfx::TextureFormat::RGBA32Sint:  return vk::Format::eR32G32B32A32Sint;
		case Qgfx::TextureFormat::RGBA16Float: return vk::Format::eR16G16B16A16Sfloat;
		case Qgfx::TextureFormat::RGBA16Uint:  return vk::Format::eR16G16B16A16Uint;
		case Qgfx::TextureFormat::RGBA16Sint:  return vk::Format::eR16G16B16A16Sint;
		case Qgfx::TextureFormat::RG32Float:   return vk::Format::eR32G32Sfloat;
		case Qgfx::TextureFormat::RG32Uint:    return vk::Format::eR32G32Uint;
		case Qgfx::TextureFormat::RG32Sint:    return vk::Format::eR32G32Sint;
		case Qgfx::TextureFormat::RGB10A2Unorm: 
		case Qgfx::TextureFormat::RGB10A2Uint:
		case Qgfx::TextureFormat::R11G11B10Float:
		case Qgfx::TextureFormat::RGBA8Unorm: return vk::Format::eR8G8B8A8Unorm;
		case Qgfx::TextureFormat::RGBA8Snorm: return vk::Format::eR8G8B8A8Snorm;
		case Qgfx::TextureFormat::RGBA8Uint:  return vk::Format::eR8G8B8A8Uint;
		case Qgfx::TextureFormat::RGBA8Sint:  return vk::Format::eR8G8B8A8Sint;
		case Qgfx::TextureFormat::RGBA8UnormSrgb: return vk::Format::eR8G8B8A8Srgb;
		case Qgfx::TextureFormat::RG16Float:
			break;
		case Qgfx::TextureFormat::RG16Unorm:
			break;
		case Qgfx::TextureFormat::RG16Snorm:
			break;
		case Qgfx::TextureFormat::RG16Uint:
			break;
		case Qgfx::TextureFormat::RG16Sint:
			break;
		case Qgfx::TextureFormat::R32Float:
			break;
		case Qgfx::TextureFormat::R32Uint:
			break;
		case Qgfx::TextureFormat::R32Sint:
			break;
		case Qgfx::TextureFormat::RG8Unorm:
			break;
		case Qgfx::TextureFormat::RG8Snorm:
			break;
		case Qgfx::TextureFormat::RG8Uint:
			break;
		case Qgfx::TextureFormat::RG8Sint:
			break;
		case Qgfx::TextureFormat::R16Float:
			break;
		case Qgfx::TextureFormat::R16Unorm:
			break;
		case Qgfx::TextureFormat::R16Snorm:
			break;
		case Qgfx::TextureFormat::R16Uint:
			break;
		case Qgfx::TextureFormat::R16Sint:
			break;
		case Qgfx::TextureFormat::R8Unorm:
			break;
		case Qgfx::TextureFormat::R8Snorm:
			break;
		case Qgfx::TextureFormat::R8Uint:
			break;
		case Qgfx::TextureFormat::R8Sint:
			break;
		case Qgfx::TextureFormat::D32Float:       return vk::Format::eD32Sfloat;
		case Qgfx::TextureFormat::D32FloatS8Uint: return vk::Format::eD32SfloatS8Uint;
		case Qgfx::TextureFormat::D24UnormS8Uint: return vk::Format::eD24UnormS8Uint;
		case Qgfx::TextureFormat::D16Unorm:       return vk::Format::eD16Unorm;
		case Qgfx::TextureFormat::D16UnormS8Uint: return vk::Format::eD16UnormS8Uint;
		default: return vk::Format::eUndefined;
		}
	}

	TextureFormat VkFormatToTexFormat(vk::Format VkFmt)
	{
	}

	vk::SurfaceTransformFlagBitsKHR SurfaceTransformToVkSurfaceTransformFlag(SurfaceTransform SrfTransform)
	{
		switch (SrfTransform)
		{
		case SurfaceTransform::Optimal:
			QGFX_UNEXPECTED("Optimal transform does not have corresponding Vulkan flag");
			return vk::SurfaceTransformFlagBitsKHR::eIdentity;

		case SurfaceTransform::Identity:                   return vk::SurfaceTransformFlagBitsKHR::eIdentity;
		case SurfaceTransform::Rotate90:                   return vk::SurfaceTransformFlagBitsKHR::eRotate90;
		case SurfaceTransform::Rotate180:                  return vk::SurfaceTransformFlagBitsKHR::eRotate180;
		case SurfaceTransform::Rotate270:                  return vk::SurfaceTransformFlagBitsKHR::eRotate270;
		case SurfaceTransform::HorizontalMirror:           return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror;
		case SurfaceTransform::HorizontalMirrorRotate90:   return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90;
		case SurfaceTransform::HorizontalMirrorRotate180:  return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180;
		case SurfaceTransform::HorizontalMirrorRotate270:  return vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270;

		default:
			QGFX_UNEXPECTED("Unexpected surface transform");
			return vk::SurfaceTransformFlagBitsKHR::eIdentity;
		}
	}

	SurfaceTransform VkSurfaceTransformFlagToSurfaceTransform(vk::SurfaceTransformFlagBitsKHR VkTransformFlag)
	{
		//QGFX_VERIFY(IsPowerOfTwo(static_cast<Uint32>(vkTransformFlag)), "Only single transform bit is expected");

		switch (VkTransformFlag)
		{
		case vk::SurfaceTransformFlagBitsKHR::eIdentity:                  return SurfaceTransform::Identity;
		case vk::SurfaceTransformFlagBitsKHR::eRotate90:                  return SurfaceTransform::Rotate90;
		case vk::SurfaceTransformFlagBitsKHR::eRotate180:                 return SurfaceTransform::Rotate180;
		case vk::SurfaceTransformFlagBitsKHR::eRotate270:                 return SurfaceTransform::Rotate270;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirror:          return SurfaceTransform::HorizontalMirror;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate90:  return SurfaceTransform::HorizontalMirrorRotate90;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate180: return SurfaceTransform::HorizontalMirrorRotate180;
		case vk::SurfaceTransformFlagBitsKHR::eHorizontalMirrorRotate270: return SurfaceTransform::HorizontalMirrorRotate270;

		default:
			QGFX_UNEXPECTED("Unexpected surface transform");
			return SurfaceTransform::Identity;
		}
	}
}