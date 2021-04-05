#pragma once

#include "IObject.hpp"

#include "GraphicsTypes.hpp"
#include "IRenderContext.hpp"
#include "ISwapChain.hpp"

#include "../Platform/NativeWindow.hpp"

namespace Qgfx
{
	class IRenderDevice : public IObject
	{

	public:

		IRenderDevice(RefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual ~IRenderDevice() = default;

		/**
		 * @brief Gets the number of avalable contexts of a certain type. This will be at least 1 for IRenderContext::Type::Universal.
		 * @param RenderContextType - Which type of context to query.
		 * @return The number of available contexts this devices supports of type 'RenderContextType'
		*/
		virtual uint32_t GetNumRenderContexts(IRenderContext::Type RenderContextType) = 0;

		/**
		 * @brief Creates a new render context. This call will fail if you pass in an RenderContextType + Index that has already been created.
		 * @param RenderContextType - The type of context to create.
		 * @param Index - The index of the render context to create. This must be less than the value returned by GetNumRenderContexts('RenderContextType').
		 * @param ppContext - Pointer which will be filled with a newly created render context (or nullptr on fail).
		*/
		virtual void CreateRenderContext(IRenderContext::Type RenderContextType, uint32_t Index, IRenderContext** ppContext) = 0;

		virtual void CreateSwapChain(const SwapChainDesc& Desc, const NativeWindow& Window, IRenderContext* pContext, ISwapChain** ppSwapChain) = 0;

		virtual void WaitIdle() = 0;

		virtual const DeviceFeatures& GetFeatures() const = 0;

	};
}