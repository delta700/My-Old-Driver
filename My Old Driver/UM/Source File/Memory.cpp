#include "Memory.h"

DWORD64 ProcessId;
DWORD64 BaseAddress;

typedef INT64(*memory)(uintptr_t);
memory Hook = nullptr;

void Memory::InitializeDriver()
{
	LoadLibraryA("user32.dll");
	Hook = (memory)GetProcAddress(LoadLibraryA("win32u.dll"), ("NtUserEvent"));
}

DWORD64 Memory::GetProcessId(LPCTSTR Name)
{
	PROCESSENTRY32 Pt{};
	HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Pt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(Snap, &Pt))
	{
		do {
			if (!lstrcmpi(Pt.szExeFile, Name))
			{
				CloseHandle(Snap);
				return Pt.th32ProcessID;
			}
		} while (Process32Next(Snap, &Pt));
	}
	CloseHandle(Snap);
	return 0;
}

DWORD64 Memory::GetBaseAddress()
{
	Request Request;

	Request.Pid = ProcessId;
	Request.Request_Base = true;

	Hook(reinterpret_cast<uintptr_t>(&Request));

	return (DWORD64)Request.Base;
}

void Memory::ReadMemory(PVOID Read_Address, PVOID Read_Buffer, DWORD Read_Size)
{
	Request Request;

	Request.Pid = ProcessId;
	Request.Request_Read = true;

	Request.Read_Buffer = Read_Buffer;
	Request.Address = (uintptr_t)Read_Address;
	Request.Size = Read_Size;

	Hook(reinterpret_cast<uintptr_t>(&Request));
}

void Memory::WriteMemory(PVOID Write_Address, PVOID Write_Buffer, DWORD Write_Size)
{
	Request Request;

	Request.Pid = ProcessId;
	Request.Request_Write = true;

	Request.Write_Buffer = Write_Buffer;
	Request.Address = (uintptr_t)Write_Address;
	Request.Size = Write_Size;

	Hook(reinterpret_cast<uintptr_t>(&Request));
}

std::string Memory::ReadString(uintptr_t Address)
{
	Request Request;

	std::vector<char> buffer(sizeof(std::string), char{ 0 });

	Request.Pid = ProcessId;
	Request.Request_Read = true;

	Request.Read_Buffer = static_cast<void*>(&buffer[0]);
	Request.Address = Address;
	Request.Size = buffer.size();

	Hook(reinterpret_cast<uintptr_t>(&Request));

	return std::string(buffer.data());
}

std::string Memory::ReadStringPointer(uintptr_t Address)
{
	if (Read<int>(Address + 16) > 15)
		Address = Read<uint32_t>(Address);

	std::string res;
	char buf;

	for (int i = 0; i < 4096; i++) {
		buf = Read<char>(Address + i);
		if (!buf)
			break;
		res += buf;
	}

	return res;
}