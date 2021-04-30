#include "Qgfx/Graphics/ITexture.hpp"

namespace Qgfx
{
    ITexture::ITexture(IRefCounter* pRefCounter, const TextureCreateInfo& CreateInfo)
        : IObject(pRefCounter)
    {
        m_Dimension = CreateInfo.Dimension;
        m_Usage = CreateInfo.Usage;
        m_Format = CreateInfo.Format;
        m_SampleCount = CreateInfo.SampleCount;
        m_Extent.Width = CreateInfo.Width;
        m_Extent.Height = CreateInfo.Height;
        m_Extent.Depth = m_Dimension == TextureDimension::e3D ? CreateInfo.Depth : 1u;
        m_NumMipLevels = CreateInfo.MipLevels;
        m_NumArrayLayers = m_Dimension != TextureDimension::e3D ? CreateInfo.ArraySize : 1u;
    }
}