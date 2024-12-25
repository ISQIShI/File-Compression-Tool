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

void FileService::MapFile(MapFileInfo& mapFileInfo, bool needAutoExtendFile)
{
	static std::mutex fileMapMutex;
	//使用内存映射方式读取文件，提高文件读取速度
	//打开文件以获取句柄
	mapFileInfo.fileHandle = CreateFile(mapFileInfo.fileName, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mapFileInfo.fileHandle == INVALID_HANDLE_VALUE)	ErrorMessageBox(NULL,_T("无法打开文件"));
	size_t aaa = file_size(mapFileInfo.fileName);
	//当需要自动扩展文件且要映射的区域超过文件大小，自动扩展
	if (needAutoExtendFile && ((mapFileInfo.fileOffset + mapFileInfo.fileMapSize) > file_size(mapFileInfo.fileName))) {
		//设置文件指针
		LARGE_INTEGER tempL_I;
		tempL_I.QuadPart = mapFileInfo.fileOffset + mapFileInfo.fileMapSize;//偏移量
		//上锁，保护文件指针线程安全
		lock_guard<std::mutex> lock(fileMapMutex);
		//二次判断,防止其他线程已经修改文件大小到满足当前线程需求
		CloseHandle(mapFileInfo.fileHandle);
		mapFileInfo.fileHandle = CreateFile(mapFileInfo.fileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (mapFileInfo.fileHandle == INVALID_HANDLE_VALUE)ErrorMessageBox(NULL, _T("重新打开文件失败"));
		if (tempL_I.QuadPart > file_size(mapFileInfo.fileName)) {
			if(!SetFilePointerEx(mapFileInfo.fileHandle, tempL_I, nullptr, FILE_BEGIN))ErrorMessageBox(NULL, _T("设置文件指针失败"));
			if(!SetEndOfFile(mapFileInfo.fileHandle))ErrorMessageBox(NULL, _T("更新文件大小失败"));
		}
	}
	//创建文件映射对象获取句柄
	mapFileInfo.fileMapHandle = CreateFileMapping(mapFileInfo.fileHandle, nullptr, PAGE_READWRITE, 0, 0, nullptr);
	if (mapFileInfo.fileMapHandle == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("无法创建文件映射对象"));
	}
	
	//映射文件视图获取文件映射指针
	mapFileInfo.mapViewPointer = MapViewOfFile(mapFileInfo.fileMapHandle, FILE_MAP_WRITE, (mapFileInfo.fileOffset >> 32) & 0xffffffff, mapFileInfo.fileOffset & 0xffffffff, mapFileInfo.fileMapSize);
	if (mapFileInfo.mapViewPointer == NULL) {
		CloseHandle(mapFileInfo.fileMapHandle);
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("无法映射文件视图"));
	}
	if (mapFileInfo.fileMapSize == 0) {
		mapFileInfo.fileMapSize = file_size(mapFileInfo.fileName) - mapFileInfo.fileOffset;
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

void FileService::ZipFile(const SelectedFileInfo& sourceFile,ZipFileInfo& targetFile, size_t sourceFileOffset, size_t sourceFileMapSize, size_t targetFileOffset, size_t targetFileMapSize)
{
	//映射将要读取的源文件
	MapFileInfo* mapSourceFileInfo = new MapFileInfo((LPTSTR)sourceFile.filePath.c_str(), sourceFileOffset, sourceFileMapSize);
	//进行文件映射
	FileService::MapFile(*mapSourceFileInfo);
	//获取文件映射指针
	BYTE* sourceFilePointer = (BYTE*)mapSourceFileInfo->mapViewPointer;
	//映射将要写入的目的文件
	MapFileInfo* mapTargetFileInfo = new MapFileInfo((LPTSTR)targetFile.tempZipFilePath.c_str(), targetFileOffset, targetFileMapSize);
	//进行文件映射
	FileService::MapFile(*mapTargetFileInfo,true);
	//获取文件映射指针
	BYTE* targetFilePointer = (BYTE*)mapTargetFileInfo->mapViewPointer;

	BYTE buffer = 0;
	BYTE bitCount = 0;
	for (size_t reader = 0; reader < mapSourceFileInfo->fileMapSize; ++reader,++sourceFilePointer) {
		//读取当前源文件指针所指向字节，并获得对应的编码
		const std::string& code = targetFile.symbolCode.at(*sourceFilePointer);
		for (char bit : code) {
			buffer = (buffer << 1) | (bit - '0');
			++bitCount;
			if (bitCount == 8) {
				*targetFilePointer = buffer;
				buffer = 0;
				bitCount = 0;
				++targetFilePointer;
			}
		}
	}
	if (bitCount != 8 - sourceFile.WPL_Size.second)ErrorMessageBox(NULL, _T("实际写入数据数与计算值不匹配"));
	//写入剩余的比特位
	if (bitCount > 0) {
		buffer <<= (8 - bitCount);
		*targetFilePointer = buffer;
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
	HANDLE fileHandle = CreateFile(zipFile.tempZipFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
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
	WriteFile(fileHandle, identification, sizeof(identification), &written, nullptr);
	//首部大小
	unsigned short headerSize = 4 + sizeof(zipFile.codeLength[0]) * zipFile.codeLength.size();
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//符号-编码长度表
	for (size_t i = 0; i < zipFile.codeLength.size(); ++i)
	{
		WriteFile(fileHandle, &zipFile.codeLength[i], sizeof(zipFile.codeLength[i]), &written, nullptr);
	}
	CloseHandle(fileHandle);
	return headerSize;
}

unsigned short FileService::WriteFileHeader(const path& targetFile, const SelectedFileInfo& sourceFile)
{
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(targetFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	DWORD written;
	//首部大小
	unsigned short headerSize = 19 + sourceFile.fileName.wstring().size() * 2;
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//填充比特数
	BYTE fileType_bitsCount = sourceFile.WPL_Size.second;
	WriteFile(fileHandle, &fileType_bitsCount, sizeof(fileType_bitsCount), &written, nullptr);
	//原始文件大小
	uintmax_t oldSize = sourceFile.oldFileSize;
	WriteFile(fileHandle, &oldSize, sizeof(oldSize), &written, nullptr);
	//数据块大小
	WriteFile(fileHandle, &sourceFile.WPL_Size.first, sizeof(sourceFile.WPL_Size.first), &written, nullptr);
	//文件名
	WriteFile(fileHandle, sourceFile.fileName.c_str(), headerSize - 19, &written, nullptr);

	CloseHandle(fileHandle);
	return headerSize;
}

