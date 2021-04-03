#include "Qgfx/Common/SpinLock.hpp"

#include <thread>

namespace Qgfx
{
	void SpinLock::YieldThread() noexcept
	{
		std::this_thread::yield();
	}
}