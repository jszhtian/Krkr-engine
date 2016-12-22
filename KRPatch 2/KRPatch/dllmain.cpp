// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#pragma warning(disable:4311)
#pragma warning(disable:4312)  

typedef unsigned char byte;
typedef unsigned short word;
unsigned char* buf;
typedef unsigned int ulong;
using namespace std;
int flag = 0;
char* text;
#define HOOK_BYTES 8  
typedef unsigned int uint;

uint hookAddr = 0;
char old_opcode[HOOK_BYTES] = {0x64,0x61,0x74,0x61,0x2E,0x78,0x70,0x33};
char new_opcode[HOOK_BYTES] = {0x64,0x61,0x74,0x61,0x2E,0x63,0x68,0x73};
char target_opecode[HOOK_BYTES] = { 0x64,0x61,0x74,0x61,0x2E,0x78,0x70,0x33 };

bool hook(void)
{
	DWORD dwflag;
	if (VirtualProtect((void*)hookAddr,HOOK_BYTES, PAGE_READWRITE,&dwflag))
	{
		memcpy(old_opcode, (void*)hookAddr, HOOK_BYTES);
		if (memcmp(old_opcode,target_opecode,HOOK_BYTES)!=0)
		{
//			MessageBoxW(0, L"Hook Fail! Unknown Target!", L"Warning", 0);
			return false;
		}
//		MessageBoxW(0, L"Hook Start!", L"Info", 0);
		memcpy((void*)hookAddr, new_opcode, HOOK_BYTES);
//		MessageBoxW(0, L"Hook Set!", L"Info", 0);
		VirtualProtect((void*)hookAddr, HOOK_BYTES, dwflag, &dwflag);
		return true;
	}
	else
	{
		MessageBoxW(0, L"Hook Fail! VirtualProtect Set Fail!", L"Warning", 0);
	}
	return false;
}

void sethookbytes(uint addr)
{
	hookAddr = addr;
}


void BeginDetour()
{
	uint hookAddr = 0x006F6BB9;
	sethookbytes(hookAddr);
//	MessageBoxW(0, L"Start Hook!", L"Info!", 0);
	hook();
//	MessageBoxW(0, L"Finish Hook!", L"Info!", 0);
}



BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
//		MessageBoxW(0, L"Hook!", L"Info!", 0);
		BeginDetour();
		break;
	default:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void dummy(void) {
	return;
}