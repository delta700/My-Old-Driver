#pragma once
#include "../../Utils/Defines.h"

namespace Memory
{
	DWORD GetDirOffset();
	ULONG_PTR GetProcessDirBase(PEPROCESS pProcess);
	PVOID GetBaseAddress(int Pid);
	NTSTATUS ReadPhysicalAddress(std::uintptr_t Address, PVOID Buffer, size_t Size, size_t* Bytes);
	NTSTATUS WritePhysicalAddress(PVOID Address, PVOID Buffer, SIZE_T Size, SIZE_T* Bytes);
	UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress);
	NTSTATUS ReadProcessMemory(int Pid, PVOID Address, PVOID Buffer, SIZE_T Size);
	NTSTATUS WriteProcessMemory(int Pid, PVOID Address, PVOID Buffer, SIZE_T Size);
}