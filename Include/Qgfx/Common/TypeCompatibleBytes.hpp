#pragma once

#include <cstdint>

namespace Qgfx
{
	template<typename ElementType>
	class TypeCompatibleBytes
	{
	public:

		ElementType* GetTypedPtr() { return reinterpret_cast<ElementType*>(m_Storage); }
		const ElementType* GetTypedPtr() const { return reinterpret_cast<ElementType*>(m_Storage); }

	private:

		alignas(ElementType) uint8_t m_Storage[sizeof(ElementType)];

	};
}