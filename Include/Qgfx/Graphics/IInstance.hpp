#pragma once

#include "APIInfo.hpp"
#include "GraphicsTypes.hpp"

namespace Qgfx
{
	class IInstance
	{
	public:

		virtual ~IInstance() = default;

		virtual const APIInfo& GetAPIInfo() const = 0;

		virtual GraphicsInstanceType GetType() const = 0;
	};
}