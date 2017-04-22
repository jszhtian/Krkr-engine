// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#define HOOK_BYTES 6  
#pragma pack(push, 4)//For Krkr2/Z

char old_opcode[HOOK_BYTES];
char new_opcode[HOOK_BYTES] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
unsigned int hookAddr = 0;
typedef unsigned int uint;
struct tTVPXP3ExtractionFilterInfo {
	unsigned __int32 SizeofStruct;
	unsigned __int64 offset;
	void* buffer;
	unsigned __int32 bufSize;
	unsigned __int32 FileHash;
};
BYTE tar[] = { 0xA5,0xA5,0xAB,0xD2,0xDA,0xB9,0xCC,0xC3,0xD8,0xD9,0xDE,0xCC,0xC3,0xC6,0xCF,0xAB };
tTVPXP3ExtractionFilterInfo testinfo;

void setinfo()//init tTVPXP3ExtractionFilterInfo for test
{
	testinfo.buffer = &tar;
	testinfo.offset = 0;
	testinfo.FileHash = 0x04101542;
	testinfo.bufSize = 0x10;
	testinfo.SizeofStruct = sizeof(testinfo);
}

void hook()//Hook TVPXP3ArchiveExtractionFilter
{
	DWORD dwflag;
	if (VirtualProtect((void*)hookAddr, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &dwflag))
	{
		memcpy(old_opcode, (void*)hookAddr, HOOK_BYTES);
		memcpy((void*)hookAddr, new_opcode, HOOK_BYTES);
		VirtualProtect((void*)hookAddr, HOOK_BYTES, dwflag, &dwflag);
	}
	else
	{
		MessageBoxW(0, L"Error!", L"info", 0);
	}
}

void __stdcall Exec(tTVPXP3ExtractionFilterInfo* input)//True TVPXP3ArchiveExtractionFilter Function
{
	__asm
	{
		push input
		call dword ptr ds : [0x710428]
	}
}
bool Fixed = false;
void TPMFix() // Pass TPM SEH Check
{
	if (Fixed==false)
	{
		HANDLE htpm = GetModuleHandle(L"cloverdays.tpm");
		unsigned __int32 targetAddr = (unsigned __int32)htpm + 0x2144;
		DWORD dwflag;
		unsigned char opCode[6] = { 0xE9,0xD1,0x00,0x00,0x00,0x00 };
		VirtualProtect((void*)targetAddr, 6, PAGE_EXECUTE_READWRITE, &dwflag);
		memcpy((void*)targetAddr, opCode, 6);
		VirtualProtect((void*)targetAddr, HOOK_BYTES, dwflag, &dwflag);
		Fixed = true;
	}
	
}

void __stdcall FakeTVPXP3ArchiveExtractionFilter(tTVPXP3ExtractionFilterInfo* info1) //Fake Function
{
	
	//MessageBoxW(0, L"Success!", L"info", 0);
	TPMFix();
	Exec(info1);
	Exec(&testinfo);//Exec customize Encrypt Block
}
#pragma pack(push, 2)
void sethookbytes(uint addr)//Calc Hook Addr
{
	setinfo();
	hookAddr = addr;
	new_opcode[0] = (char)0xE8;
	(uint&)new_opcode[1] = (uint)FakeTVPXP3ArchiveExtractionFilter - addr - 5;
	hook();
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hookAddr = 0x00603ba0;//Hook Addr
		sethookbytes(hookAddr);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

