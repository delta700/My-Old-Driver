#include "Memory.h"

#define WINDOWS_1803 17134
#define WINDOWS_1809 17763
#define WINDOWS_1903 18362
#define WINDOWS_1909 18363
#define WINDOWS_2004 19041
#define WINDOWS_20H2 19569
#define WINDOWS_21H1 20180
#define WINDOWS_22H2 19045

DWORD Memory::GetUserDirectoryTableBaseOffset()
{
	RTL_OSVERSIONINFOW OSVer = { 0 };
	RtlGetVersion(&OSVer);

	switch (OSVer.dwBuildNumber)
	{
	case WINDOWS_1803:
		return 0x0278;
		break;
	case WINDOWS_1809:
		return 0x0278;
		break;
	case WINDOWS_1903:
		return 0x0280;
		break;
	case WINDOWS_1909:
		return 0x0280;
		break;
	case WINDOWS_2004:
		return 0x0388;
		break;
	case WINDOWS_20H2:
		return 0x0388;
		break;
	case WINDOWS_21H1:
		return 0x0388;
		break;
	case WINDOWS_22H2:
		return 0x0388;
		break;
	default:
		return 0x0388;
	}
}

uint64_t Memory::GetProcessDirectoryTableBase(PEPROCESS pProcess)
{
	PUCHAR Process = (PUCHAR)pProcess;
	ULONG_PTR ProcessDirBase = *(PULONG_PTR)(Process + 0x28);

	if (ProcessDirBase == 0)
	{
		DWORD UserDirOffset = GetUserDirectoryTableBaseOffset();
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

NTSTATUS Memory::ReadPhysicalAddress(PVOID TargetAddress, PVOID Read_Buffer, SIZE_T Read_Size, SIZE_T* Read_Bytes)
{
	MM_COPY_ADDRESS Target_Address = { 0 };
	Target_Address.PhysicalAddress.QuadPart = (LONGLONG)TargetAddress;

	return MmCopyMemory(Read_Buffer, Target_Address, Read_Size, MM_COPY_MEMORY_PHYSICAL, Read_Bytes);
}

NTSTATUS Memory::WritePhysicalAddress(PVOID TargetAddress, PVOID Write_Buffer, SIZE_T Write_Size, SIZE_T* Write_Bytes)
{
	PHYSICAL_ADDRESS Target_Address = { 0 };
	Target_Address.QuadPart = (LONGLONG)TargetAddress;

	PVOID Memory = MmMapIoSpaceEx(Target_Address, Write_Size, PAGE_READWRITE);

	memcpy(Memory, Write_Buffer, Write_Size);
	MmUnmapIoSpace(Memory, Write_Size);

	return STATUS_SUCCESS;
}

uint64_t Memory::TranslateLinearAddress(uint64_t DirectoryTableBase, uint64_t VirtualAddress)
{
	DirectoryTableBase &= ~0xf;

	uint64_t pMask = (~0xfull << 8) & 0xfffffffffull;
	uint64_t PageOffset = VirtualAddress & ~(~0ul << 12);
	uint64_t Pte = ((VirtualAddress >> 12) & (0x1ffll));
	uint64_t Pt = ((VirtualAddress >> 21) & (0x1ffll));
	uint64_t Pd = ((VirtualAddress >> 30) & (0x1ffll));
	uint64_t Pdp = ((VirtualAddress >> 39) & (0x1ffll));

	SIZE_T ReadSize = 0;
	uint64_t Pdpe = 0;
	ReadPhysicalAddress(PVOID(DirectoryTableBase + 8 * Pdp), &Pdpe, sizeof(Pdpe), &ReadSize);
	if (~Pdpe & 1) return 0;

	uint64_t Pde = 0;
	ReadPhysicalAddress(PVOID((Pdpe & pMask) + 8 * Pd), &Pde, sizeof(Pde), &ReadSize);
	if (~Pde & 1) return 0;

	if (Pde & 0x80) return (Pde & (~0ull << 42 >> 12)) + (VirtualAddress & ~(~0ull << 30));

	uint64_t PteAddr = 0;
	ReadPhysicalAddress(PVOID((Pde & pMask) + 8 * Pt), &PteAddr, sizeof(PteAddr), &ReadSize);
	if (~PteAddr & 1) return 0;

	if (PteAddr & 0x80) return (PteAddr & pMask) + (VirtualAddress & ~(~0ull << 21));

	VirtualAddress = 0;
	ReadPhysicalAddress(PVOID((PteAddr & pMask) + 8 * Pte), &VirtualAddress, sizeof(VirtualAddress), &ReadSize);
	VirtualAddress &= pMask;

	if (!VirtualAddress) return 0;

	return VirtualAddress + PageOffset;
}

NTSTATUS Memory::ReadProcessMemory(int Pid, PVOID Read_Address, PVOID Read_Buffer, SIZE_T Read_Size)
{
	PEPROCESS Process = nullptr;
	if (Pid == 0) return STATUS_UNSUCCESSFUL;

	NTSTATUS NtRet = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (NtRet != STATUS_SUCCESS) return NtRet;

	ULONG_PTR ProcessDirBase = GetProcessDirectoryTableBase(Process);
	ObDereferenceObject(Process);

	SIZE_T CurOffset = 0;
	SIZE_T TotalSize = Read_Size;

	while (TotalSize)
	{
		uint64_t CurPhysicalAddress = TranslateLinearAddress(ProcessDirBase, (ULONG64)Read_Address + CurOffset);
		if (!CurPhysicalAddress) return STATUS_UNSUCCESSFUL;

		ULONG64 ReadSize = min(PAGE_SIZE - (CurPhysicalAddress & 0x7FFFFFFFFFFF), TotalSize);
		SIZE_T ReadBytes = 0;

		NtRet = ReadPhysicalAddress((PVOID)CurPhysicalAddress, (PVOID)((ULONG64)Read_Buffer + CurOffset), ReadSize, &ReadBytes);

		TotalSize -= ReadBytes;
		CurOffset += ReadBytes;

		if (NtRet != STATUS_SUCCESS) break;

		if (ReadBytes == 0) break;
	}

	return NtRet;
}

NTSTATUS Memory::WriteProcessMemory(int Pid, PVOID Write_Address, PVOID Write_Buffer, SIZE_T Write_Size)
{
	PEPROCESS Process = nullptr;
	if (Pid == 0) return STATUS_UNSUCCESSFUL;

	NTSTATUS NtRet = PsLookupProcessByProcessId((HANDLE)Pid, &Process);
	if (NtRet != STATUS_SUCCESS) return NtRet;

	ULONG_PTR ProcessDirBase = GetProcessDirectoryTableBase(Process);
	ObDereferenceObject(Process);

	SIZE_T CurOffset = 0;
	SIZE_T TotalSize = Write_Size;

	while (TotalSize)
	{
		UINT64 CurPhysicalAddress = TranslateLinearAddress(ProcessDirBase, (ULONG64)Write_Address + CurOffset);
		if (!CurPhysicalAddress) return STATUS_UNSUCCESSFUL;

		ULONG64 WriteSize = min(PAGE_SIZE - (CurPhysicalAddress & 0x7FFFFFFFFFFF), TotalSize);
		SIZE_T WriteBytes = 0;

		NtRet = WritePhysicalAddress((PVOID)CurPhysicalAddress, (PVOID)((ULONG64)Write_Buffer + CurOffset), WriteSize, &WriteBytes);

		TotalSize -= WriteBytes;
		CurOffset += WriteBytes;

		if (NtRet != STATUS_SUCCESS) break;

		if (WriteBytes == 0) break;
	}

	return NtRet;
}
