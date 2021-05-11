#pragma once

#include <cstdint>
#include <limits>

#include "GraphicsTypes.hpp"
#include "IObject.hpp"

#include "../Common/HashUtils.hpp"

namespace Qgfx
{
	enum class AddressMode
	{
		eClamp = 0,
		eRepeat,
		eMirroredRepeat,
	};

	struct SamplerCreateInfo
	{
		AddressMode AddressModeU = AddressMode::eClamp;
		AddressMode AddressModeV = AddressMode::eClamp;
		AddressMode AddressModeW = AddressMode::eClamp;
		FilterMode MagFilter =     FilterMode::eNearest;
		FilterMode MinFilter =     FilterMode::eNearest;
		FilterMode MipMapFilter =  FilterMode::eNearest;
		float LodMinClamp = 0;
		float LodMaxClamp = std::numeric_limits<float>::max();
		uint32_t MaxAnisotropy = 1;

		bool bCompareEnable = false;
		CompareFunc Compare = CompareFunc::eAlways;

		bool operator==(const SamplerCreateInfo& Rhs)const
		{
			// It is ignored in comparison operation.
			return  MinFilter == Rhs.MinFilter &&
				MagFilter == Rhs.MagFilter &&
				MipMapFilter == Rhs.MipMapFilter &&
				AddressModeU == Rhs.AddressModeU &&
				AddressModeV == Rhs.AddressModeV &&
				AddressModeW == Rhs.AddressModeW &&
				MaxAnisotropy == Rhs.MaxAnisotropy &&
				bCompareEnable == Rhs.bCompareEnable &&
				Compare == Rhs.Compare &&
				LodMinClamp == Rhs.LodMinClamp &&
				LodMaxClamp == Rhs.LodMaxClamp;
		}
	};

	class ISampler : public IObject
	{
	protected:

		ISampler(IRefCounter* pRefCounter)
			: IObject(pRefCounter)
		{
		}

		~ISampler() = default;
	};
}

namespace std
{
	template <>
	struct hash<Qgfx::SamplerCreateInfo>
	{
		size_t operator()(const Qgfx::SamplerCreateInfo& SamplerCI) const
		{
			// Sampler name is ignored in comparison operator
			// and should not be hashed
			return Qgfx::ComputeHash( // SamDesc.Name,
				static_cast<int>(SamplerCI.MinFilter),
				static_cast<int>(SamplerCI.MagFilter),
				static_cast<int>(SamplerCI.MipMapFilter),
				static_cast<int>(SamplerCI.AddressModeU),
				static_cast<int>(SamplerCI.AddressModeV),
				static_cast<int>(SamplerCI.AddressModeW),
				SamplerCI.MaxAnisotropy,
				static_cast<int>(SamplerCI.Compare),
				SamplerCI.LodMinClamp,
				SamplerCI.LodMaxClamp);
		}
	};
}