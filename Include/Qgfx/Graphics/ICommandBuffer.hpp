#pragma once

#include "IObject.hpp"

namespace Qgfx
{
	enum class CommandBufferState
	{
		Recording = 0,
		Ready,
		Executing,
	};

	class ICommandBuffer : public IObject
	{
	public:

		ICommandBuffer(IRefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		virtual void Finish() = 0;

		virtual CommandBufferState GetCurrentState() = 0;
	};
}