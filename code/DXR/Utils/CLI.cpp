#include <pch.hpp>
#include "CLI.hpp"
#include <algorithm>

CLI g_CLI = {};

const CLI& GetCLI()
{
	return g_CLI;
}

void ParseCLI()
{
	std::string cli = GetCommandLineA();
	
	if (cli.find("-console") != cli.npos)
	{
		g_CLI.Console = 1;
	}

	if (cli.find("-validation") != cli.npos)
	{
		g_CLI.Validation = 1;
	}

	if (cli.find("-warp") != cli.npos)
	{
		g_CLI.Warp = 1;
	}
}
