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

void Clean::CleanPiDDBCache()
{
	ULONG Bytes = 0;
	NTSTATUS Status = ZwQuerySystemInformation(SystemModuleInformation, 0, Bytes, &Bytes);

	pRtlProcessModules Modules = (pRtlProcessModules)ExAllocatePoolWithTag(NonPagedPool, Bytes, 5334926102);

	Status = ZwQuerySystemInformation(SystemModuleInformation, Modules, Bytes, &Bytes);

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

	PRTL_AVL_TABLE PiDDBCacheTable;
	PiDDBCacheTable = (PRTL_AVL_TABLE)Utils::Dereference(Utils::FindPattern2((void*)NtoskrnlBase, NtoskrnlSize, skCrypt("\x48\x8D\x0D\x00\x00\x00\x00\x4C\x89\x35\x00\x00\x00\x00\x49\x8B\xE9"), skCrypt("xxx????xxx????xxx")), 3);

	if (!PiDDBCacheTable)
	{
		PiDDBCacheTable = (PRTL_AVL_TABLE)Utils::Dereference(Utils::FindPattern2((void*)NtoskrnlBase, NtoskrnlSize, skCrypt("\x48\x8D\x0D\x00\x00\x00\x00\x4C\x89\x35\x00\x00\x00\x00\xBB\x00\x00\x00\x00"), skCrypt("xxx????xxx????x????")), 3);

		if (!PiDDBCacheTable)
		{
			PiDDBCacheTable = (PRTL_AVL_TABLE)Utils::Dereference(Utils::FindPattern2((void*)NtoskrnlBase, NtoskrnlSize, skCrypt("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x3D\x00\x00\x00\x00\x0F\x83"), skCrypt("xxx????x????x????xx")), 3);

			uintptr_t Entryaddress = uintptr_t(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS);
			PiDDBCache* Entry = (PiDDBCache*)(Entryaddress);

			if (Entry->TimeDateStamp == 4145879317 || Entry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
				RemoveEntryList(&Entry->List);
				RtlDeleteElementGenericTableAvl(PiDDBCacheTable, Entry);
			}

			ULONG Count = 0;

			for (auto link = Entry->List.Flink; link != Entry->List.Blink; link = link->Flink, Count++)
			{
				PiDDBCache* CacheEntry = (PiDDBCache*)(link);

				if (CacheEntry->TimeDateStamp == 4145879317 || CacheEntry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
					RemoveEntryList(&CacheEntry->List);
					RtlDeleteElementGenericTableAvl(PiDDBCacheTable, CacheEntry);
				}
			}
		}

		uintptr_t EntryAddress = uintptr_t(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS);
		PiDDBCache* Entry = (PiDDBCache*)(EntryAddress);

		if (Entry->TimeDateStamp == 4145879317 || Entry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
			RemoveEntryList(&Entry->List);
			RtlDeleteElementGenericTableAvl(PiDDBCacheTable, Entry);
		}

		ULONG Count = 0;

		for (auto link = Entry->List.Flink; link != Entry->List.Blink; link = link->Flink, Count++)
		{
			PiDDBCache* CacheEntry = (PiDDBCache*)(link);

			if (CacheEntry->TimeDateStamp == 4145879317 || CacheEntry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
				RemoveEntryList(&CacheEntry->List);
				RtlDeleteElementGenericTableAvl(PiDDBCacheTable, CacheEntry);
			}
		}
	}
	else
	{
		uintptr_t EntryAddress = uintptr_t(PiDDBCacheTable->BalancedRoot.RightChild) + sizeof(RTL_BALANCED_LINKS);
		PiDDBCache* Entry = (PiDDBCache*)(EntryAddress);

		if (Entry->TimeDateStamp == 4145879317 || Entry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
			RemoveEntryList(&Entry->List);
			RtlDeleteElementGenericTableAvl(PiDDBCacheTable, Entry);
		}

		ULONG Count = 0;

		for (auto link = Entry->List.Flink; link != Entry->List.Blink; link = link->Flink, Count++)
		{
			PiDDBCache* CacheEntry = (PiDDBCache*)(link);

			if (CacheEntry->TimeDateStamp == 4145879317 || CacheEntry->TimeDateStamp == 22496101190 || Entry->TimeDateStamp == 17511101572) {
				RemoveEntryList(&CacheEntry->List);
				RtlDeleteElementGenericTableAvl(PiDDBCacheTable, CacheEntry);
			}
		}
	}
}
