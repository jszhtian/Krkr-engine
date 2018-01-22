#include "stdafx.h"
#include "TVPHook.h"
#include "shlwapi.h"
#include "tp_stub.h"
#include "detours.h"
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Shlwapi.lib")
TVPHook* TVPHook::handle = NULL;

char* wtoc(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char *out = new char[dwMinSize];
	WideCharToMultiByte(CP_OEMCP, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

int WINAPI NewMultiByteToWideChar(UINT cp, DWORD dwFg, LPCSTR lpMBS, int cbMB, LPWSTR lpWCS, int ccWC)
{
	//MessageBoxW(0, L"Hooked!", L"Info", 0);
	auto obj = GetobjTVPhook();
	UINT outcp;
	switch (cp)
	{
	case CP_ACP:
	case CP_OEMCP:
	case CP_THREAD_ACP:
		outcp = 932;
		break;

	default:
		break;
	}
	return((PfuncMultiByteToWideChar)obj->g_pOldMultiByteToWideChar)(outcp, dwFg, lpMBS, cbMB, lpWCS, ccWC);
}

HMODULE WINAPI HookLoadLibraryW(LPCWSTR lpLibFileName)
{
	HMODULE       Module;
	TVPHook*  hookobj;


	hookobj = GetobjTVPhook();
	Module = ((PfuncLoadLibraryW)hookobj->g_pOldLoadLibraryW)(lpLibFileName);
	hookobj->initKrkrHook(lpLibFileName, Module);
	return Module;
}


TVPHook::TVPHook()
{
	g_POldV2Link = NULL;
	g_POldCreateStream = NULL;
	g_pOldMultiByteToWideChar = NULL;
	g_pOldLoadLibraryW = NULL;
	inited = false;
}


TVPHook::~TVPHook()
{
}

bool TVPHook::init()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	g_pOldLoadLibraryW= DetourFindFunction("Kernel32.dll", "LoadLibraryW");
	DetourAttach(&g_pOldLoadLibraryW, HookLoadLibraryW);
	DetourTransactionCommit();
	return true;
}


PVOID GetTVPCreateStreamAddress()
{
	auto baseAddr = (DWORD)GetModuleHandle(NULL);
	PVOID CallIStreamStub = (PVOID)(baseAddr + 0xF3A0);
	return CallIStreamStub;
}

class tTJSCriticalSection
{
	CRITICAL_SECTION CS;
public:
	tTJSCriticalSection() { InitializeCriticalSection(&CS); }
	~tTJSCriticalSection() { DeleteCriticalSection(&CS); }

	void Enter() { EnterCriticalSection(&CS); }
	void Leave() { LeaveCriticalSection(&CS); }
};

class tTJSCriticalSectionHolder
{
	tTJSCriticalSection *Section;
public:
	tTJSCriticalSectionHolder(tTJSCriticalSection &cs)
	{
		Section = &cs;
		Section->Enter();
	}

	~tTJSCriticalSectionHolder()
	{
		Section->Leave();
	}

};

static tTJSCriticalSection LocalCreateStreamCS;

std::wstring GetKrkrFileName(LPCWSTR Name)
{
	std::wstring Info(Name);

	if (Info.find_last_of(L">") != std::wstring::npos)
		Info = Info.substr(Info.find_last_of(L">") + 1, std::wstring::npos);

	if (Info.find_last_of(L"/") != std::wstring::npos)
		Info = Info.substr(Info.find_last_of(L"/") + 1, std::wstring::npos);

	return Info;
}

void FileNameToLower(std::wstring& FileName)
{
	for (auto& Item : FileName)
	{
		if (Item <= L'Z' && Item >= L'A')
			Item += towlower(Item);
	}
}

IStream * TVPHook::CreateLocalStream(LPCWSTR lpFileName)
{
	tTJSCriticalSectionHolder CSHolder(LocalCreateStreamCS);
	std::wstring FileName;
	FileName = GetKrkrFileName(lpFileName);
	FileNameToLower(FileName);
	std::wstring NewPath = L"Project\\" + FileName;
	//std::cout << "NewPath:" << wtoc(NewPath.c_str()) << std::endl;
	IStream* *Stream;
	auto hr=SHCreateStreamOnFileEx(NewPath.c_str(), STGM_READ | STGM_SHARE_EXCLUSIVE, 0, FALSE, NULL, Stream);
	if (SUCCEEDED(hr))
	{
		return *Stream;
	}
	else
	{
		return NULL;
	}
	
}


tTJSBinaryStream* __fastcall HookTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	TVPHook*      objhook;
	tTJSBinaryStream*  Stream;
	IStream*           IStream;

	objhook = GetobjTVPhook();
	
	IStream = objhook->CreateLocalStream(_name.c_str());
	if (IStream)
	{
		Stream = TVPCreateBinaryStreamAdapter(IStream);
		if (!Stream)
		{
			Stream = ((PfuncCreateStream)objhook->g_POldCreateStream)(_name, flags);
		}
		else
		{
			std::cout << "REPLACE FILE:" << wtoc(_name.c_str()) << std::endl;
		}
	}
	else
	{
		Stream = ((PfuncCreateStream)objhook->g_POldCreateStream)(_name, flags);
	}
	return Stream;
}

HRESULT __stdcall HookV2Link(iTVPFunctionExporter *exporter)
{
	TVPHook* objTVPhook = GetobjTVPhook();
	if (!objTVPhook->inited)
	{
		TVPInitImportStub(exporter);
		objTVPhook->TVPFunctionExporter = exporter;
		std::cout << "KRKR Function List:0x" << std::hex << (DWORD)exporter << std::endl;
		auto res=GetTVPCreateStreamAddress();
		objTVPhook->g_POldCreateStream = res;
		std::cout << "TVPCreateStream:0x" << std::hex << (DWORD)res << std::endl;
		if (res!=NULL)
		{
			DetourTransactionBegin();
			DetourAttach(&objTVPhook->g_POldCreateStream, HookTVPCreateStream);
			DetourTransactionCommit();
		}
		objTVPhook->inited = true;
	}
	return ((PfuncV2Link)objTVPhook->g_POldV2Link)(exporter);
}

void TVPHook::initKrkrHook(LPCWSTR lpFileName, HMODULE Module)
{
	auto extStr = PathFindExtensionW(lpFileName);
	TVPHook* objTVPhook = GetobjTVPhook();
	if (StrCmpW(extStr,L".dll")==0|| StrCmpW(extStr, L".tpm") == 0)
	{
		auto pV2Link = GetProcAddress(Module, "V2Link");
		objTVPhook->g_POldV2Link = pV2Link;
		if (pV2Link!=NULL&&!inited)
		{
			DetourTransactionBegin();
			DetourAttach(&objTVPhook->g_POldV2Link, HookV2Link);
			DetourTransactionCommit();
		}
	}
}


TVPHook * GetobjTVPhook()
{
	if (TVPHook::handle==NULL)
	{
		TVPHook::handle = new TVPHook;	
	}
	return TVPHook::handle;
}

bool TVPHook::uninit()
{
	DetourDetach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	DetourDetach(&g_pOldLoadLibraryW, HookLoadLibraryW);
	DetourDetach(&g_POldV2Link, HookV2Link);
	g_POldV2Link = NULL;
	g_POldCreateStream = NULL;
	g_pOldMultiByteToWideChar = NULL;
	g_pOldLoadLibraryW = NULL;
	return false;
}