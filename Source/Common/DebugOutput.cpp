#include "Qgfx/Common/DebugOutput.hpp"

#include <mutex>
#include <iostream>
#include <cstdlib>

namespace Qgfx
{
	std::mutex g_DebugMessageMutex = {};
	std::mutex g_DebugAssertFailedMutex = {};
	DebugMessageCallbackType g_DebugMessageCallback = nullptr;
	DebugAssertFailedCallbackType g_DebugAssertFailedCallback = nullptr;

	void SetDebugMessageCallback(DebugMessageCallbackType DbgMessageCallback)
	{
		std::lock_guard Lock(g_DebugMessageMutex);
		g_DebugMessageCallback = DbgMessageCallback;
	}

	void SetDebugAssertFailedCallback(DebugAssertFailedCallbackType DbgAssertFailedCallback)
	{
		std::lock_guard Lock(g_DebugAssertFailedMutex);
		g_DebugAssertFailedCallback = DbgAssertFailedCallback;
	}

	void WriteDebugMessage(DebugMessageSeverity Severity, const char* Message, const char* Function, const char* File, int Line)
	{
		std::lock_guard Lock(g_DebugMessageMutex);
		if (g_DebugMessageCallback)
		{
			g_DebugMessageCallback(Severity, Message, Function, File, Line);
		}
	}

	void DebugAssertFailed(const char* Message, const char* Function, const char* File, int Line)
	{
		std::lock_guard Lock(g_DebugAssertFailedMutex);
		if (g_DebugAssertFailedCallback)
		{
			g_DebugAssertFailedCallback(Message, Function, File, Line);
		}
		else
		{
			std::cerr << "Qgfx: Runtime assertion failed in " << Function << "() (" << File << ", " << Line << "): " << Message << '\n';
		}

		// This will likely have already been called by g_DebugAssertFailedCallback
		std::exit(3);
	}
}
