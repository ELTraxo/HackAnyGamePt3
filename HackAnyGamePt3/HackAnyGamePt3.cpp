#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "proc.h"
#include "mem.h"

int main()
{
	HANDLE hProcess = 0;

	uintptr_t moduleBase = 0, localPlayerPtr = 0, healthAddr = 0;

	bool bHealth = false, bAmmo = false, bRecoil = false;

	const int newValue = 1337;

	//Get ProcId of the target process
	DWORD procId = GetProcId(L"ac_client.exe");

	if (procId)
	{
		//Get Handle to Process
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

		//Getmodulebaseaddress
		moduleBase = GetModuleBaseAddress(procId, L"ac_client.exe");

		//Resolve address
		localPlayerPtr = moduleBase + 0x10f4f4;

		//Resolve base address of the pointer chain
		healthAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0xF8 });
	}

	DWORD dwExit = 0;
	while (GetExitCodeProcess( hProcess, &dwExit ) && dwExit == STILL_ACTIVE)
	{
		//Health continuous write
		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			bHealth = !bHealth;
		}

		//unlimited ammo patch
		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			bAmmo = !bAmmo;

			if (bAmmo)
			{
				//FF 06 = inc [esi]
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x06", 2, hProcess);
			}

			else
			{
				//FF 0E = dec [esi]
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x0E", 2, hProcess);
			}
		}

		//no recoil NOP
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			bRecoil = !bRecoil;

			if (bRecoil)
			{
				mem::NopEx((BYTE*)(moduleBase + 0x63786), 10, hProcess);
			}

			else
			{
				//50 8D 4C 24 1C 51 8B CE FF D2; the original stack setup and call
				mem::PatchEx((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10, hProcess);
			}
		}

		//Continuous write
		if (bHealth)
		{
			mem::PatchEx((BYTE*)healthAddr, (BYTE*)&newValue, sizeof(newValue), hProcess);
		}

		Sleep(10);
	}

	std::cout << "Process not found, press enter to exit\n";
	getchar();
}