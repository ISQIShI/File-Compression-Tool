#pragma once
#include <filesystem>
#include <unordered_map>
#include <mutex>

using BYTE = unsigned char;
using namespace std;
using namespace filesystem;

struct SelectedFileInfo {
	//�ļ����·��(���ڻ�ԭ�ļ��нṹ)
	path fileName;
	//�ļ�����·��
	path filePath;
	//�ļ�����(0Ϊ�ļ�,1Ϊ�ļ���)
	bool isFolder;
	//ԭ�ļ���С(��λΪ�ֽ�)
	uintmax_t oldFileSize;
	//��ǰ�ļ��ķ���-Ƶ�ʱ�
	unordered_map<BYTE, size_t> symbolFrequency;
	//ѹ�����ļ���С(�ֽ�)���Լ���ı�����(bit)
	pair<uintmax_t, BYTE> WPL_Size;
	//�߳���,������������
	unique_ptr<std::mutex> threadLock;
	//���캯��
	SelectedFileInfo(const path& name,const path& filepath, bool isfolder, uintmax_t oldfilesize) :fileName(name), filePath(filepath), isFolder(isfolder), oldFileSize(oldfilesize), WPL_Size(0, 0),threadLock(std::make_unique<std::mutex>()){}
	//mutex�������޷����ƺ��ƶ�,ʹ��unique_ptr����ָ���װ�����ƶ�
	//�ƶ����캯��
	SelectedFileInfo(SelectedFileInfo&& selectedFileInfo) noexcept
		: fileName(move(selectedFileInfo.fileName)), filePath(move(selectedFileInfo.filePath)), isFolder(selectedFileInfo.isFolder), oldFileSize(selectedFileInfo.oldFileSize),
		symbolFrequency(move(selectedFileInfo.symbolFrequency)), WPL_Size(selectedFileInfo.WPL_Size),threadLock(move(selectedFileInfo.threadLock)){}
	//�ƶ���ֵ�����
	SelectedFileInfo& operator=(SelectedFileInfo&& selectedFileInfo) noexcept {
		if (this != &selectedFileInfo) {
			fileName = move(selectedFileInfo.fileName);
			filePath = move(selectedFileInfo.filePath);
			isFolder = selectedFileInfo.isFolder;
			oldFileSize = selectedFileInfo.oldFileSize;
			symbolFrequency = move(selectedFileInfo.symbolFrequency);
			WPL_Size = selectedFileInfo.WPL_Size;
			threadLock = move(selectedFileInfo.threadLock);
		}
		return *this;
	}
	//��ֹ��������͸�ֵ����
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
};