#include "Memory.h"

int main()
{
	Memory::InitializeDriver();
	ProcessId = Memory::GetProcessId(L"explorer.exe");
	BaseAddress = Memory::GetBaseAddress();

	printf("ProcessId: 0x%llX \n", ProcessId);
	printf("BaseAddress: 0x%llX \n", BaseAddress);

	system("Pause");
}
