#pragma once
#include<windows.h>
#include<tchar.h>
#include <filesystem>
#include <unordered_map>

using namespace std;
using namespace filesystem;

struct MapFileInfo
{
	//文件名
	LPTSTR fileName = nullptr;
	//打开的文件句柄
	HANDLE fileHandle = nullptr;
	//文件映射对象句柄
	HANDLE fileMapHandle = nullptr;
	//文件映射指针
	LPVOID mapViewPointer = nullptr;
	//文件映射的起始位置
	size_t fileOffset;
	//文件映射大小
	size_t fileMapSize;
	MapFileInfo(LPTSTR name,size_t offset = 0,size_t size = 0):fileName(name), fileOffset(offset), fileMapSize(size) {}
	~MapFileInfo()
	{	
		//释放资源
		if (mapViewPointer)UnmapViewOfFile(mapViewPointer);
		if (fileMapHandle)CloseHandle(fileMapHandle);
		if (fileHandle)CloseHandle(fileHandle);
	}
};

class WinFileProc
{
public:
	//错误信息弹窗
	static void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);
	//使用内存映射读取文件
	static void MapFileReader(MapFileInfo & mapFileInfo);
	//使用递归获取文件夹大小
    static uintmax_t GetFileSize(const path& fileName);
	//将源文件某区域中的数据依照 符号-编码表 写入目的文件
	static void ZipFile(const path& sourceFile, const  path& destFile, const unordered_map<BYTE, string>& symbolCode, size_t fileOffset = 0, size_t fileMapSize = 0);
	//写入压缩文件首部
	static void WriteZipFileHeader(const path& destFile, const vector<pair<BYTE, BYTE>>& codeLength);
	//写入文件(夹)首部
	static void WriteFileHeader(const path& destFile, const path& sourceFile, const pair<uintmax_t, BYTE>& WPL_Size);
};