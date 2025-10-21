#include <pch.hpp>

#include "Error.hpp"

#include <stdarg.h>

void FatalError(const std::string_view fmt, ...)
{
	char message[1028] = {};
	va_list args;

	va_start(args, fmt);
	vsprintf_s(message, fmt.data(), args);
	va_end(args);

	MessageBoxA(nullptr, message, "Intro to DirectX Raytracing", MB_OK | MB_ICONERROR);

	if (IsDebuggerPresent())
	{
		DebugBreak();
	}

	TerminateProcess(GetCurrentProcess(), 1);
}
