#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	struct TextureViewCreateInfo
	{
		TextureFormat Format;
		TextureAspectFlags Aspect;
		TextureViewDimension Dimension;
		uint32_t BaseMipLevel;
		uint32_t MipLevelCount;
		uint32_t BaseArrayLayer;
		uint32_t ArrayLayerCount;
	};

	class ITextureView : public IObject
	{
	public:

		ITextureView(IRefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual TextureAspectFlags GetAspect() = 0;
		virtual TextureViewDimension GetDimension() = 0;
		virtual TextureFormat GetFormat() = 0;
		virtual uint32_t GetBaseMipLevel() = 0;
		virtual uint32_t GetLevelCount() = 0;
		virtual uint32_t GetBaseArrayLayer() = 0;
		virtual uint32_t GetLayerCount() = 0;
	};
}