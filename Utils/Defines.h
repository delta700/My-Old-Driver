#pragma once
#include <ntifs.h>
#include <windef.h>
#include <cstdint>

#include "../Encryption/Encrypt_String.h"

typedef struct Reqest
{
	ULONG Pid;
	PVOID Base;
	BOOLEAN Reqest_Base;
	BOOLEAN Reqest_Read;
	BOOLEAN Reqest_Write;
	void* Read_Buffer;
	void* Write_Buffer;
	UINT_PTR Address;
	ULONGLONG Size;
}Reqest;

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
	SystemModuleInformation = 11
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

struct PiDDBCache
{
	LIST_ENTRY List;
	UNICODE_STRING DriverName;
	ULONG TimeDateStamp;
	NTSTATUS LoadStatus;
	char _0x0028[16];
};

extern "C" NTKERNELAPI NTSTATUS ZwQuerySystemInformation(ULONG InfoClass, PVOID Buffer, ULONG Length, PULONG ReturnLength);
extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);