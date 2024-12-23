#include"FileService.h"

void FileService::ErrorMessageBox(const HWND& hwnd, const TCHAR* msg, bool showErrorCode)
{
	//�����ַ������ڽ��մ������
	TCHAR errorCode[20] = _T("");
	if (showErrorCode) _stprintf_s(errorCode, _T("\n�������:%lu"), GetLastError());
	//��msg������������ƴ������
	size_t length = _tcslen(msg) + _tcslen(errorCode) + 1;
	TCHAR* finalMsg = new TCHAR[length];
	_stprintf_s(finalMsg, length, _T("%s%s"), msg, errorCode);
	//�������󵯴�
	MessageBox(hwnd, finalMsg, _T("������Ϣ"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
	//����ʹ��new���ٵĿռ�
	delete[]finalMsg;
	//�˳�����
	exit(GetLastError());
}

void FileService::MapFileReader(MapFileInfo& mapFileInfo)
{
	//ʹ���ڴ�ӳ�䷽ʽ��ȡ�ļ�������ļ���ȡ�ٶ�
	//���ļ��Ի�ȡ���
	mapFileInfo.fileHandle = CreateFile(mapFileInfo.fileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mapFileInfo.fileHandle == INVALID_HANDLE_VALUE)ErrorMessageBox(NULL,_T("�޷����ļ�"));
	//�����ļ�ӳ������ȡ���
	mapFileInfo.fileMapHandle = CreateFileMapping(mapFileInfo.fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (mapFileInfo.fileMapHandle == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
		ErrorMessageBox(NULL, _T("�޷������ļ�ӳ�����"));
	}
	//ӳ���ļ���ͼ��ȡ�ļ�ӳ��ָ��
	mapFileInfo.mapViewPointer = MapViewOfFile(mapFileInfo.fileMapHandle, FILE_MAP_READ, (mapFileInfo.fileOffset >> 32) & 0xffffffff, mapFileInfo.fileOffset & 0xffffffff, mapFileInfo.fileMapSize);
	if (mapFileInfo.mapViewPointer == NULL) {
        CloseHandle(mapFileInfo.fileHandle);
        CloseHandle(mapFileInfo.fileMapHandle);
		ErrorMessageBox(NULL, _T("�޷�ӳ���ļ���ͼ"));
	}
	if (mapFileInfo.fileMapSize == 0) {
		mapFileInfo.fileMapSize = file_size(mapFileInfo.fileName);
	}
}

uintmax_t FileService::GetFileSize(const path& fileName)
{
	if (!exists(fileName))ErrorMessageBox(NULL, _T("�ļ�(��)������,�޷���ȡ��С"));
	//��ȡ��ͨ�ļ���С
	if (is_regular_file(fileName))return file_size(fileName);
	//��ȡ�ļ��д�С
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
	//�����ļ�ӳ��
	FileService::MapFileReader(*mapFileInfo);
	//��ȡ�ļ�ָ��
	BYTE* filePointer = (BYTE*)mapFileInfo->mapViewPointer;
	//��ѹ�����ļ�
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		delete mapFileInfo;
		FileService::ErrorMessageBox(NULL, _T("�޷����ļ�"));
	}
	//�����ļ�ָ��
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	// д�뻺����
	std::vector<BYTE> writeBuffer; // д�뻺����
	size_t bufferCapacity = 4096; // ���û�������С
	writeBuffer.reserve(bufferCapacity);

	UINT currentWord = 0; // ��ǰ 32 λ���ݿ�
	int bitCount = 0;     // ��ǰ���ݿ���������λ��

	// �����ļ�ӳ������
	for (size_t i = 0; i < mapFileInfo->fileMapSize; ++i) {
		const std::string& code = symbolCode.at(filePointer[i]);

		// ��������λ��䵽��ǰ���ݿ�
		for (char bit : code) {
			currentWord = (currentWord << 1) | (bit - '0');
			++bitCount;

			// �����ǰ���ݿ�������32 λ�������뻺����
			if (bitCount == 32) {
				// ת��ΪС���ֽ��򲢼��뻺����
				for (int j = 0; j < 4; ++j) {
					writeBuffer.push_back(static_cast<BYTE>((currentWord >> (8 * (3 - j))) & 0xFF));
				}

				currentWord = 0;
				bitCount = 0;

				// ������������ˣ�д���ļ�
				if (writeBuffer.size() >= bufferCapacity) {
					DWORD written;
					WriteFile(fileHandle, writeBuffer.data(), writeBuffer.size(), &written, nullptr);
					writeBuffer.clear();
				}
			}
		}
	}

	// ����ʣ�಻�� 32 λ�����ݿ�
	if (bitCount > 0) {
		currentWord <<= (32 - bitCount); // ����뵽 32 λ
		for (int j = 0; j < 4; ++j) {
			writeBuffer.push_back(static_cast<BYTE>((currentWord >> (8 * (3 - j))) & 0xFF));
		}
	}

	// д��ʣ������
	if (!writeBuffer.empty()) {
		DWORD written;
		WriteFile(fileHandle, writeBuffer.data(), writeBuffer.size(), &written, nullptr);
	}
	CloseHandle(fileHandle);
	delete mapFileInfo;
}

void FileService::WriteZipFileHeader(const path& destFile, const vector<pair<BYTE, BYTE>>& codeLength)
{
	//��ѹ�����ļ�
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("�޷����ļ�"));
	}
	//�����ļ�ָ��
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	DWORD written;
	//��ʶǩ��
	char identification[2] = { 'y','a' };
	WriteFile(fileHandle, identification, sizeof(identification), &written, nullptr);
	//�ײ���С
	unsigned short headerSize = 4 + sizeof(codeLength[0]) * codeLength.size();
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//����-���볤�ȱ�
	for (size_t i = 0; i < codeLength.size(); ++i)
	{
		WriteFile(fileHandle, &codeLength[i], sizeof(codeLength[i]), &written, nullptr);
	}

	CloseHandle(fileHandle);
}

void FileService::WriteFileHeader(const path& destFile, const path& sourceFile, const pair<uintmax_t, BYTE>& WPL_Size)
{
	//��ѹ�����ļ�
	HANDLE fileHandle = CreateFile(destFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		FileService::ErrorMessageBox(NULL, _T("�޷����ļ�"));
	}
	//�����ļ�ָ��
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_END);
	DWORD written;
	//�ײ���С
	unsigned short headerSize = 19 + sourceFile.native().size() * 2;
	WriteFile(fileHandle, &headerSize, sizeof(headerSize), &written, nullptr);
	//�ļ����ͺ���������
	BYTE fileType_bitsCount = ((is_directory(sourceFile) ? 1 : 0) << 7) + WPL_Size.second;
	WriteFile(fileHandle, &fileType_bitsCount, sizeof(fileType_bitsCount), &written, nullptr);
	//ԭʼ�ļ�(��)��С
	uintmax_t oldSize = GetFileSize(sourceFile);
	WriteFile(fileHandle, &oldSize, sizeof(oldSize), &written, nullptr);
	//���ݿ��С
	WriteFile(fileHandle, &WPL_Size.first, sizeof(WPL_Size.first), &written, nullptr);
	//�ļ���
	WriteFile(fileHandle, sourceFile.c_str(), sourceFile.native().size() * 2, &written, nullptr);

	CloseHandle(fileHandle);
}

