// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "tp_stub.h"
#include "detours.h"
#pragma comment(lib, "detours.lib")
using std::cin ;
using std::cout;
using std::endl;
using std::hex;
using std::dec;
typedef HANDLE(WINAPI* fnCreateFileW)(LPCTSTR filename, DWORD acc, DWORD smo, LPSECURITY_ATTRIBUTES lpsec, DWORD credis, DWORD flags, HANDLE htmpf);
fnCreateFileW pcreatefile;
PVOID g_pOldMultiByteToWideChar = NULL;
typedef int(WINAPI *PfuncMultiByteToWideChar)(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar);
bool FirstRUN = true;
bool FirstFont = true;
char* checkName;
iTVPFunctionExporter *exporter = NULL;
typedef HFONT(WINAPI* fnCreateFontIndirectW)(LOGFONTW *lplf);
fnCreateFontIndirectW pCreateFontIndirectW;
/*
00CE1710 / $  51                     push ecx;  MSG. < ModuleEntryPoint>
00CE1711 | .  803D 8D820701 00       cmp byte ptr ds : [0x107828D], 0x0
00CE1718 | .  75 0C                  jnz short MSG.00CE1726
00CE171A | .  C605 8D820701 01       mov byte ptr ds : [0x107828D], 0x1
00CE1721 | .  E8 5AF7FFFF            call MSG.00CE0E80
00CE1726 | >  B8 38450701            mov eax, MSG.01074538//iTVPFunctionExporter
00CE172B | .  59                     pop ecx;  kernel32.77038744


01091710  /$  51                     push ecx
01091711  |.  803D 8D824201 00       cmp byte ptr ds:[0x142828D],0x0
01091718  |.  75 0C                  jnz short twinkle.01091726
0109171A  |.  C605 8D824201 01       mov byte ptr ds:[0x142828D],0x1
01091721  |.  E8 5AF7FFFF            call twinkle.01090E80
01091726  |>  B8 38454201            mov eax,twinkle.01424538//iTVPFunctionExporter
0109172B  |.  59                     pop ecx                                  ;  PackinOn.0F4D0000
*/

int WINAPI NewMultiByteToWideChar(UINT cp, DWORD dwFg, LPCSTR lpMBS, int cbMB, LPWSTR lpWCS, int ccWC)
{
	//MessageBoxW(0, L"Hooked!", L"Info", 0);
	return((PfuncMultiByteToWideChar)g_pOldMultiByteToWideChar)(932, dwFg, lpMBS, cbMB, lpWCS, ccWC);
}
char* wtoc(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char *out = new char[dwMinSize];
	WideCharToMultiByte(CP_OEMCP, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

static void make_console() {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	cout << "Open Console Success!" << endl;
}

int OnStartUp() 
{
	cout << "Initialization..."<<endl;
	//MessageBoxW(NULL, L"Load!", L"Info", 0);
	_asm 
	{
		mov exporter, 0x1074538//MSG
		//mov exporter, 0x01094538//twinkle
	}
	TVPInitImportStub(exporter);
	ttstr tmp(L"Dummy Test!");
	std::wcout << tmp.c_str() << endl;
	cout << "KRKR Function List:0x" << std::hex << (DWORD)exporter << endl;
	//Sleep(1500);
	//MessageBoxW(NULL, L"Start!", L"Info", 0);
	//cout << "AddAutoPath" << endl;
	//TVPAddAutoPath(L"F:\\tmp2\\MSG\\Project\\");
	return 0;
}

int AddPath() 
{
	cout << "AddAutoPath" << endl;
	TVPAddAutoPath(L"F:\\tmp2\\MSG\\Project\\");
	return 0;
}

HANDLE WINAPI newcreatefile(LPCTSTR filename, DWORD acc, DWORD smo, LPSECURITY_ATTRIBUTES lpsec, DWORD credis, DWORD flags, HANDLE htmpf)
{
	//std::wcout << filename << endl;

	if (FirstRUN)
	{
		checkName = wtoc(filename);
		std::string textstore;
		textstore = checkName;
		transform(textstore.begin(), textstore.end(), textstore.begin(), ::tolower);
		int pos1;
		pos1 = textstore.find("data.xp3", 0);
		if (pos1!=textstore.npos)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnStartUp, NULL, 0, 0);
			FirstRUN = false;
		}

	}
	return pcreatefile(filename, acc, smo, lpsec, credis, flags, htmpf);
}

HFONT WINAPI newCreateFontIndirectW(LOGFONTW *lplf)
{
	if (FirstFont&&!FirstRUN)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AddPath, NULL, 0, 0);
		FirstFont = false;
	}
	return pCreateFontIndirectW(lplf);
}
void BeginDetour() 
{
	pcreatefile = (fnCreateFileW)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "CreateFileW");
	pCreateFontIndirectW = (fnCreateFontIndirectW)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontIndirectW");
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");

	DetourTransactionBegin();
	DetourAttach((void**)&pcreatefile, newcreatefile);	
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	DetourAttach((void**)&pCreateFontIndirectW, newCreateFontIndirectW);
	DetourTransactionCommit();
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		make_console();
		BeginDetour();
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnStartUp, NULL, 0, 0);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

