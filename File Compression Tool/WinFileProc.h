#pragma once
#include<windows.h>
#include<tchar.h>
#include <filesystem>
#include <unordered_map>

using namespace std;
using namespace filesystem;

struct MapFileInfo
{
	//�ļ���
	LPTSTR fileName = nullptr;
	//�򿪵��ļ����
	HANDLE fileHandle = nullptr;
	//�ļ�ӳ�������
	HANDLE fileMapHandle = nullptr;
	//�ļ�ӳ��ָ��
	LPVOID mapViewPointer = nullptr;
	//�ļ�ӳ�����ʼλ��
	size_t fileOffset;
	//�ļ�ӳ���С
	size_t fileMapSize;
	MapFileInfo(LPTSTR name,size_t offset = 0,size_t size = 0):fileName(name), fileOffset(offset), fileMapSize(size) {}
	~MapFileInfo()
	{	
		//�ͷ���Դ
		if (mapViewPointer)UnmapViewOfFile(mapViewPointer);
		if (fileMapHandle)CloseHandle(fileMapHandle);
		if (fileHandle)CloseHandle(fileHandle);
	}
};

class WinFileProc
{
public:
	//������Ϣ����
	static void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);
	//ʹ���ڴ�ӳ���ȡ�ļ�
	static void MapFileReader(MapFileInfo & mapFileInfo);
	//ʹ�õݹ��ȡ�ļ��д�С
    static uintmax_t GetFileSize(const path& fileName);
	//��Դ�ļ�ĳ�����е��������� ����-����� д��Ŀ���ļ�
	static void ZipFile(const path& sourceFile, const  path& destFile, const unordered_map<BYTE, string>& symbolCode, size_t fileOffset = 0, size_t fileMapSize = 0);
	//д��ѹ���ļ��ײ�
	static void WriteZipFileHeader(const path& destFile, const vector<pair<BYTE, BYTE>>& codeLength);
	//д���ļ�(��)�ײ�
	static void WriteFileHeader(const path& destFile, const path& sourceFile, const pair<uintmax_t, BYTE>& WPL_Size);
};