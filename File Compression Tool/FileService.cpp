#include"FileService.h"
#include <mutex>

//内存分配粒度
SYSTEM_INFO system_info;
void InitializeSystemInfo() {
	GetSystemInfo(&system_info);
}
// 在程序开始时初始化 system_info
struct SystemInfoInitializer {
	SystemInfoInitializer() {
		InitializeSystemInfo();
	}
} systemInfoInitializer;
DWORD MapFileInfo::allocationGranularity = system_info.dwAllocationGranularity;

void FileService::ErrorMessageBox(const HWND& hwnd, const TCHAR* msg, bool showErrorCode)
{
	//创建字符串用于接收错误代码
	TCHAR errorCode[20] = _T("");
	if (showErrorCode) _stprintf_s(errorCode, _T("\n错误代码:%lu"), GetLastError());
	//将msg内容与错误代码拼接起来
	size_t length = _tcslen(msg) + _tcslen(errorCode) + 1;
	TCHAR* finalMsg = new TCHAR[length];
	_stprintf_s(finalMsg, length, _T("%s%s"), msg, errorCode);
	//弹出错误弹窗
	MessageBox(hwnd, finalMsg, _T("错误信息"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
	//销毁使用new开辟的空间
	delete[]finalMsg;
	//退出程序
	exit(GetLastError());
}

void FileService::MapFile(MapFileInfo& mapFileInfo,bool readOnly)
{
	if ((mapFileInfo.fileOffset + mapFileInfo.fileMapSize) > file_size(mapFileInfo.fileName))
	{
		ErrorMessageBox(NULL, _T("映射区域不在文件内"));
	}
	static std::mutex fileMapMutex;
	//使用内存映射方式读取文件，提高文件读取速度
	//打开文件以获取句柄
	//根据是否只读映射文件
	DWORD dwDesiredAccess, dwShareMode, flProtect, dwDesiredAccess_MapViewOfFile;
	if (readOnly) {
		dwDesiredAccess = GENERIC_READ;
		dwShareMode = FILE_SHARE_READ;
		flProtect = PAGE_READONLY;
		dwDesiredAccess_MapViewOfFile = FILE_MAP_READ;
	}
	else {
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		flProtect = PAGE_READWRITE;
		dwDesiredAccess_MapViewOfFile = FILE_MAP_WRITE;
	}
	mapFileInfo.fileHandle = CreateFile(mapFileInfo.fileName, dwDesiredAccess, dwShareMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mapFileInfo.fileHandle == INVALID_HANDLE_VALUE) {
		ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//创建文件映射对象获取句柄
	mapFileInfo.fileMapHandle = CreateFileMapping(mapFileInfo.fileHandle, nullptr, flProtect, 0, 0, nullptr);
	if (mapFileInfo.fileMapHandle == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("无法创建文件映射对象"));
	}
	
	//对偏移量和映射大小进行修正
	size_t realOffset = mapFileInfo.fileOffset - mapFileInfo.offsetCorrection;
	size_t realMapSize = mapFileInfo.fileMapSize + mapFileInfo.offsetCorrection;

	//映射文件视图获取文件映射指针
	mapFileInfo.mapViewPointer = MapViewOfFile(mapFileInfo.fileMapHandle, dwDesiredAccess_MapViewOfFile, (realOffset >> 32) & 0xffffffff, realOffset & 0xffffffff, realMapSize);
	if (mapFileInfo.mapViewPointer == NULL) {
		CloseHandle(mapFileInfo.fileMapHandle);
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("无法映射文件视图"));
	}

	//修正文件指针
	BYTE* tempPointer = (BYTE *) mapFileInfo.mapViewPointer;
	tempPointer += mapFileInfo.offsetCorrection;
	mapFileInfo.mapViewPointer = tempPointer;

	if (mapFileInfo.fileMapSize == 0) {
		mapFileInfo.fileMapSize = file_size(mapFileInfo.fileName) - mapFileInfo.fileOffset;
	}
}

void FileService::ExtendFile(const path& extendFile,size_t extendSize, bool needCreatFile)
{
	if (!needCreatFile && !exists(extendFile))ErrorMessageBox(NULL, _T("文件不存在,无法扩展文件"));
	if (needCreatFile || extendSize > file_size(extendFile)) {
		//打开文件以获取句柄
		HANDLE fileHandle = CreateFile(extendFile.c_str(), GENERIC_READ |GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE) {
			ErrorMessageBox(NULL, _T("无法打开文件"));
		}
		//设置文件指针
		LARGE_INTEGER tempL_I;
		tempL_I.QuadPart = extendSize;//偏移量
		if (!SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_BEGIN))ErrorMessageBox(NULL, _T("设置文件指针失败"));
		if (!SetEndOfFile(fileHandle))ErrorMessageBox(NULL, _T("更新文件大小失败"));
		CloseHandle(fileHandle);
	}
}

uintmax_t FileService::GetFileSize(const path& fileName)
{
	if (!exists(fileName))ErrorMessageBox(NULL, _T("文件(夹)不存在,无法获取大小"));
	//获取普通文件大小
	if (is_regular_file(fileName))return file_size(fileName);
	//获取文件夹大小
	uintmax_t size = 0;
    for (const auto& entry : filesystem::recursive_directory_iterator(fileName))
    {
        if (entry.is_regular_file())
        {
            size += entry.file_size();
        }
    }
	return size;
}

void FileService::ZipFile(const SelectedFileInfo& sourceFile,ZipFileInfo& targetFile, size_t dataBlockIndex,size_t sourceFileOffset, size_t sourceFileMapSize, size_t targetFileOffset, size_t targetFileMapSize)
{
	//映射将要读取的源文件
	MapFileInfo* mapSourceFileInfo = new MapFileInfo((LPTSTR)sourceFile.filePath.c_str(), sourceFileOffset, sourceFileMapSize);
	//进行文件映射
	FileService::MapFile(*mapSourceFileInfo,true);
	//获取文件映射指针
	BYTE* sourceFilePointer = (BYTE*)mapSourceFileInfo->mapViewPointer;
	//映射将要写入的目的文件
	MapFileInfo* mapTargetFileInfo = new MapFileInfo((LPTSTR)targetFile.tempZipFilePath.c_str(), targetFileOffset, targetFileMapSize);
	//进行文件映射
	FileService::MapFile(*mapTargetFileInfo);
	//获取文件映射指针
	BYTE* targetFilePointer = (BYTE*)mapTargetFileInfo->mapViewPointer;

	BYTE buffer = 0;
	BYTE bitCount = 0;
	
	size_t writer = 0;
	size_t readerLimit = mapSourceFileInfo->fileMapSize;
	if (sourceFileMapSize) readerLimit -= 7;
	BYTE abandonBits = 0;
	for (size_t reader = 0; reader < readerLimit; ++reader,++sourceFilePointer) {
		//读取当前源文件指针所指向字节，并获得对应的编码
		const std::string& code = targetFile.symbolCode.at(*sourceFilePointer);
		for (char bit : code) {
			if (dataBlockIndex && abandonBits != sourceFile.dataBlockWPL_Size[dataBlockIndex - 1].second) {
				++abandonBits;
				continue;
			}
			buffer = (buffer << 1) | (bit - '0');
			++bitCount;
			if (bitCount == 8) {
				*targetFilePointer = buffer;
				buffer = 0;
				bitCount = 0;
				++targetFilePointer;
				++writer;
			}
		}
	}
	//写入剩余的比特位
	if (bitCount > 0) {
		if (sourceFileMapSize) {
			while (bitCount < 8) {
				const std::string& code = targetFile.symbolCode.at(*sourceFilePointer);
				for (char bit : code) {
					buffer = (buffer << 1) | (bit - '0');
					++bitCount;
					if (bitCount == 8) {
						*targetFilePointer = buffer;
						buffer = 0;
						bitCount = 0;
						++writer;
						break;
					}
				}
				if (bitCount == 0)break;
			}
		}
		else {
			buffer <<= (8 - bitCount);
			*targetFilePointer = buffer;
			++writer;
		}
	}
	if (targetFileMapSize != writer) {
		ErrorMessageBox(NULL, _T("实际写入数据数与计算值不匹配"));
	}
	delete mapSourceFileInfo;
	delete mapTargetFileInfo;
}

unsigned short FileService::WriteZipFileHeader(ZipFileInfo& zipFile)
{
	zipFile.tempZipFilePath.assign(zipFile.zipFilePath);
	while (exists(zipFile.tempZipFilePath)) {
		zipFile.tempZipFilePath += ".temp";
	}
	//创建并打开压缩包文件
	HANDLE fileHandle = CreateFile(zipFile.tempZipFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("无法创建压缩包文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_BEGIN);
	DWORD written;
	//标识签名
	char identification[2] = { 'y','a' };
	WriteFile(fileHandle, identification, 2, &written, nullptr);
	//首部大小
	unsigned short headerSize = 4 + 2 * zipFile.codeLength.size();
	WriteFile(fileHandle, &headerSize, 2, &written, nullptr);
	//符号-编码长度表
	for (size_t i = 0; i < zipFile.codeLength.size(); ++i)
	{
		WriteFile(fileHandle, &zipFile.codeLength[i], 2, &written, nullptr);
	}
	CloseHandle(fileHandle);
	return headerSize;
}

unsigned short FileService::WriteFileHeader(const SelectedFileInfo& sourceFile, const ZipFileInfo& targetFile)
{
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(targetFile.tempZipFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = targetFile.newFileSize;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_BEGIN);
	DWORD written;
	//首部大小
	unsigned short headerSize = 19 + sourceFile.fileName.wstring().size() * 2;
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//原始文件大小
	uintmax_t oldSize = sourceFile.oldFileSize;
	WriteFile(fileHandle, &oldSize, sizeof(oldSize), &written, nullptr);
	//压缩后大小
	WriteFile(fileHandle, &sourceFile.WPL_Size.first, sizeof(sourceFile.WPL_Size.first), &written, nullptr);
	//填充比特数
	WriteFile(fileHandle, &sourceFile.WPL_Size.second, sizeof(sourceFile.WPL_Size.second), &written, nullptr);
	//文件名
	WriteFile(fileHandle, sourceFile.fileName.c_str(), headerSize - 19, &written, nullptr);

	CloseHandle(fileHandle);
	return headerSize;
}

