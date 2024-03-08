#pragma once
#include "Define.h"

namespace Util
{
	uint64_t FindPattern(uint64_t Address, uint64_t Len, BYTE* bMask, char* sMask);
	uintptr_t GetModuleBase(const char* Name);
}
