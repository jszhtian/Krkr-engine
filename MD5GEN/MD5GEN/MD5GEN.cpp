// MD5GEN.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
using namespace std;

string GetMd5(LPCWSTR FileDirectory)
{
	string hash;
	HANDLE hFile = CreateFile(FileDirectory, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)                                        //���CreateFile����ʧ��  
	{
		cout << "CreateFile go wrong :" << GetLastError() << endl;                //��ʾCreateFile����ʧ�ܣ����������š�visual studio�п��ڡ����ߡ�>��������ҡ������ô���ŵõ�������Ϣ��  
		CloseHandle(hFile);
	}
	HCRYPTPROV hProv = NULL;
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)       //���CSP��һ����Կ�����ľ��  
	{
		cout << "CryptAcquireContext go wrong:" << GetLastError() << endl;
	}
	HCRYPTPROV hHash = NULL;
	if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash) == FALSE)     //��ʼ������������hash������������һ����CSP��hash������صľ��������������������CryptHashData���á�  
	{
		cout << "CryptCreateHash go wrong:" << GetLastError() << endl;
	}
	DWORD dwFileSize = GetFileSize(hFile, 0);    //��ȡ�ļ��Ĵ�С  
	if (dwFileSize == 0xFFFFFFFF)               //�����ȡ�ļ���Сʧ��  
	{
		cout << "GetFileSize go wrong:" << GetLastError() << endl;
	}
	byte* lpReadFileBuffer = new byte[dwFileSize];
	DWORD lpReadNumberOfBytes;
	if (ReadFile(hFile, lpReadFileBuffer, dwFileSize, &lpReadNumberOfBytes, NULL) == 0)        //��ȡ�ļ�  
	{
		cout << "ReadFile go wrong:" << GetLastError() << endl;
	}
	if (CryptHashData(hHash, lpReadFileBuffer, lpReadNumberOfBytes, 0) == FALSE)      //hash�ļ�  
	{
		cout << "CryptHashData go wrong:" << GetLastError() << endl;
	}

	delete[] lpReadFileBuffer;
	CloseHandle(hFile);          //�ر��ļ����  
	BYTE *pbHash;
	DWORD dwHashLen = sizeof(DWORD);
	//����ע�͵��Ĵ��벻��ʹ�ã���Ϊ�Ѿ�֪��md5ֵ��ռ32���ֽڣ�û�б�Ҫͨ��CryptGetHashParam�������õ��ֽ�����  
	/*
	BYTE *pbHashSize;
	if (!(pbHashSize=(byte*)malloc(dwHashLen)))      //ΪpbHashSize�����ڴ�
	{
	cout<<"memory allocation failed:"<<GetLastError()<<endl;
	}
	//���ڶ���������ֵ��ΪHP_HASHSIZE��dwHashLen�д����hashֵ���ֽ�����������ñ����ڽ���������������ΪHP_HASHVAL�ĵ���ǰ���������ܷ�����ȷ�������ڴ档
	if (CryptGetHashParam(hHash,HP_HASHSIZE,pbHashSize,&dwHashLen,0))
	{
	free(pbHashSize);
	}
	else
	{
	cout<<"get size go wrong"<<GetLastError()<<endl;
	}*/
	if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))      //��Ҳ��֪��ΪʲôҪ����������CryptGetHashParam������ǲ��յ�msdn         
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
	if (CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0))            //���md5ֵ  
	{
		/*for (DWORD i = 0; i<dwHashLen; i++)         //���md5ֵ  
		{
			printf("%02x", pbHash[i]);
		}
		cout << endl;*/
		char tmp[5];
		for (DWORD i = 0; i < dwHashLen; i++)         //���md5ֵ  
		{
			sprintf(tmp,"%02x", pbHash[i]);
			hash += tmp;
		}
		//cout << endl << hash << endl;
	}
	//�ƺ���  
	if (CryptDestroyHash(hHash) == FALSE)          //����hash����  
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
			//����ļ���  
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
	getcwd(current_address, 100); //��ȡ��ǰ·��  
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

