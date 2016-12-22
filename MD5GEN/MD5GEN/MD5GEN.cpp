// MD5GEN.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
using namespace std;

string GetMd5(LPCWSTR FileDirectory)
{
	string hash;
	HANDLE hFile = CreateFile(FileDirectory, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)                                        //如果CreateFile调用失败  
	{
		cout << "CreateFile go wrong :" << GetLastError() << endl;                //提示CreateFile调用失败，并输出错误号。visual studio中可在“工具”>“错误查找”中利用错误号得到错误信息。  
		CloseHandle(hFile);
	}
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //获得CSP中一个密钥容器的句柄  
	{
		cout << "CryptAcquireContext go wrong:" << GetLastError() << endl;
	}
	HCRYPTPROV hHash = NULL;
	if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)     //初始化对数据流的hash，创建并返回一个与CSP的hash对象相关的句柄。这个句柄接下来将被CryptHashData调用。  
	{
		cout << "CryptCreateHash go wrong:" << GetLastError() << endl;
	}
	DWORD dwFileSize = GetFileSize(hFile, 0);    //获取文件的大小  
	if (dwFileSize == 0xFFFFFFFF)               //如果获取文件大小失败  
	{
		cout << "GetFileSize go wrong:" << GetLastError() << endl;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //读取文件  
	{
		cout << "ReadFile go wrong:" << GetLastError() << endl;
	}
	if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash文件  
	{
		cout << "CryptHashData go wrong:" << GetLastError() << endl;
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
		cout << "get length wrong" << endl;
	}
	if (pbHash = (byte*)malloc(dwHashLen))
	{
	}
	else
	{
		cout << "allocation failed" << endl;
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
			sprintf(tmp,"%02x", pbHash[i]);
			hash += tmp;
		}
		//cout << endl << hash << endl;
	}
	//善后工作  
	if (CryptDestroyHash(hHash) == FALSE)          //销毁hash对象  
	{
		cout << "CryptDestroyHash go wrong:" << GetLastError() << endl;
	}
	if (CryptReleaseContext(hProv, 0) == FALSE)
	{
		cout << "CryptReleaseContext go wrong:" << GetLastError() << endl;
	}
	return hash;
}

vector<string> Getfilelist(string dir)
{
	vector<string> Filelist;
	_finddata_t file;
	long lf;
	if ((lf = _findfirst(dir.c_str(), &file)) == -1) {
		cout << dir << " not found!!!" << endl;
	}
	else {
		while (_findnext(lf, &file) == 0) {
			//输出文件名  
			//cout<<file.name<<endl;  
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
				continue;
			Filelist.push_back(file.name);
		}
	}
	_findclose(lf);
	sort(Filelist.begin(), Filelist.end());
	return Filelist;
}

typedef struct Fileinfo
{
	string filename;
	string hash;
}Fileinfo;

int main()
{
	char current_address[100];
	memset(current_address, 0, 100);
	getcwd(current_address, 100); //获取当前路径  
	cout << current_address << endl;
	strcat(current_address, "\\*");
	vector<Fileinfo> md5list;
	vector<string> files = Getfilelist((string)current_address);
	for (int i = 0; i < files.size(); i++)
	{
		if (1/*files[i].find("xp3",3)!=files[i].npos*/)
		{
			//cout << files[i] << endl;
			CString tmp = CString(files[i].c_str());
			string out;
			USES_CONVERSION;
			LPCWSTR wszTmp = A2CW(W2A(tmp));
			out = GetMd5(wszTmp);
			//cout << out << endl;
			Fileinfo tmpstruc;
			tmpstruc.filename = files[i];
			tmpstruc.hash = out;
			md5list.push_back(tmpstruc);
		}
	}
	ofstream fout("Md5list.dat");
	fout << "---MD5LIST BEGIN---" << endl;
	for (vector<Fileinfo>::iterator iter = md5list.begin(); iter != md5list.end(); iter++)
	{
		cout << "File Name:" << iter->filename << endl;
		fout << iter->filename << endl;
		cout << "File Hash:" << iter->hash<< endl;
		fout << iter->hash << endl;
	}
	fout << "---MD5LIST END---" << endl;

    return 0;
}

