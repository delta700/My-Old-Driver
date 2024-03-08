#include "Memory/Memory.h"
#include "Clean/Clean.h"

/*
*		Credit: Hook https://www.unknowncheats.me/forum/anti-cheat-bypass/425352-driver-communication-using-data-ptr-called-function.html
*             
*		Author:  hxxven https://github.com/hxxven10
*/

void RequestThread(uintptr_t _Request)
{
    Request* Req = (Request*)_Request;

    if (Req->Request_Base != false)
    {
        Req->Base = Memory::GetBaseAddress(Req->Pid);
    }

    if (Req->Request_Read != false)
    {
        Memory::ReadProcessMemory(Req->Pid, (PVOID)Req->Address, Req->Read_Buffer, Req->Size);
    }

    if (Req->Request_Write != false)
    {
        Memory::WriteProcessMemory(Req->Pid, (PVOID)Req->Address, Req->Write_Buffer, Req->Size);
    }

}

NTSTATUS DriverEntry()
{
    uintptr_t Base = Util::GetModuleBase(skCrypt("win32k.sys"));

    //NtUserShowSystemCursor
    uintptr_t Address = Base + 0x65800;

    *reinterpret_cast<uintptr_t*>(Address) = reinterpret_cast<uintptr_t>(&RequestThread);

    Clean::CleanUnloadedDrivers();

    return STATUS_SUCCESS;
}
