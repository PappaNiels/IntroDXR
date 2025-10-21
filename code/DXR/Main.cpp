#include "pch.hpp"

#include <Utils/CLI.hpp>
#include <Renderer/Renderer.hpp>

void CreateConsole()
{
	if (GetCLI().Console == 0)
	{
		return;
	}

	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(consoleHandle, &dwMode);

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

	SetConsoleMode(consoleHandle, dwMode);
	SetConsoleTitle(TEXT("Intro DirectX Raytracing | Debug Console"));
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE, LPSTR cmdLine, INT cmdShow)
{
	ParseCLI();

	CreateConsole();

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	Renderer* renderer = new Renderer();
	renderer->Initialize();

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (GetCLI().Console)
	{
		FreeConsole();
	}

	renderer->Shutdown();
	delete renderer;

	return 0;
}