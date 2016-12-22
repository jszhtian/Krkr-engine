// SLLoader.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址  

//#define MAX_PATH  256
using namespace std;
ofstream flog("Report.log");

typedef struct Fileinfo
{
	string filename;
	string hash;
}Fileinfo;
int flag = 0;
vector <Fileinfo> CheckInfo;
char* wtoc(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_ACP, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char *out = new char[dwMinSize];
	WideCharToMultiByte(CP_OEMCP, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

string GetMd5(LPCWSTR FileDirectory)
{
	string hash;
	HANDLE hFile = CreateFile(FileDirectory, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)                                        //如果CreateFile调用失败  
	{
		flog << "CreateFile go wrong :" << GetLastError() << endl;                //提示CreateFile调用失败，并输出错误号。visual studio中可在“工具”>“错误查找”中利用错误号得到错误信息。  
		CloseHandle(hFile);
	}
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //获得CSP中一个密钥容器的句柄  
	{
		flog << "CryptAcquireContext go wrong:" << GetLastError() << endl;
	}
	HCRYPTPROV hHash = NULL;
	if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)     //初始化对数据流的hash，创建并返回一个与CSP的hash对象相关的句柄。这个句柄接下来将被CryptHashData调用。  
	{
		flog << "CryptCreateHash go wrong:" << GetLastError() << endl;
	}
	DWORD dwFileSize = GetFileSize(hFile, 0);    //获取文件的大小  
	if (dwFileSize == 0xFFFFFFFF)               //如果获取文件大小失败  
	{
		flog << "GetFileSize go wrong:" << GetLastError() << endl;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //读取文件  
	{
		flog << "ReadFile go wrong:" << GetLastError() << endl;
	}
	if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash文件  
	{
		flog << "CryptHashData go wrong:" << GetLastError() << endl;
	}

	delete[] lpReadFileBuffer;
	CloseHandle(hFile);          //关闭文件句柄  
	BYTE *pbHash;
	DWORD dwHashLen = sizeof(DWORD);
	//以下注释掉的代码不用使用，因为已经知道md5值就占32个字节，没有必要通过CryptGetHashParam函数来得到字节数。  
	/*
	BYTE *pbHashSize;
	if (!(pbHashSize=(byte*)malloc(dwHashLen)))      //为pbHashSize分配内存
	{
	cout<<"memory allocation failed:"<<GetLastError()<<endl;
	}
	//将第二个参数的值设为HP_HASHSIZE。dwHashLen中存放着hash值的字节数。这个调用必须在将第三个参数设置为HP_HASHVAL的调用前，这样才能分配正确数量的内存。
	if (CryptGetHashParam(hHash,HP_HASHSIZE,pbHashSize,&dwHashLen,0))
	{
	free(pbHashSize);
	}
	else
	{
	cout<<"get size go wrong"<<GetLastError()<<endl;
	}*/
	if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))      //我也不知道为什么要先这样调用CryptGetHashParam，这块是参照的msdn         
	{
	}
	else
	{
		flog << "get length wrong" << endl;
	}
	if (pbHash = (byte*)malloc(dwHashLen))
	{
	}
	else
	{
		flog << "allocation failed" << endl;
	}
	if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))            //获得md5值  
	{
		/*for (DWORD i = 0; i<dwHashLen; i++)         //输出md5值
		{
		printf("%02x", pbHash[i]);
		}
		cout << endl;*/
		char tmp[5];
		for (DWORD i = 0; i < dwHashLen; i++)         //输出md5值  
		{
			sprintf(tmp, "%02x", pbHash[i]);
			hash += tmp;
		}
		//cout << endl << hash << endl;
	}
	//善后工作  
	if (CryptDestroyHash(hHash) == FALSE)          //销毁hash对象  
	{
		flog << "CryptDestroyHash go wrong:" << GetLastError() << endl;
	}
	if (CryptReleaseContext(hProv, 0) == FALSE)
	{
		flog << "CryptReleaseContext go wrong:" << GetLastError() << endl;
	}
	return hash;
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

BOOL FolderExists(LPCTSTR lpPath)
{
	return FindFirstFileExists(lpPath, FILE_ATTRIBUTE_DIRECTORY);
}


int main()
{
	flog << "Load MD5 List!" << endl;
	fstream TFile2("Md5list.dat", ios::in | ios::out);
	if (TFile2.fail())
	{
		flog << "XP3 ArchiveFiles Check Fail!" << endl;
		flog << "MD5 File Missing!" << endl;
		MessageBoxW(0, L"MD5 File Missing!", L"Warning", 0);
		flag = 1;
	}
	TFile2.seekg(0x0, ios::beg);
	char magic[128];
	string magproc;
	TFile2.getline(magic, 128);
	magproc = magic;
	if (magproc.find("MD5",3) == magproc.npos)
	{
		flog << "XP3 ArchiveFiles Check Fail!" << endl;
		flog << "Invalid MD5 file!" << endl;
		MessageBoxW(0, L"Invalid MD5 file!", L"Warning", 0);
		flag = 1;
		return -1;
	}
	char buf[128];
	string proc;
	while (!TFile2.eof())
	{
		Fileinfo tmpblock;
		TFile2.getline(buf, 128);
		proc = buf;
		if (proc.find("END", 3) != proc.npos)
		{
			break;
		}
/*		if (proc.find("xp3", 3) == proc.npos)
		{
			flog << "XP3 ArchiveFiles Check Fail!" << endl;
			flog << "Invalid MD5 file!" << endl;
			flog << "Please check files!" << endl;
			flag = 1;
			break;
		}
*/
		tmpblock.filename = proc;
		TFile2.getline(buf, 128);
		proc = buf;
		tmpblock.hash = proc;
		CheckInfo.push_back(tmpblock);
	}
	flog << "Load MD5 List Finish!" << endl;
	flog << "File Check Processing!!!" << endl;
	double start = clock();
	for (vector<Fileinfo>::iterator iter = CheckInfo.begin(); iter != CheckInfo.end(); iter++)
	{
		/*cout << "File Name:" << iter->filename << endl;
		cout << "File Hash:" << iter->hash << endl;*/
		CString tmp = CString(iter->filename.c_str());
		string out;
		USES_CONVERSION;
		LPCWSTR wszTmp = A2CW(W2A(tmp));
		try
		{
			out = GetMd5(wszTmp);
		}
		catch (const std::exception& err)
		{
			flog << "An Unexpected Error occurred: " << err.what() << endl;
			flog << "Processing File: " << iter->filename << endl;
			MessageBoxW(0, L"An Unexpected Error occurred!", L"Warning", 0);
			flag = 1;
			break;
		}
		
		if (out!=iter->hash)
		{
			flog << "XP3 ArchiveFiles Check Fail!" << endl;
			flog << "XP3 file:" <<iter->filename<<"Check Fail!"<< endl;
			MessageBoxW(0, L"XP3 ArchiveFiles Check Fail!", L"Warning", 0);
			flag = 1;
			break;
		}
	}
	double finish = clock();
	double duration = (double)(finish - start) / CLOCKS_PER_SEC;
	flog << "Time to process file check is " << duration << "seconds"<<endl;
	if (flag==1)
	{
		flog << "Please check files!" << endl;
		MessageBoxW(0, L"Please check files!", L"Warning", 0);
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	wchar_t* DirPath = new wchar_t[MAX_PATH];
	char* DLLPath = new char[MAX_PATH]; //testdll.dll
	//wchar_t* DetourPath = new wchar_t[MAX_PATH]; //detoured.dll
	wchar_t* ExePath = new wchar_t[MAX_PATH];//exefile
	GetCurrentDirectory(MAX_PATH, DirPath);
	CString Dir=DirPath;
	CString DLL= DirPath;
	DLL.Insert(DLL.GetLength(), L"\\KRPatch.dll");
	//CString Detour= DirPath;
	//Detour.Insert(Detour.GetLength(), L"detoured.dll");
	CString Exe = DirPath;
	Exe.Insert(Exe.GetLength(), L"\\ＰＲＥＴＴＹ×Ｃ∧ＴＩＯＮ.exe");
		//DetourPath = Detour.AllocSysString();
	ExePath = Exe.AllocSysString();
	DLLPath = wtoc(DLL.AllocSysString());
	bool check1= FilePathExists(Exe);
	bool check2= FilePathExists(DLL);
	flog << "Target EXE:" << wtoc(Exe.AllocSysString()) << std::endl << "Target DLL:" << wtoc(DLL.AllocSysString()) << std::endl;
	if (check1==false||check2==false)
	{
		flog << "Execute files can not found!" << endl;
		MessageBoxW(0, L"Execute files missing!", L"Warning", 0);
		return -1;
	}
	
	//system("pause");
	DetourCreateProcessWithDll(ExePath,NULL,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi,DLLPath,0);
	return 0;
}

