#pragma once
#include "Defines.h"

namespace Utils
{
	UINT64 FindPattern(UINT64 Address, UINT64 Len, BYTE* bMask, char* sMask);
	uintptr_t Dereference(uintptr_t Address, unsigned int Offset);
	uintptr_t FindPattern2(void* Start, size_t Length, const char* Pattern, const char* Mask);
	uintptr_t GetModulesBase(const char* Name);
}