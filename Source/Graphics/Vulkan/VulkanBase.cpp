#define VMA_IMPLEMENTATION

#include "Qgfx/Graphics/Vulkan/VulkanBase.hpp"

namespace Qgfx
{
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
