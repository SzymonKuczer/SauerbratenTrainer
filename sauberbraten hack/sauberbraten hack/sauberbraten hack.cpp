#include <iostream>
#include "external.h"
#include <vector>

int main()
{
	//name of exe of our game
	wchar_t gameName[]{ L"sauerbraten.exe" };

	/* in cheat engine we found this static address
	of the player pointer (sauerbraten.exe + 2A3528): */
	size_t playerOffset{ 0x2A3528 };

	size_t healthOffset{ 0x00312930 };

	//get the process id of our game:
	DWORD procId = GetProcId(gameName);

	//get the address where the .exe got loaded inside our games process:
	uintptr_t moduleBase = GetModuleBaseAddress(procId, gameName);

	//get a handle to the process of our game with all access:
	HANDLE hProcess{ OpenProcess(PROCESS_ALL_ACCESS, NULL, procId) };

	//where the pointer to our player is located
	uintptr_t playerBase{ moduleBase + playerOffset };
	BYTE* playerAddress;

	uintptr_t healthBase{ moduleBase + healthOffset };

	//all our offsets for health
	std::vector<unsigned int> healthOffsets = { 0x340, 0x118, 0x0 };

	//finds the health address through multi level pointers 
	uintptr_t healthAddress = FindDynamicAddress(hProcess, healthBase, healthOffsets);

	//read the address where our player is from the player pointer
	ReadProcessMemory(hProcess, (BYTE*)playerBase, &playerAddress, sizeof(playerAddress), nullptr);



	//address of selected weapon
	BYTE* selectedWeaponAddress{ playerAddress + 0x018C };

	BYTE* ammoAddresses[]
	{
		(playerAddress + 0x01AC) , //static pistol ammo address	
		(playerAddress + 0x01A8) , //static Grenade launcher ammo
		(playerAddress + 0x01A0) , //static rocket launcher ammo
		(playerAddress + 0x0194) , //static chainsaw ammo
		(playerAddress + 0x019C) , //static mingun ammo
		(playerAddress + 0x01A4) , //static sniper ammo
	};

	const size_t weaponCount{ sizeof(ammoAddresses) / sizeof(BYTE*) };

	//variable to store our original health value in
	int originalHealth{ 100 };

	//variable to store the slected weapon id
	int selectedWeapon;

	//array to store our original ammo values in
	int originalAmmoValues[weaponCount];

	//new value we want to write
	int hackedValue{ 1337 };

	bool hackEnabled{ false };

	//our menu output/display:

	std::cout << "Hack Loaded \n"
		<< "Press F2 to toggle the cheat \n"
		<< "Use F3 to switch through weapons (also unlisted ones) \n"
		<< "Press END to exit the cheat!" << std::endl;

	DWORD exitCode{ 0 };

	//as long as our game is running:
	while (GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE)
	{
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			hackEnabled = !hackEnabled;

			if (hackEnabled) // backup original values
			{
				/* read the current health value from memory and store it in original health*/
				ReadProcessMemory(hProcess, (BYTE*)healthAddress, &originalHealth, sizeof(originalHealth), nullptr
				);

				//loop over each ammo address and backup its values:
				for (size_t i = 0; i < weaponCount; ++i)
				{
					//i-th address in the ammo address array
					//address of the i-th value
					ReadProcessMemory(hProcess, ammoAddresses[i], &originalAmmoValues[i], sizeof(originalAmmoValues[i]), nullptr);
				}
			}else
			{
				//restore original values
				//write the opriginal health value back
				WriteProcessMemory(hProcess,(BYTE*)healthAddress, &originalHealth, sizeof(originalHealth), nullptr);

				//loop over each ammo address and restore its values:
				for (size_t i = 0; i < weaponCount; ++i)
				{
					WriteProcessMemory(hProcess, ammoAddresses[i], &originalAmmoValues[i], sizeof(originalAmmoValues[i]), nullptr);
				}
			}
		}
			//if END gets pressed we end our cheat
			if (GetAsyncKeyState(VK_END))
			{
				break;
			}

		if (hackEnabled)
		{
			WriteProcessMemory(hProcess, (BYTE*)healthAddress, &hackedValue, sizeof(originalHealth), nullptr);

			//loop over each ammo address and write hackedValue to it:
			for (size_t i = 0; i < weaponCount; ++i)
			{
				WriteProcessMemory(hProcess, ammoAddresses[i], &hackedValue, sizeof(hackedValue), nullptr);
			}

			if (GetAsyncKeyState(VK_F3) & 1)
			{
				ReadProcessMemory(hProcess, selectedWeaponAddress, &selectedWeapon, sizeof(selectedWeapon), nullptr);

				//set selected weapon to the next one
				selectedWeapon = ++selectedWeapon % weaponCount;

				WriteProcessMemory(hProcess, selectedWeaponAddress, &selectedWeapon, sizeof(selectedWeapon), nullptr);
			}
		}
	}
	std::cout << "Cheat ended!" << std::endl;
	return 0;
}