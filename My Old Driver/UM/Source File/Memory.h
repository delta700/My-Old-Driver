#pragma once
#include <Windows.h>
#include <iostream>

#include <TlHelp32.h>
#include <vector>

extern DWORD64 ProcessId;
extern DWORD64 BaseAddress;

typedef struct Request
{
	int Pid;
	PVOID Base;

	BOOLEAN Request_Base;
	BOOLEAN Request_Read;
	BOOLEAN Request_Write;

	PVOID Read_Buffer;
	PVOID Write_Buffer;

	uintptr_t Address;
	ULONGLONG Size;
}Request;

namespace Memory
{
	void InitializeDriver();

	DWORD64 GetProcessId(LPCTSTR Name);
	DWORD64 GetBaseAddress();

	void ReadMemory(PVOID Read_Address, PVOID Read_Buffer, DWORD Read_Size);
	void WriteMemory(PVOID Write_Address, PVOID Write_Buffer, DWORD Write_Size);

	std::string ReadString(uintptr_t Address);
	std::string ReadStringPointer(uintptr_t Address);

	template<typename T> T Read(uintptr_t Address)
	{
		T _Value;
		ReadMemory((PVOID)Address, &_Value, sizeof(T));
		return _Value;
	}
	template<typename T> bool Write(uintptr_t Address, const T& _Value)
	{
		return WriteMemory(__int64(Address), (UINT_PTR)&_Value, sizeof(T));
	}
}
