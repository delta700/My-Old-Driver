#include "Memory/Memory.h"
#include "Clean/Clean.h"

/*
* Credit: Hook https://www.unknowncheats.me/forum/anti-cheat-bypass/425352-driver-communication-using-data-ptr-called-function.html
*
* Author: Hxxven https://github.com/hxxven10
*/

void RequestThread(uintptr_t _Request)
{
    DebugPrint("[+] RequestThread Called \n");
    
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

void CleanThread()
{
    DebugPrint("[+] CleanThread Called \n");

    Clean::CleanUnloadedDrivers();
}

NTSTATUS DriverEntry(PDRIVER_OBJECT Driver_Object, PUNICODE_STRING Registry_Path)
{
    UNREFERENCED_PARAMETER(Driver_Object);
    UNREFERENCED_PARAMETER(Registry_Path);

    DebugPrint("[+] Start Driver \n");

    uintptr_t Base = Util::GetModuleBase(skCrypt("win32k.sys"));

    if (!Base) {
        DebugPrint("[-] Faild to Base \n");
        return STATUS_FAILED_DRIVER_ENTRY;
    }

    DebugPrint("[+] Base: 0x%llX \n", Base);

    uintptr_t Offset = Base + 0x65F08;

    DebugPrint("[+] Offset: 0x%llX \n", Offset);

    *reinterpret_cast<uintptr_t*>(Offset) = reinterpret_cast<uintptr_t>(&RequestThread);

    CleanThread();

    DebugPrint("[+] Driver SuccessFully Loaded \n");

    return STATUS_SUCCESS;
}
