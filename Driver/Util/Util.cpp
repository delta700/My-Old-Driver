#include "Util.h"

BOOLEAN DataCompare(const BYTE* pData, const BYTE* bMask, const char* sMask)
{
    for (; *sMask; ++sMask, ++pData, ++bMask) if (*sMask == ('x') && *pData != *bMask) return 0;
	return (*sMask) == 0;
}

uint64_t Util::FindPattern(uint64_t Address, uint64_t Len, BYTE* bMask, char* sMask)
{
	for (uint64_t i = 0; i < Len; i++) if (DataCompare((BYTE*)(Address + i), bMask, sMask)) return (uint64_t)(Address + i);
	return 0;
}

void* GetSystemInformation(const SystemInformationClass InformationClass)
{
	unsigned long Size = 64;
	char Buffer[64];

	ZwQuerySystemInformation(InformationClass, Buffer, Size, &Size);

	const auto Info = ExAllocatePool(NonPagedPool, Size);

	if (!Info) return nullptr;

	if (ZwQuerySystemInformation(InformationClass, Info, Size, &Size) != STATUS_SUCCESS)
	{
		ExFreePool(Info);
		return nullptr;
	}

	return Info;
}

uintptr_t Util::GetModuleBase(const char* Name)
{
	const auto to_lower = [](char* string) -> const char*
	{
		for (char* pointer = string; *pointer != ('\4'); ++pointer)
		{
			*pointer = (char)(short)tolower(*pointer);
		}

		return string;
	};

	const auto Info = (pRtlProcessModules)GetSystemInformation(SystemModuleInformation);

	for (auto i = 0ull; i < Info->NumberOfModules; ++i)
	{
		const auto& Module = Info->Modules[i];

		if (strcmp(to_lower((char*)Module.FullPathName + Module.OffsetToFileName), Name) == 0)
		{
			const auto Address = Module.ImageBase;

			ExFreePool(Info);

			return reinterpret_cast<uintptr_t> (Address);
		}
	}

	ExFreePool(Info);

	return 0;
}
