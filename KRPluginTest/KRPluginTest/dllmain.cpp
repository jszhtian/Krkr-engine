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
using std::wstring;
using std::string;
using std::wcout;

typedef HANDLE(WINAPI* fnCreateFileW)(LPCTSTR filename, DWORD acc, DWORD smo, LPSECURITY_ATTRIBUTES lpsec, DWORD credis, DWORD flags, HANDLE htmpf);
fnCreateFileW pcreatefile;
PVOID g_pOldMultiByteToWideChar = NULL;
PVOID g_pOldSetWindowTextW = NULL;
typedef int(WINAPI *PfuncMultiByteToWideChar)(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar);
typedef bool (WINAPI *PfuncSetWindowTextW)(
	_In_     HWND    hWnd,
	_In_opt_ LPCWSTR lpString
	);

bool FirstRUN = true;
bool ExtraFont = false;
bool FirstFont = true;
bool NextRun = false;
bool StartupSuccess = false;
bool debugmode = false;
char* checkName;
iTVPFunctionExporter *exporter = NULL;
std::wstring WindowTitle;

typedef HFONT(WINAPI* fnCreateFontIndirectW)(LOGFONTW *lplf);
fnCreateFontIndirectW pCreateFontIndirectW;
DWORD FunctionExporterAddress = 0;

typedef HWND(WINAPI* fnCreateWindowExW)(
	__in DWORD dwExStyle,
	__in_opt LPCWSTR lpClassName,
	__in_opt LPCWSTR lpWindowName,
	__in DWORD dwStyle,
	__in int X,
	__in int Y,
	__in int nWidth,
	__in int nHeight,
	__in_opt HWND hWndParent,
	__in_opt HMENU hMenu,
	__in_opt HINSTANCE hInstance,
	__in_opt LPVOID lpParam
);
fnCreateWindowExW pCreateWindowExW;
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
	if (debugmode)
	{
		cout << "Initialization..." << endl;
	}
	
	if (FunctionExporterAddress==0)
	{
		cout << "Compute Address Fail!" << endl;
		return -1;
	}
	else
	{
		exporter = (iTVPFunctionExporter *)FunctionExporterAddress;
	}
	//MessageBoxW(NULL, L"Load!", L"Info", 0);
	/*
	_asm 
	{
		//mov exporter, 0x01074538//MSG
		//mov exporter, 0x01094538//twinkle
		//mov exporter, 0x00BB4538//まいてつ
	}
	*/
	TVPInitImportStub(exporter);
	//ttstr tmp(L"Dummy Test!");
	//std::wcout << tmp.c_str() << endl;
	cout << "KRKR Function List:0x" << std::hex << (DWORD)exporter << endl;
	StartupSuccess = true;
	//Sleep(1500);
	//MessageBoxW(NULL, L"Start!", L"Info", 0);
	//cout << "AddAutoPath" << endl;
	//TVPAddAutoPath(L"F:\\tmp2\\MSG\\Project\\");
	return 0;
}

int AddPath() 
{
	//std::wcout << L"Add1 Path Start!" << endl;
	wchar_t *tmp=new wchar_t[MAX_PATH];
	memset(tmp, 0, sizeof(wchar_t)*MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, tmp);
	ttstr Path=tmp;
	Path = Path + L"\\Project\\";
	//wcout << "AddAutoPath:"<<Path.c_str()<< endl;
	TVPAddAutoPath(Path);
	return 0;
}

int AddPath1()
{
	//std::wcout << L"Add1 Path Start!" << endl;
	wchar_t *tmp = new wchar_t[MAX_PATH];
	memset(tmp, 0, sizeof(wchar_t)*MAX_PATH);
	GetCurrentDirectoryW(MAX_PATH, tmp);
	ttstr Path = tmp;
	Path = Path + L"\\ProjectPre\\";
	//wcout << "AddAutoPath:"<<Path.c_str()<< endl;
	TVPAddAutoPath(Path);
	return 0;
}
HANDLE WINAPI newcreatefile(LPCTSTR filename, DWORD acc, DWORD smo, LPSECURITY_ATTRIBUTES lpsec, DWORD credis, DWORD flags, HANDLE htmpf)
{
	if (debugmode)
	{
		
		std::cout << wtoc(filename) << endl;
	}
	
	

	if (FirstRUN)
	{

		//std::wcout << L"First RUN" << endl;
		checkName = wtoc(filename);
		std::string textstore;
		textstore = checkName;
		transform(textstore.begin(), textstore.end(), textstore.begin(), ::tolower);
		int pos1;
		pos1 = textstore.find("data.xp3", 0);
		if (pos1!=textstore.npos)
		{
			//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnStartUp, NULL, 0, 0);
			//OnStartUp();
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnStartUp, NULL, 0, 0);
			FirstRUN = false;
			//std::wcout << L"First:" << FirstRUN << L"Second" << SecondRUN <<L"StartupSuccess"<<StartupSuccess<<endl;
		}

	}
	/*

	if (StartupSuccess)
	{
		checkName = wtoc(filename);
		std::string textstore;
		textstore = checkName;
		transform(textstore.begin(), textstore.end(), textstore.begin(), ::tolower);
		int pos1;
		pos1 = textstore.find("evimage.xp3", 0);
		if (pos1 != textstore.npos)
		{
			NextRun = true;
		}
	}
	if (NextRun)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AddPath, NULL, 0, 0);
		NextRun = false;
	}

	*/
	

	return pcreatefile(filename, acc, smo, lpsec, credis, flags, htmpf);
}

int WINAPI NewMultiByteToWideChar(UINT cp, DWORD dwFg, LPCSTR lpMBS, int cbMB, LPWSTR lpWCS, int ccWC)
{
	//MessageBoxW(0, L"Hooked!", L"Info", 0);
	return((PfuncMultiByteToWideChar)g_pOldMultiByteToWideChar)(932, dwFg, lpMBS, cbMB, lpWCS, ccWC);
}

bool WINAPI NewSetWindowTextW(HWND    hWnd, LPCWSTR lpString)
{
	if (!StrCmpW(WindowTitle.c_str(),L"NULL"))
	{
		return((PfuncSetWindowTextW)g_pOldSetWindowTextW)(hWnd, lpString);
	}
	else
	{
		wchar_t test[] = L"千恋＊万花";
		int res = memcmp(lpString, test, sizeof(wchar_t) * 5);
		if (res == 0)
		{
			if (debugmode)
			{
				cout << "Change title!" << endl;
			}
			return((PfuncSetWindowTextW)g_pOldSetWindowTextW)(hWnd, WindowTitle.c_str());
		}
		else
		{
			return((PfuncSetWindowTextW)g_pOldSetWindowTextW)(hWnd, lpString);
		}
	}
	
}

HFONT WINAPI newCreateFontIndirectW(LOGFONTW *lplf)
{
	
	/*
	LOGFONTW lf;
	memcpy(&lf, lplf, sizeof(LOGFONTW));
	if (!ExtraFont)
	{
	wcscpy(lf.lfFaceName, L"微软雅黑");
	}
	else
	{
	if (FirstFont)
	{
	wcout << "Extra Font Find!" << endl;
	}

	wcscpy(lf.lfFaceName, L"Source Han Serif CN");
	}
	*/

	if (FirstFont&&StartupSuccess)
	{
		if (debugmode)
		{
			cout << "Load Path!" << endl;
		}
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AddPath, NULL, 0, 0);
		FirstFont = false;
	}
	
	
	return pCreateFontIndirectW(lplf);
}

HWND WINAPI newCreateWindowEXW(
	__in DWORD dwe,
	__in_opt LPCWSTR lpcn,
	__in_opt LPCWSTR lpwn,
	__in DWORD dws,
	__in int xpos,
	__in int ypos,
	__in int nw,
	__in int nh,
	__in_opt HWND hwndp,
	__in_opt HMENU hm,
	__in_opt HINSTANCE hi,
	__in_opt LPVOID lpp)
{
	
	if (StartupSuccess)
	{
		if (debugmode)
		{
			cout << "Load PrePath!" << endl;
		}
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AddPath1, NULL, 0, 0);
	}
	return pCreateWindowExW(dwe, lpcn, lpwn, dws, xpos, ypos, nw, nh, hwndp, hm, hi, lpp);
}


/*
bool addFont()
{
int res = 0;
res = AddFontResourceW(L"SourceHanSerifCN.otf");
if (res == 0)
{
return false;
}
else
{
return true;
}
}
*/


void BeginDetour() 
{
	pcreatefile = (fnCreateFileW)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "CreateFileW");
	pCreateFontIndirectW = (fnCreateFontIndirectW)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontIndirectW");
	pCreateWindowExW = (fnCreateWindowExW)GetProcAddress(GetModuleHandle(L"USER32.dll"), "CreateWindowExW");
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	g_pOldSetWindowTextW = DetourFindFunction("User32.dll", "SetWindowTextW");
	DetourTransactionBegin();
	DetourAttach((void**)&pcreatefile, newcreatefile);	
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	DetourAttach(&g_pOldSetWindowTextW, NewSetWindowTextW);
	DetourAttach((void**)&pCreateFontIndirectW, newCreateFontIndirectW);
	DetourAttach((void**)&pCreateWindowExW, newCreateWindowEXW);
	DetourTransactionCommit();
}

void ComputeAddress()
{
	cout << "EXE Base Address:0x" << hex << (DWORD)GetModuleHandle(NULL) << endl;
	FunctionExporterAddress = (DWORD)GetModuleHandle(NULL) + 0x3B4538;
	cout << "Compute Result:0x" << hex << FunctionExporterAddress << endl;
}

BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(lpPath, &fd);
	BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
	BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
	FindClose(hFind);
	return RetValue;
}

BOOL FilePathExists(LPCTSTR lpPath)
{
	return FindFirstFileExists(lpPath, FALSE);
}

void CheckComputer()
{
	std::hash<std::wstring > h;
	std::unique_ptr<wchar_t> username(new wchar_t[128]);
	std::unique_ptr<wchar_t> computername(new wchar_t[128]);
	//username = new wchar_t[128];
	//computername = new wchar_t[128];
	DWORD dwUserNameLen = 128;
	DWORD dwComputerNameLen = 128;
	GetUserNameW(username.get(), &dwUserNameLen);
	GetComputerNameW(computername.get(), &dwComputerNameLen);
	std::wstring checkstr = computername.get();
	checkstr += username.get();
	size_t result = h(checkstr);
	if (!FindFirstFileExists(L"check.bin", FALSE))
	{
		std::ofstream crt("check.bin");
		crt.close();
	}

	std::fstream file1;
	file1.open("check.bin", std::ios::in);
	std::string getinfo;
	file1 >> getinfo;
	file1.close();
	if (getinfo == "")
	{
		getinfo = '0';
	}
	size_t chk = std::stoll(getinfo);
	if (chk != result)
	{
		MessageBoxW(NULL, L"本补丁免费发布，且不可用于任何商业用途。\n如果您通过淘宝等途径获得本补丁，请立即给卖家差评！\n未经过汉化组允许禁止以录播直播等形式使用此补丁内容!", L"重要信息", MB_OK | MB_ICONEXCLAMATION);
		file1.open("check.bin", std::ios::out);
		file1 << std::to_string(result);
		file1.close();
	}

}


void precheck()
{
	CheckComputer();
	LPTSTR lpPath = new wchar_t[MAX_PATH];
	StrCpyW(lpPath, L".\\KRPlugin.ini");
	int debugswitch=-1;
	debugswitch = GetPrivateProfileIntW(L"DEBUG", L"DEBUGMODE", -1, lpPath);
	switch (debugswitch)
	{
	case 1:
		MessageBoxW(NULL, L"debug已启用！", L"消息", 0);
		debugmode = true;
		break;
	case 0:
		debugmode = false;
		break;
	case -1:
		debugmode = false;
		break;
	default:
		MessageBoxW(NULL, L"未找到配置文件！", L"警告", 0);
		break;
	}
	for (int j=0;j<10;j++)
	{
		std::wstring checkName;
		checkName += L"ARC";
		checkName+=std::to_wstring(j);
		//MessageBoxW(NULL, checkName.c_str(), L"Test", 0);
		wchar_t Getinfo[255];
		memset(Getinfo, 0, 255 * sizeof(wchar_t));
		GetPrivateProfileStringW(L"Archive", checkName.c_str(), NULL, Getinfo, 255, lpPath);
		wchar_t GetTitle[255];
		GetPrivateProfileStringW(L"CONFIG", L"TITLE", NULL, GetTitle, 255, lpPath);
		if (!StrCmpW(GetTitle,L""))
		{
			WindowTitle = L"千恋＊万花【落樱汉化组】";
		}
		else
		{
			WindowTitle = GetTitle;
		}
		//MessageBoxW(NULL, Getinfo, L"Test", 0);
		if (!StrCmpW(Getinfo,L""))
		{
			break;
		}
		else
		{
			bool checkresult = FilePathExists(Getinfo);
			std::wstring text = L"丢失以下文件：";
			text += Getinfo;
			if (checkresult==false)
			{
				MessageBoxW(NULL, text.c_str(), L"警告", 0);
				exit(-1);
			}
		}
	}

}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//make_console();

		precheck();
		if (debugmode)
		{
			make_console();
		}
		ComputeAddress();
		BeginDetour();
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnStartUp, NULL, 0, 0);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) void dummy(void) {
	return;
}