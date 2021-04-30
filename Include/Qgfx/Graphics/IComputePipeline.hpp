#pragma once

#include "IPipeline.hpp"

namespace Qgfx
{
	struct ComputeState : public ProgrammableStage
	{
	};

	struct ComputePipelineCreateInfo
	{
		ComputeState Compute = {};
	};
}