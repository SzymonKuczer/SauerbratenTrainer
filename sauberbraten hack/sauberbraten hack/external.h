#pragma once//tells the compiler to include this header only once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>

//returns the procId of our game Process
DWORD GetProcId(const wchar_t* procName);

//gets the address where a module of our game got loaded to:
uintptr_t GetModuleBaseAddress(DWORD procID, const wchar_t* modName);

//find dynamic address a multi level pointer is pointing to:
uintptr_t FindDynamicAddress(
	HANDLE hProc,
	uintptr_t ptr,
	std::vector<unsigned int> offsets
);

//write new code to memory
void PatchExternal(
	HANDLE hProcess,
	BYTE* dst,
	BYTE* src,
	size_t size
);

//class for replacing code with code that does nothing:
class NopExternal
{
private:
	HANDLE hProcess; // hold the handle to our game
	BYTE* dst; //address of the code to Patch
	size_t size; //size of the Patch
	BYTE* originalCode; //backup of the original code
	BYTE* nopCode; //code that does nothing
public:
	NopExternal(HANDLE hProcess, BYTE* dst, BYTE* src, size_t size); //constructor
	~NopExternal(); //destructor
	void enable(); //enable the patch
	void disable(); //disable the patch
};

