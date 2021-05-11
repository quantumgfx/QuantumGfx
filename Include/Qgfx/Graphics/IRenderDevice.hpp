#pragma once

#include "GraphicsTypes.hpp"

#include "IBuffer.hpp"
#include "ICommandQueue.hpp"
#include "IComputePipeline.hpp"
#include "IGraphicsPipeline.hpp"
#include "IPipeline.hpp"
#include "ISampler.hpp"
#include "IShaderModule.hpp"
#include "ISwapChain.hpp"
#include "ITexture.hpp"

#include "../Common/RefCountedObject.hpp"

#include "../Platform/NativeWindow.hpp"

namespace Qgfx
{
	enum class RenderDeviceFeatureState
	{
		eDisabled = 0,
		eEnabled,
		eOptional
	};

	struct RenderDeviceFeatures
	{
		RenderDeviceFeatureState ComputeShaders =     RenderDeviceFeatureState::eDisabled;
		RenderDeviceFeatureState TesselationShaders = RenderDeviceFeatureState::eDisabled;
		RenderDeviceFeatureState GeometryShaders =    RenderDeviceFeatureState::eDisabled;
		RenderDeviceFeatureState IndirectRendering =  RenderDeviceFeatureState::eDisabled;
		RenderDeviceFeatureState PolygonModeLine =    RenderDeviceFeatureState::eDisabled;
		RenderDeviceFeatureState PolygonModePoint =   RenderDeviceFeatureState::eDisabled;
	};

	struct RenderDeviceCreateInfo
	{
		RenderDeviceFeatures Features;
	};

	class IRenderDevice
	{
	public:

		virtual void WaitIdle() = 0;

		virtual IBuffer* CreateBuffer(const BufferCreateInfo& CreateInfo) = 0;

		virtual void DestroyBuffer(IBuffer* pBuffer) = 0;

		virtual void CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& CreateInfo, IGraphicsPipeline** ppGraphicsPipeline) = 0;

		virtual void CreateSampler(const SamplerCreateInfo& CreateInfo, ISampler** ppSampler) = 0;

		virtual void CreateShaderModule(const ShaderModuleCreateInfo& CreateInfo, IShaderModule** ppShaderModule) = 0;

		virtual ITexture* CreateTexture(const TextureCreateInfo& CreateInfo) = 0;

		virtual void DestroyTexture(ITexture* pTexture) = 0;

		virtual ISwapChain* CreateSwapChain(const SwapChainCreateInfo& CreateInfo) = 0;

		virtual ICommandQueue* GetDefaultQueue() = 0;

		virtual void Destroy() = 0;

		inline const RenderDeviceFeatures& GetFeatures() const { return m_Features; }

	protected:

		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;

	protected:

		RenderDeviceFeatures m_Features;

	};
}