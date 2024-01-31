#include "Memory.h"

DWORD64 BaseAddress{};
DWORD64 ProcessId{};

typedef INT64(*memory)(uintptr_t);
memory Hook= nullptr;

void Memory::InitDriver()
{
	LoadLibraryA("user32.dll");
	Hook = (memory)GetProcAddress(LoadLibraryA("win32u.dll"), ("NtUserShowSystemCursor"));
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
	Req Reqest;
	Reqest.Reqest_Base = true;
	Reqest.Pid = ProcessId;
	Hook(reinterpret_cast<uintptr_t>(&Reqest));
	return (DWORD64)Reqest.Base;
}

void Memory::ReadMemory(UINT_PTR Read_Address, void* Buffer, size_t Read_Size)
{
	Req Reqest;
	Reqest.Reqest_Read = true;
	Reqest.Pid = ProcessId;
	Reqest.Read_Buffer = Buffer;
	Reqest.Address = Read_Address;
	Reqest.Size = Read_Size;
	Hook(reinterpret_cast<uintptr_t>(&Reqest));
}

BOOL Memory::WriteMemory(UINT_PTR Write_Address, UINT_PTR Source_Address, SIZE_T Write_Size)
{
	Req Reqest;
	Reqest.Reqest_Write = true;
	Reqest.Pid = ProcessId;
	Reqest.Write_Buffer = (void*)Source_Address;
	Reqest.Address = Write_Address;
	Reqest.Size = Write_Size;
	Hook(reinterpret_cast<uintptr_t>(&Reqest));
	return true;
}

std::string Memory::ReadString(UINT_PTR Address)
{
	Req Reqest;
	std::vector<char> buffer(sizeof(std::string), char{ 0 });
	Reqest.Reqest_Read = true;
	Reqest.Pid = ProcessId;
	Reqest.Read_Buffer = static_cast<void*>(&buffer[0]);
	Reqest.Address = Address;
	Reqest.Size = buffer.size();
	Hook(reinterpret_cast<uintptr_t>(&Reqest));
	return std::string(buffer.data());
}

std::string Memory::ReadStringPointer(UINT_PTR Address)
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