#include <windows.h>
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
extern "C" void __stdcall XP3ArchiveAttractFilter_v2(
	unsigned __int32 hash,
	unsigned __int64 offset, void * buffer, long bufferlen)
{
	unsigned __int32 key = hash>>0xc;
	if (bufferlen>4)
	{
		for (int index = 5; index < bufferlen; index++)
		{
			((unsigned char*)buffer)[index] ^= key;
		}
	}
	

}