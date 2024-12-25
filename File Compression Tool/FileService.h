#pragma once
#include<windows.h>
#include<tchar.h>
#include <filesystem>
#include "ZipFileInfo.h"
#include "SelectedFileInfo.h"

using namespace std;
using namespace filesystem;

struct MapFileInfo
{
	//内存分配粒度
	static DWORD allocationGranularity;
	//文件名
	LPTSTR fileName = nullptr;
	//打开的文件句柄
	HANDLE fileHandle = nullptr;
	//文件映射对象句柄
	HANDLE fileMapHandle = nullptr;
	//文件映射指针
	LPVOID mapViewPointer = nullptr;
	//文件映射的起始位置(偏移量)
	size_t fileOffset;
	//偏移以及映射大小修正量
	size_t offsetCorrection;
	//文件映射大小
	size_t fileMapSize;
	MapFileInfo(LPTSTR name,size_t offset = 0,size_t size = 0):fileName(name), fileOffset(offset), fileMapSize(size) {
		offsetCorrection = fileOffset % allocationGranularity;
	}
	~MapFileInfo()
	{	
		//释放资源
		if (mapViewPointer)UnmapViewOfFile(mapViewPointer);
		if (fileMapHandle)CloseHandle(fileMapHandle);
		if (fileHandle)CloseHandle(fileHandle);
	}
};

class FileService
{
public:
	//错误信息弹窗
	static void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);
	//使用内存映射文件
	static void MapFile(MapFileInfo & mapFileInfo,bool needAutoExtendFile = false);
	//使用递归获取文件夹大小
    static uintmax_t GetFileSize(const path& fileName);
	//将源文件某区域中的数据依照 符号-编码表 写入目的文件
	static void ZipFile(const SelectedFileInfo& sourceFile,ZipFileInfo& targetFile, size_t sourceFileOffset = 0, size_t sourceFileMapSize = 0, size_t targetFileOffset = 0, size_t targetFileMapSize = 0);
	//写入压缩文件首部
	static unsigned short WriteZipFileHeader(ZipFileInfo& zipFile);
	//写入文件首部
	static unsigned short WriteFileHeader(const path& targetFile, const SelectedFileInfo& sourceFile);
};