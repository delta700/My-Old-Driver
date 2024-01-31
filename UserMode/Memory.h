#pragma once
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <vector>

extern DWORD64 BaseAddress;
extern DWORD64 ProcessId;

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
}Req;

namespace Memory
{
	void InitDriver();
	DWORD64 GetProcessId(LPCTSTR Name);
	DWORD64 GetBaseAddress();
	void ReadMemory(UINT_PTR Read_Address, void* Buffer, size_t Read_Size);
	BOOL WriteMemory(UINT_PTR Write_Address, UINT_PTR Source_Address, SIZE_T Write_Size);
	std::string ReadString(UINT_PTR Address);
	std::string ReadStringPointer(UINT_PTR Address);

	template<typename T> T Read(uintptr_t Address)
	{
		T _Value;
		ReadMemory(Address, &_Value, sizeof(T));
		return _Value;
	}
	template<typename T> bool Write(uintptr_t Address, const T& _Value)
	{
		return WriteMemory(__int64(Address), (UINT_PTR)&_Value, sizeof(T));
	}
}