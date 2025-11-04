#pragma once

#include <string_view>

void _Assert(bool expression, const std::string_view fmt, const std::string_view file, int line, ...);

#if defined _DEBUG
#define ASSERT(expression, fmt, ...) _Assert(expression, fmt, __FILE__, __LINE__ ##__VA_ARGS__)
#else
#define ASSERT(expression, fmt, ...) (void)(expression)
#endif
