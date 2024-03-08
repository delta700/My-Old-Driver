#include "Memory/Memory.h"
#include "Clean/Clean.h"

/*
*		Credit: Hook https://www.unknowncheats.me/forum/anti-cheat-bypass/425352-driver-communication-using-data-ptr-called-function.html
*             
*		Author:  hxxven https://github.com/hxxven10
*/

void RequestThread()
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

    //NtUserShowSystemCursor
    uintptr_t Address = Base + 0x65800;

    *reinterpret_cast<uintptr_t*>(Address) = reinterpret_cast<uintptr_t>(&Hook);

    Clean::CleanUnloadedDrivers();

    return STATUS_SUCCESS;
}
