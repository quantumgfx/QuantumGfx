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
		case Qgfx::TextureFormat::Depth32Float:       return vk::Format::eD32Sfloat;
		case Qgfx::TextureFormat::Depth32FloatStencil8Uint: return vk::Format::eD32SfloatS8Uint;
		case Qgfx::TextureFormat::Depth24PlusStencil8Uint: return vk::Format::eD24UnormS8Uint;
		case Qgfx::TextureFormat::Depth16Unorm:       return vk::Format::eD16Unorm;
		case Qgfx::TextureFormat::Depth16UnormStencil8Uint: return vk::Format::eD16UnormS8Uint;
		default: return vk::Format::eUndefined;
		}
	}

	TextureFormat VkFormatToTexFormat(vk::Format VkFmt)
	{
	}

}