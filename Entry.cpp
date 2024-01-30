#include "Memory/Memory.h"
#include "Clean/Clean.h"

NTSTATUS Hook(uintptr_t _Reqest)
{
    Reqest* Req = (Reqest*)_Reqest;

    if (Req->Reqest_Base != false)
    {
        Req->Base = Memory::GetBaseAddress(Req->Pid);
    }

    if (Req->Reqest_Read != false)
    {
        Memory::ReadProcessMemory(Req->Pid, (PVOID)Req->Address, Req->Read_Buffer, Req->Size);
    }

    if (Req->Reqest_Write != false)
    {
        Memory::WriteProcessMemory(Req->Pid, (PVOID)Req->Address, Req->Write_Buffer, Req->Size);
    }

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry()
{
    uintptr_t Base = Utils::GetModulesBase(skCrypt("win32k.sys"));

    uintptr_t Address = Base + 414408;

    uintptr_t Pointer = *reinterpret_cast<uintptr_t*>(Address);
    *reinterpret_cast<uintptr_t*>(Address) = reinterpret_cast<uintptr_t>(&Hook);

    Clean::CleanUnloadedDrivers();

    return STATUS_SUCCESS;
}
