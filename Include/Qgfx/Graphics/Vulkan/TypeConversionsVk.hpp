#pragma once

#include "BaseVk.hpp"

#include "../GraphicsTypes.hpp"

namespace Qgfx
{
	vk::Format TexFormatToVkFormat(TextureFormat TexFmt);

	TextureFormat VkFormatToTexFormat(vk::Format VkFmt);

	vk::SurfaceTransformFlagBitsKHR SurfaceTransformToVkSurfaceTransformFlag(SurfaceTransform SrfTransform);

	SurfaceTransform VkSurfaceTransformFlagToSurfaceTransform(vk::SurfaceTransformFlagBitsKHR vkTransformFlag);
}