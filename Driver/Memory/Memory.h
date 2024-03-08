#pragma once
#include "../../Util/Define.h"

namespace Memory
{
	DWORD GetUserDirectoryTableBaseOffset();
	ULONG_PTR GetProcessDirBase(PEPROCESS pProcess);

	PVOID GetBaseAddress(int Pid);

	NTSTATUS ReadPhysicalAddress(PVOID TargetAddress, PVOID Read_Buffer, SIZE_T Read_Size, SIZE_T* Read_Bytes);
	NTSTATUS WritePhysicalAddress(PVOID TargetAddress, PVOID Write_Buffer, SIZE_T Write_Size, SIZE_T* Write_Bytes);

	uint64_t TranslateLinearAddress(uint64_t DirectoryTableBase, uint64_t VirtualAddress);

	NTSTATUS ReadProcessMemory(int Pid, PVOID Read_Address, PVOID Read_Buffer, SIZE_T Read_Size);
	NTSTATUS WriteProcessMemory(int Pid, PVOID Write_Address, PVOID Write_Buffer, SIZE_T Write_Size);
}
