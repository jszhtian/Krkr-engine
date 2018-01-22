#include "stdafx.h"
#include "TVPHook.h"
#include "shlwapi.h"
#include "tp_stub.h"
#include "detours.h"
#include <algorithm> 
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
PVOID GetTVPCreateStreamAddress()
{
	auto baseAddr = (DWORD)GetModuleHandle(NULL);
	//FIND TVPCreateStream at TVPCreateIStream Proc, the First Call
	PVOID CallIStreamStub = (PVOID)(baseAddr + 0xF3A0);
	return CallIStreamStub;
}

iTVPFunctionExporter* GetTVPFunctionExporter()
{
	auto baseAddr = (DWORD)GetModuleHandle(NULL);
	//FIND TVPFunctionExporter at LoadLibraryW position
	auto TVPFunctionExporter = (iTVPFunctionExporter*)(baseAddr + 0x3B4538);
	return TVPFunctionExporter;
}

tTJSBinaryStream* __fastcall HookTVPCreateStream(const ttstr & _name, tjs_uint32 flags)
{
	TVPHook*      objhook;
	objhook = GetobjTVPhook();
	if (!objhook->inited)
	{
		objhook->TVPFunctionExporter = GetTVPFunctionExporter();
		TVPInitImportStub(objhook->TVPFunctionExporter);
		std::cout << "KRKR Function List:0x" << std::hex << (DWORD)objhook->TVPFunctionExporter << std::endl;
		objhook->inited = true;
	}
	tTJSBinaryStream*  Stream;
	IStream*           IStream;

	IStream = objhook->CreateLocalStream(_name.c_str());
	//std::cout << "LOAD FILE:" << wtoc(_name.c_str()) << std::endl;
	if (IStream)
	{
		Stream = TVPCreateBinaryStreamAdapter(IStream);
		if (!Stream)
		{
			std::cout << "REPLACE FILE FAIL:" << wtoc(_name.c_str()) << std::endl;
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


bool TVPHook::init()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	DetourTransactionCommit();
	auto res = GetTVPCreateStreamAddress();
	g_POldCreateStream = res;
	std::cout << "TVPCreateStream:0x" << std::hex << (DWORD)res << std::endl;
	if (res != NULL)
	{
		DetourTransactionBegin();
		DetourAttach(&g_POldCreateStream, HookTVPCreateStream);
		DetourTransactionCommit();
	}
	return true;
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
	std::transform(FileName.begin(), FileName.end(), FileName.begin(), ::tolower);
}

IStream * TVPHook::CreateLocalStream(LPCWSTR lpFileName)
{
	tTJSCriticalSectionHolder CSHolder(LocalCreateStreamCS);
	std::wstring FileName;
	FileName = GetKrkrFileName(lpFileName);
	//std::cout << "CreateStream:" << wtoc(FileName.c_str()) << std::endl;
	FileNameToLower(FileName);
	std::wstring NewPath = L"Project\\" + FileName;
	
	IStream* pStream;
	auto hr=SHCreateStreamOnFileEx(NewPath.c_str(), STGM_READ, 0, FALSE, NULL, &pStream);
	if (SUCCEEDED(hr))
	{
		return pStream;
	}
	else
	{
		//std::cout << "HRESULT:" <<std::hex<<hr << std::endl;
		return NULL;
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
	g_POldV2Link = NULL;
	g_POldCreateStream = NULL;
	g_pOldMultiByteToWideChar = NULL;
	g_pOldLoadLibraryW = NULL;
	return false;
}