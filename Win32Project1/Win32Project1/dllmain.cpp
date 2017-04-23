// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#define HOOK_BYTES 6  
#pragma pack(push, 4)

char old_opcode[HOOK_BYTES];
char new_opcode[HOOK_BYTES] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
unsigned int hookAddr = 0;
typedef unsigned int uint;
struct tjsStr
{
	__int32 refCount;
	wchar_t* LongStr;
	wchar_t ShortStr[0x16];
	__int32 Length;
	unsigned __int32 Heapflag;
	unsigned __int32 Hint;
};


class tTJSBinaryStream
{
private:
public:
	//-- must implement
	virtual tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence) = 0;
	/* if error, position is not changed */

	//-- optionally to implement
	virtual tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size) = 0;
	/* returns actually read size */

	virtual tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint write_size) = 0;
	/* returns actually written size */

	virtual void TJS_INTF_METHOD SetEndOfStorage();
	// the default behavior is raising a exception
	/* if error, raises exception */

	//-- should re-implement for higher performance
	virtual tjs_uint64 TJS_INTF_METHOD GetSize() = 0;

	virtual ~tTJSBinaryStream() { ; }

	tjs_uint64 GetPosition();

	void SetPosition(tjs_uint64 pos);

	void ReadBuffer(void *buffer, tjs_uint read_size);
	void WriteBuffer(const void *buffer, tjs_uint write_size);

	tjs_uint64 ReadI64LE(); // reads little-endian integers
	tjs_uint32 ReadI32LE();
	tjs_uint16 ReadI16LE();
};
void TJS_INTF_METHOD tTJSBinaryStream::SetEndOfStorage()
{

}

tjs_uint64 tTJSBinaryStream::GetPosition()
{
	return Seek(0, SEEK_CUR);
}

void tTJSBinaryStream::SetPosition(tjs_uint64 pos)
{
	Seek(pos, TJS_BS_SEEK_SET);
}

void tTJSBinaryStream::ReadBuffer(void *buffer, tjs_uint read_size)
{
	Read(buffer, read_size);
}

void tTJSBinaryStream::WriteBuffer(const void *buffer, tjs_uint write_size)
{
	Write(buffer, write_size);
}

tjs_uint64 tTJSBinaryStream::ReadI64LE()
{
	tjs_uint64 temp;
	ReadBuffer(&temp, 8);
	return temp;
}

tjs_uint32 tTJSBinaryStream::ReadI32LE()
{
	tjs_uint32 temp;
	ReadBuffer(&temp, 4);
	return temp;
}

tjs_uint16 tTJSBinaryStream::ReadI16LE()
{
	tjs_uint16 temp;
	ReadBuffer(&temp, 2);
	return temp;
}


class tTVPLocalFileStream : public tTJSBinaryStream
{
private:
	HANDLE Handle;

public:
	tTVPLocalFileStream(LPCWSTR localname, tjs_uint32 flag);
	~tTVPLocalFileStream();

	tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence);

	tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size);
	tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint write_size);

	void TJS_INTF_METHOD SetEndOfStorage();

	tjs_uint64 TJS_INTF_METHOD GetSize();

	HANDLE GetHandle() const { return Handle; }
};

void TJS_INTF_METHOD tTVPLocalFileStream::SetEndOfStorage()
{
	SetEndOfFile(Handle);
}

tjs_uint64 TJS_INTF_METHOD tTVPLocalFileStream::GetSize()
{
	tjs_uint64 ret;
	*(DWORD*)&ret = GetFileSize(Handle, ((DWORD*)&ret + 1));
	return ret;
}

tjs_uint TJS_INTF_METHOD tTVPLocalFileStream::Write(const void *buffer, tjs_uint write_size)
{
	DWORD ret = 0;
	WriteFile(Handle, buffer, write_size, &ret, NULL);
	return ret;
}

tjs_uint TJS_INTF_METHOD tTVPLocalFileStream::Read(void *buffer, tjs_uint read_size)
{
	DWORD ret = 0;
	ReadFile(Handle, buffer, read_size, &ret, NULL);
	return ret;
}

tjs_uint64 TJS_INTF_METHOD tTVPLocalFileStream::Seek(tjs_int64 offset, tjs_int whence)
{
	DWORD dwmm;
	switch (whence)
	{
	case TJS_BS_SEEK_SET:	dwmm = FILE_BEGIN;		break;
	case TJS_BS_SEEK_CUR:	dwmm = FILE_CURRENT;	break;
	case TJS_BS_SEEK_END:	dwmm = FILE_END;		break;
	default:				dwmm = FILE_BEGIN;		break; // may be enough
	}

	DWORD low;

	low = SetFilePointer(Handle, (LONG)offset, ((LONG*)&offset) + 1, dwmm);

	if (low == 0xffffffff && GetLastError() != NO_ERROR)
	{
		return TJS_UI64_VAL(0xffffffffffffffff);
	}
	tjs_uint64 ret;
	*(DWORD*)&ret = low;
	*((DWORD*)&ret + 1) = *((DWORD*)&offset + 1);

	return ret;
}

tTVPLocalFileStream::~tTVPLocalFileStream()
{
	if (Handle != INVALID_HANDLE_VALUE) CloseHandle(Handle);
}

tTVPLocalFileStream::tTVPLocalFileStream(LPCWSTR localname, tjs_uint32 flag)
{
	Handle = INVALID_HANDLE_VALUE;

	tjs_uint32 access = flag & TJS_BS_ACCESS_MASK;

	DWORD dwcd;
	DWORD rw;
	switch (access)
	{
	case TJS_BS_READ:
		rw = GENERIC_READ;					dwcd = OPEN_EXISTING;		break;
	case TJS_BS_WRITE:
		rw = GENERIC_WRITE;					dwcd = CREATE_ALWAYS;		break;
	case TJS_BS_APPEND:
		rw = GENERIC_WRITE;					dwcd = OPEN_ALWAYS;			break;
	case TJS_BS_UPDATE:
		rw = GENERIC_WRITE | GENERIC_READ;	dwcd = OPEN_EXISTING;		break;
	}
	Handle = CreateFile(
		localname,
		rw,
		FILE_SHARE_READ, // read shared accesss is strongly needed
		NULL,
		dwcd,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (Handle == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, L"CreateFile Failed!", L"Info", 0);
	}

	if (access == TJS_BS_APPEND) // move the file pointer to last
		SetFilePointer(Handle, 0, NULL, FILE_END);
}


tTJSBinaryStream* ExecOldCreateStream(tjsStr* name, tjs_uint32 flags)
{
	__asm
	{
		push flags
		push name
		call dword ptr ds : [0x0114e656]
	}
}

tTJSBinaryStream* FakeTVPCreateStream(tjsStr* name, tjs_uint32 flags)
{
	if (wcscmp(name->ShortStr,L"/startup.tjs"))
	{
		tTVPLocalFileStream* tmpTVPFSS = new tTVPLocalFileStream(L"startup.tjs", flags);
		return tmpTVPFSS;
	}
	else
	{
		return ExecOldCreateStream(name, flags);
	}
}



void hook()
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

void sethookbytes(uint addr)
{
	hookAddr = addr;
	new_opcode[0] = (char)0xE8;
	(uint&)new_opcode[1] = (uint)FakeTVPCreateStream - addr - 5;
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
		hookAddr = 0x0114e656;
		sethookbytes(hookAddr);
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