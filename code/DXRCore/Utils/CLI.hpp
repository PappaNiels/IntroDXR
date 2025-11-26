#pragma once

#include <cstdint>

struct CLI
{
	uint8_t Validation : 1;
	uint8_t Warp : 1;
	uint8_t Console : 1;
};

const CLI& GetCLI();
void ParseCLI();