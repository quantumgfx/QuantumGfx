#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"

namespace Qgfx
{
	enum class TextureAspectFlagBits
	{
		eNone = 0x00,
		eColor = 0x01,
		eDepth = 0x02,
		eStencil = 0x04,
		eDepthStencil = eDepth | eStencil,
		eAll = eColor | eDepth | eStencil,
	};

	template<>
	struct EnableEnumFlags<TextureAspectFlagBits>
	{
		static const bool bEnabled = true;
	};

	using TextureAspectFlags = Flags<TextureAspectFlagBits>;

	enum class TextureViewDimension
	{
		e1D = 0,
		e1DArray,
		e2D,
		e2DArray,
		eCube,
		eCubeArray,
		e3D
	};

	struct TextureViewCreateInfo
	{
		TextureFormat Format;
		TextureAspectFlags Aspect;
		TextureViewDimension Dimension;
		uint32_t BaseMipLevel;
		uint32_t NumMipLevels;
		uint32_t BaseArrayLayer;
		uint32_t NumArrayLayers;
	};

	class ITextureView : public IObject
	{
	public:

		inline TextureAspectFlags GetAspect() const { return m_Aspect; }
		inline TextureViewDimension GetDimension() const { return m_Dimension; }
		inline TextureFormat GetFormat() const { return m_Format; }
		inline uint32_t GetBaseMipLevel()const { return m_BaseMipLevel; }
		inline uint32_t GetNumMipLevels() const { return m_NumMipLevels; }
		inline uint32_t GetBaseArrayLayer() { return m_BaseArrayLayer; }
		inline uint32_t GetNumArrayLayers() const { return m_NumArrayLayers; }

	protected:

		ITextureView(IRefCounter* pRefCounter, const TextureViewCreateInfo& CreateInfo)
			: IObject(pRefCounter)
		{
			m_Aspect = CreateInfo.Aspect;
			m_Dimension = CreateInfo.Dimension;
			m_Format = CreateInfo.Format;
			m_BaseMipLevel = CreateInfo.BaseMipLevel;
			m_NumMipLevels = CreateInfo.NumMipLevels;
			m_BaseArrayLayer = CreateInfo.BaseArrayLayer;
			m_NumArrayLayers = CreateInfo.NumArrayLayers;
		}

		~ITextureView() = default;

		TextureAspectFlags m_Aspect;
		TextureViewDimension m_Dimension;
		TextureFormat m_Format;
		uint32_t m_BaseMipLevel;
		uint32_t m_NumMipLevels;
		uint32_t m_BaseArrayLayer;
		uint32_t m_NumArrayLayers;

	};
}