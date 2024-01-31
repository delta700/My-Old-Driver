#include "clean.h"

BOOLEAN Clean::CleanUnloadedDrivers()
{
	ULONG Bytes = 0;
	NTSTATUS Status = ZwQuerySystemInformation(SystemModuleInformation, 0, Bytes, &Bytes);

	if (!Bytes)
	{
		return false;
	}

	pRtlProcessModules Modules = (pRtlProcessModules)ExAllocatePoolWithTag(NonPagedPool, Bytes, 1342019890);

	Status = ZwQuerySystemInformation(SystemModuleInformation, Modules, Bytes, &Bytes);

	if (!NT_SUCCESS(Status))
	{
		return false;
	}

	pRtlProcessModuleInformation Module = Modules->Modules;
	UINT64 NtoskrnlBase = 0, NtoskrnlSize = 0;

	for (ULONG i = 0; i < Modules->NumberOfModules; i++)
	{
		if (!strcmp((char*)Module[i].FullPathName, skCrypt("\\SystemRoot\\system32\\ntoskrnl.exe")))
		{
			NtoskrnlBase = (UINT64)Module[i].ImageBase;
			NtoskrnlSize = (UINT64)Module[i].ImageSize;
			break;
		}
	}

	if (Modules)
		ExFreePoolWithTag(Modules, 0);

	if (NtoskrnlBase <= 0)
	{
		return false;
	}

	UINT64 MmUnloadedDriversPtr = Utils::FindPattern((UINT64)NtoskrnlBase, (UINT64)NtoskrnlSize, (BYTE*)(LPCSTR)skCrypt("\x4C\x8B\x00\x00\x00\x00\x00\x4C\x8B\xC9\x4D\x85\x00\x74"), (PCHAR)skCrypt("xx?????xxxxx?x"));

	if (!MmUnloadedDriversPtr)
	{
		return false;
	}

	UINT64 MmUnloadedDrivers = (UINT64)((PUCHAR)MmUnloadedDriversPtr + *(PULONG)((PUCHAR)MmUnloadedDriversPtr + 3) + 7);
	UINT64 BufferPtr = *(UINT64*)MmUnloadedDrivers;

	PVOID NewBuffer = ExAllocatePoolWithTag(NonPagedPoolNx, 2000, 1342019890);

	if (!NewBuffer)
		return false;

	memset(NewBuffer, 0, 2000);

	*(UINT64*)MmUnloadedDrivers = (UINT64)NewBuffer;

	ExFreePoolWithTag((PVOID)BufferPtr, 1342019890);

	return true;
}
