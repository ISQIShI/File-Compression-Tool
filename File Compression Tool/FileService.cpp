#include"FileService.h"

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

void FileService::MapFileReader(MapFileInfo& mapFileInfo)
{
	//使用内存映射方式读取文件，提高文件读取速度
	//打开文件以获取句柄
	mapFileInfo.fileHandle = CreateFile(mapFileInfo.fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mapFileInfo.fileHandle == INVALID_HANDLE_VALUE)ErrorMessageBox(NULL,_T("无法打开文件"));
	//创建文件映射对象获取句柄
	mapFileInfo.fileMapHandle = CreateFileMapping(mapFileInfo.fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (mapFileInfo.fileMapHandle == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("无法创建文件映射对象"));
	}
	//映射文件视图获取文件映射指针
	mapFileInfo.mapViewPointer = MapViewOfFile(mapFileInfo.fileMapHandle, FILE_MAP_READ, (mapFileInfo.fileOffset >> 32) & 0xffffffff, mapFileInfo.fileOffset & 0xffffffff, mapFileInfo.fileMapSize);
	if (mapFileInfo.mapViewPointer == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
        CloseHandle(mapFileInfo.fileMapHandle);
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

void FileService::ZipFile(const path& sourceFile, const path& destFile, const unordered_map<BYTE, string>& symbolCode, size_t fileOffset, size_t fileMapSize)
{
	MapFileInfo* mapFileInfo = new MapFileInfo((LPTSTR)sourceFile.c_str(), fileOffset, fileMapSize);
	//进行文件映射
	FileService::MapFileReader(*mapFileInfo);
	//获取文件指针
	BYTE* filePointer = (BYTE*)mapFileInfo->mapViewPointer;
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		delete mapFileInfo;
		FileService::ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	// 写入缓冲区
	std::vector<BYTE> writeBuffer; // 写入缓冲区
	size_t bufferCapacity = 4096; // 设置缓冲区大小
	writeBuffer.reserve(bufferCapacity);

	UINT currentWord = 0; // 当前 32 位数据块
	int bitCount = 0;     // 当前数据块中已填充的位数

	// 遍历文件映射区域
	for (size_t i = 0; i < mapFileInfo->fileMapSize; ++i) {
		const std::string& code = symbolCode.at(filePointer[i]);

		// 将编码逐位填充到当前数据块
		for (char bit : code) {
			currentWord = (currentWord << 1) | (bit - '0');
			++bitCount;

			// 如果当前数据块已满（32 位），加入缓冲区
			if (bitCount == 32) {
				// 转换为小端字节序并加入缓冲区
				for (int j = 0; j < 4; ++j) {
					writeBuffer.push_back(static_cast<BYTE>((currentWord >> (8 * (3 - j))) & 0xFF));
				}

				currentWord = 0;
				bitCount = 0;

				// 如果缓冲区满了，写入文件
				if (writeBuffer.size() >= bufferCapacity) {
					DWORD written;
					WriteFile(fileHandle, writeBuffer.data(), writeBuffer.size(), &written, nullptr);
					writeBuffer.clear();
				}
			}
		}
	}

	// 处理剩余不足 32 位的数据块
	if (bitCount > 0) {
		currentWord <<= (32 - bitCount); // 左对齐到 32 位
		for (int j = 0; j < 4; ++j) {
			writeBuffer.push_back(static_cast<BYTE>((currentWord >> (8 * (3 - j))) & 0xFF));
		}
	}

	// 写入剩余数据
	if (!writeBuffer.empty()) {
		DWORD written;
		WriteFile(fileHandle, writeBuffer.data(), writeBuffer.size(), &written, nullptr);
	}
	CloseHandle(fileHandle);
	delete mapFileInfo;
}

void FileService::WriteZipFileHeader(const path& destFile, const vector<pair<BYTE, BYTE>>& codeLength)
{
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	DWORD written;
	//标识签名
	char identification[2] = { 'y','a' };
	WriteFile(fileHandle, identification, sizeof(identification), &written, nullptr);
	//首部大小
	unsigned short headerSize = 4 + sizeof(codeLength[0]) * codeLength.size();
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//符号-编码长度表
	for (size_t i = 0; i < codeLength.size(); ++i)
	{
		WriteFile(fileHandle, &codeLength[i], sizeof(codeLength[i]), &written, nullptr);
	}

	CloseHandle(fileHandle);
}

void FileService::WriteFileHeader(const path& destFile, const path& sourceFile, const pair<uintmax_t, BYTE>& WPL_Size)
{
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	DWORD written;
	//首部大小
	unsigned short headerSize = 19 + sourceFile.native().size() * 2;
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//文件类型和填充比特数
	BYTE fileType_bitsCount = ((is_directory(sourceFile) ? 1 : 0) << 7) + WPL_Size.second;
	WriteFile(fileHandle, &fileType_bitsCount, sizeof(fileType_bitsCount), &written, nullptr);
	//原始文件(夹)大小
	uintmax_t oldSize = GetFileSize(sourceFile);
	WriteFile(fileHandle, &oldSize, sizeof(oldSize), &written, nullptr);
	//数据块大小
	WriteFile(fileHandle, &WPL_Size.first, sizeof(WPL_Size.first), &written, nullptr);
	//文件名
	WriteFile(fileHandle, sourceFile.c_str(), sourceFile.native().size() * 2, &written, nullptr);

	CloseHandle(fileHandle);
}

