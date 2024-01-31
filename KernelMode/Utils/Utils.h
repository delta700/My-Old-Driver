#pragma once
#include "Defines.h"

namespace Utils
{
	UINT64 FindPattern(UINT64 Address, UINT64 Len, BYTE* bMask, char* sMask);
	uintptr_t GetModulesBase(const char* Name);
}