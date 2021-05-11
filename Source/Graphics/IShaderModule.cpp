#include "Qgfx/Graphics/IShaderModule.hpp"
#include "Qgfx/Graphics/IRenderDevice.hpp"

namespace Qgfx
{

	IShaderModule::IShaderModule(IRenderDevice* pRenderDevice)
		: m_pRenderDevice(pRenderDevice)
	{
		pRenderDevice->AddRef();
	}

	IShaderModule::~IShaderModule()
	{
	}

	void ShaderModuleDeleter::operator()(IShaderModule* pShaderModule)
	{
		IRenderDevice* pRenderDevice = pShaderModule->m_pRenderDevice;
		pShaderModule->~IShaderModule();
		pRenderDevice->FreeShaderModuleObject(pShaderModule);
		pRenderDevice->Release();
	}
}