#pragma once

namespace Qgfx
{
	enum class DebugMessageSeverity
	{
		Info = 0,
		Warning,
		Error,
		FatalError
	};

	/// Type of the debug messag callback function

	/// \param [in] Severity - Message severity
	/// \param [in] Message - Debug message
	/// \param [in] Function - Name of the function or nullptr
	/// \param [in] Function - File name or nullptr
	/// \param [in] Line - Line number
	typedef void (*DebugMessageCallbackType)(DebugMessageSeverity Severity, const char* Message, const char* Function, const char* File, int Line);

	typedef void (*DebugAssertFailedCallbackType)(const char* Message, const char* Function, const char* File, int Line);

	void SetDebugMessageCallback(DebugMessageCallbackType DbgMessageCallback);

	void SetDebugAssertFailedCallback(DebugAssertFailedCallbackType DbgAssertFailedCallback);

	void WriteDebugMessage(DebugMessageSeverity Severity, const char* Message, const char* Function, const char* File, int Line);

	void DebugAssertFailed(const char* Message, const char* Function, const char* File, int Line);
}