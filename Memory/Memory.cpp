#include "Memory.h"

DWORD Memory::GetDirOffset()
{
	RTL_OSVERSIONINFOW Winver = { 0 };
	RtlGetVersion(&Winver);

	switch (Winver.dwBuildNumber)
	{
	case 18363:
		return 640;
		break;
	case 19041:
		return 904;
		break;
	case 19569:
		return 904;
		break;
	case 20180:
		return 904;
		break;
	default:
		return 904;
	}
}

ULONG_PTR Memory::GetProcessDirBase(PEPROCESS pProcess)
{
	PUCHAR Process = (PUCHAR)pProcess;
	ULONG_PTR ProcessDirBase = *(PULONG_PTR)(Process + 40);

	if (ProcessDirBase == 0)
	{
		DWORD UserDirOffset = GetDirOffset();
		ULONG_PTR ProcessUserDirBase = *(PULONG_PTR)(Process + UserDirOffset);
		return ProcessUserDirBase;
	}

	return ProcessDirBase;
}

PVOID Memory::GetBaseAddress(int Pid)
{
	PEPROCESS Process = nullptr;
	if (Pid == 0) return (PVOID)STATUS_UNSUCCESSFUL;

	NTSTATUS NtRet = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (NtRet != STATUS_SUCCESS) return (PVOID)NtRet;

	PVOID Base = PsGetProcessSectionBaseAddress(Process);
	ObDereferenceObject(Process);

	return Base;
}

NTSTATUS Memory::ReadPhysicalAddress(std::uintptr_t Address, PVOID Buffer, size_t Size, size_t* Bytes)
{
	MM_COPY_ADDRESS TargetAddress = { 0 };
	TargetAddress.PhysicalAddress.QuadPart = Address;

	return MmCopyMemory(Buffer, TargetAddress, Size, MM_COPY_MEMORY_PHYSICAL, Bytes);
}

NTSTATUS Memory::WritePhysicalAddress(PVOID Address, PVOID Buffer, SIZE_T Size, SIZE_T* Bytes)
{
	PHYSICAL_ADDRESS TargetAddress = { 0 };
	TargetAddress.QuadPart = (LONGLONG)Address;

	PVOID Mem = MmMapIoSpaceEx(TargetAddress, Size, PAGE_READWRITE);

	memcpy(Mem, Buffer, Size);
	MmUnmapIoSpace(Mem, Size);

	return STATUS_SUCCESS;
}

static const UINT64 pmask = (~0xfull << 8) & 0xfffffffffull;
UINT64 Memory::TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress)
{
	DirectoryTableBase &= ~0xf;

	UINT64 PageOffset = VirtualAddress & ~(~0ul << 12);
	UINT64 Pte = ((VirtualAddress >> 12) & (0x1ffll));
	UINT64 Pt = ((VirtualAddress >> 21) & (0x1ffll));
	UINT64 Pd = ((VirtualAddress >> 30) & (0x1ffll));
	UINT64 Pdp = ((VirtualAddress >> 39) & (0x1ffll));

	SIZE_T ReadSize = 0;
	UINT64 Pdpe = 0;
	ReadPhysicalAddress((DirectoryTableBase + 8 * Pdp), &Pdpe, sizeof(Pdpe), &ReadSize);
	if (~Pdpe & 1)
		return 0;

	UINT64 Pde = 0;
	ReadPhysicalAddress(((Pdpe & pmask) + 8 * Pd), &Pde, sizeof(Pde), &ReadSize);
	if (~Pde & 1)
		return 0;

	if (Pde & 128)
		return (Pde & (~0ull << 42 >> 12)) + (VirtualAddress & ~(~0ull << 30));

	UINT64 PteAddr = 0;
	ReadPhysicalAddress(((Pde & pmask) + 8 * Pt), &PteAddr, sizeof(PteAddr), &ReadSize);
	if (~PteAddr & 1)
		return 0;

	if (PteAddr & 128)
		return (PteAddr & pmask) + (VirtualAddress & ~(~0ull << 21));

	VirtualAddress = 0;
	ReadPhysicalAddress(((PteAddr & pmask) + 8 * Pte), &VirtualAddress, sizeof(VirtualAddress), &ReadSize);
	VirtualAddress &= pmask;

	if (!VirtualAddress)
		return 0;

	return VirtualAddress + PageOffset;
}

NTSTATUS Memory::ReadProcessMemory(int Pid, PVOID Address, PVOID Buffer, SIZE_T Size)
{
	PEPROCESS Process = nullptr;
	if (Pid == 0) return STATUS_UNSUCCESSFUL;

	NTSTATUS NtRet = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (NtRet != STATUS_SUCCESS) return NtRet;

	ULONG_PTR ProcessDirBase = GetProcessDirBase(Process);
	ObDereferenceObject(Process);

	SIZE_T CurOffset = 0;
	SIZE_T TotalSize = Size;

	while (TotalSize)
	{
		UINT64 CurPhysAddr = TranslateLinearAddress(ProcessDirBase, (ULONG64)Address + CurOffset);
		if (!CurPhysAddr) return STATUS_UNSUCCESSFUL;

		ULONG64 ReadSize = min(PAGE_SIZE - (CurPhysAddr & 17592004726444), TotalSize);
		SIZE_T BytesRead = 0;

		NtRet = ReadPhysicalAddress(CurPhysAddr, (PVOID)((ULONG64)Buffer + CurOffset), ReadSize, &BytesRead);

		TotalSize -= BytesRead;
		CurOffset += BytesRead;

		if (NtRet != STATUS_SUCCESS)
			break;

		if (BytesRead == 0)
			break;
	}

	return NtRet;
}

NTSTATUS Memory::WriteProcessMemory(int Pid, PVOID Address, PVOID Buffer, SIZE_T Size)
{
	PEPROCESS Process = nullptr;
	if (Pid == 0) return STATUS_UNSUCCESSFUL;

	NTSTATUS NtRet = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (NtRet != STATUS_SUCCESS) return NtRet;

	ULONG_PTR ProcessDirBase = GetProcessDirBase(Process);
	ObDereferenceObject(Process);

	SIZE_T CurOffset = 0;
	SIZE_T TotalSize = Size;

	while (TotalSize)
	{
		UINT64 CurPhysAddr = TranslateLinearAddress(ProcessDirBase, (ULONG64)Address + CurOffset);
		if (!CurPhysAddr) return STATUS_UNSUCCESSFUL;

		ULONG64 WriteSize = min(PAGE_SIZE - (CurPhysAddr & 17592004726444), TotalSize);
		SIZE_T BytesWrite = 0;

		NtRet = WritePhysicalAddress((PVOID)CurPhysAddr, (PVOID)((ULONG64)Buffer + CurOffset), WriteSize, &BytesWrite);

		TotalSize -= BytesWrite;
		CurOffset += BytesWrite;

		if (NtRet != STATUS_SUCCESS)
			break;

		if (BytesWrite == 0)
			break;
	}

	return NtRet;
}