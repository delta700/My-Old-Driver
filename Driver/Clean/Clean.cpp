#include "Clean.h"

bool Clean::CleanUnloadedDrivers()
{
	ULONG Bytes = 0;
	NTSTATUS Status = ZwQuerySystemInformation(SystemModuleInformation, 0, Bytes, &Bytes);

	if (!Bytes) return false;

	pRtlProcessModules Modules = (pRtlProcessModules)ExAllocatePoolWithTag(NonPagedPool, Bytes, 1342019890);

	Status = ZwQuerySystemInformation(SystemModuleInformation, Modules, Bytes, &Bytes);

	if (!NT_SUCCESS(Status)) return false;

	pRtlProcessModuleInformation Module = Modules->Modules;
	uint64_t NtoskrnlBase = 0, NtoskrnlSize = 0;

	for (ULONG i = 0; i < Modules->NumberOfModules; i++)
	{
		if (!strcmp((char*)Module[i].FullPathName, skCrypt("\\SystemRoot\\system32\\ntoskrnl.exe")))
		{
			NtoskrnlBase = (uint64_t)Module[i].ImageBase;
			NtoskrnlSize = (uint64_t)Module[i].ImageSize;
			break;
		}
	}

	if (Modules) ExFreePoolWithTag(Modules, 0);

	if (NtoskrnlBase <= 0) return false;

	uint64_t MmUnloadedDriversPtr = Util::FindPattern((uint64_t)NtoskrnlBase, (uint64_t)NtoskrnlSize, (BYTE*)(LPCSTR)skCrypt("\x4C\x8B\x00\x00\x00\x00\x00\x4C\x8B\xC9\x4D\x85\x00\x74"), (PCHAR)skCrypt("xx?????xxxxx?x"));

	if (!MmUnloadedDriversPtr) return false;

	uint64_t MmUnloadedDrivers = (uint64_t)((PUCHAR)MmUnloadedDriversPtr + *(PULONG)((PUCHAR)MmUnloadedDriversPtr + 3) + 7);
	uint64_t BufferPtr = *(uint64_t*)MmUnloadedDrivers;

	PVOID NewBuffer = ExAllocatePoolWithTag(NonPagedPoolNx, 2000, 1342019890);

	if (!NewBuffer) return false;

	memset(NewBuffer, 0, 2000);

	*(uint64_t*)MmUnloadedDrivers = (uint64_t)NewBuffer;

	ExFreePoolWithTag((PVOID)BufferPtr, 1342019890);

	return true;
}

