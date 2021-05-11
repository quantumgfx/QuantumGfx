#include "Qgfx/Graphics/ITexture.hpp"
#include "Qgfx/Graphics/IRenderDevice.hpp"

namespace Qgfx
{

    ITexture::ITexture(IRenderDevice* pRenderDevice, const TextureCreateInfo& CreateInfo)
        : m_pRenderDevice(pRenderDevice)
    {
        m_pRenderDevice->AddRef();

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

    void TextureDeleter::operator()(ITexture* pTexture)
    {
        IRenderDevice* pRenderDevice = pTexture->m_pRenderDevice;
        pRenderDevice->DeleteTexture(pTexture);
        pRenderDevice->Release();
    }
}