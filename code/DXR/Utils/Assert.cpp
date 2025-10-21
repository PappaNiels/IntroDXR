#include <pch.hpp>

#include "Assert.hpp"

#include <stdarg.h>

void _Assert(bool expression, const std::string_view fmt, const std::string_view file, int line, ...)
{
	if (expression)
	{
		return;
	}

	char message[1028] = {};
	va_list args;

	va_start(args, line);
	vsprintf_s(message, fmt.data(), args);
	va_end(args);

	sprintf_s(message, "%s\nFile: %s\nLine: %i", message, file.data(), line);

	int ret = MessageBoxA(nullptr, message, "Intro to DirectX Raytracing | Assert", MB_ABORTRETRYIGNORE | MB_ICONERROR);

	switch (ret)
	{
	case IDRETRY:
	{
		DebugBreak();
		TerminateProcess(GetCurrentProcess(), 1);
		return;
	}
	case IDIGNORE:
		return;
	case IDABORT:
	{
		TerminateProcess(GetCurrentProcess(), 1);
		return;
	}
	}
}
