#pragma once
#include <ntifs.h>
#include <windef.h>
#include <cstdint>

#include "../Encrypt/Encrypt_String.h"

#define DebugPrint(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " " __VA_ARGS__)

typedef struct Request
{
	int Pid;
	PVOID Base;

	BOOLEAN Request_Base;
	BOOLEAN Request_Read;
	BOOLEAN Request_Write;

	void* Read_Buffer;
	void* Write_Buffer;

	uintptr_t Address;
	ULONGLONG Size;
}Request;

typedef enum SystemInformationClass
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeofdayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation = 0xB
} SystemInformationClass,
* pSystemInformationClass;

typedef struct RtlProcessModuleInformation
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RtlProcessModuleInformation, * pRtlProcessModuleInformation;

typedef struct RtlProcessModules
{
	ULONG NumberOfModules;
	RtlProcessModuleInformation Modules[1];
} RtlProcessModules, * pRtlProcessModules;

extern "C" 
{
	NTKERNELAPI NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);
	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);
}
