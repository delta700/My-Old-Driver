#include "Memory.h"

int main() 
{
	Memory::InitDriver();
	ProcessId = Memory::GetProcessId(L"explorer.exe");
	BaseAddress = Memory::GetBaseAddress();
    
	printf("ProcessId : % llX \n", ProcessId);
	printf("BaseAddress : % llX \n", BaseAddress);

	system("pause");
}