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
	std::mutex* threadLock = new std::mutex;
	//���캯��
	SelectedFileInfo(const path& name,const path& filepath, bool isfolder, uintmax_t oldfilesize) :fileName(name), filePath(filepath), isFolder(isfolder), oldFileSize(oldfilesize), WPL_Size(0, 0) {}
	~SelectedFileInfo() { delete threadLock; }
};