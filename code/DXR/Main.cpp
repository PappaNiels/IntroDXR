#include "pch.hpp"

#include <Utils/CLI.hpp>
#include <Renderer/Renderer.hpp>

#include <chrono>

extern Renderer* CreateSample();

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

	Renderer* renderer = CreateSample();
	renderer->Initialize();

	auto oldTime = std::chrono::high_resolution_clock::now();
	float elapsedTime = 0.0f;
	uint32_t frameCount = 0;

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		auto newTime = std::chrono::high_resolution_clock::now();
		auto delta = newTime - oldTime;
		auto dT = static_cast<float>(delta.count()) * 1.0e-9f;
		oldTime = newTime;

		elapsedTime += dT;

		if (elapsedTime > 1.0f)
		{
			char buffer[128];

			sprintf_s(buffer, "FPS: %f\n", frameCount / elapsedTime);
			OutputDebugStringA(buffer);

			elapsedTime = 0.0f;
			frameCount = 0;
		}

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		renderer->Update(dT);
		renderer->Render();

		frameCount++;
	}

	if (GetCLI().Console)
	{
		FreeConsole();
	}

	renderer->Shutdown();
	delete renderer;

	return 0;
}