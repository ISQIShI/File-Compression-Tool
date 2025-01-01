#pragma once
#include <filesystem>
#include <unordered_map>
#include <mutex>

using BYTE = unsigned char;
using UINT = unsigned int;
using namespace std;
using namespace filesystem;


struct FileInfo {
	//�ļ�����(0Ϊ�ļ�,1Ϊ�ļ���)
	bool isFolder;
	//�ļ����·��(���ڻ�ԭ�ļ��нṹ)
	path fileName;
	//ԭ�ļ���С(��λΪ�ֽ�)
	uintmax_t oldFileSize;
	//ѹ�����ļ���С(�ֽ�)���Լ���ı�����(bit)
	pair<uintmax_t, BYTE> WPL_Size;
	FileInfo(){}
	FileInfo(bool isfolder,const path& name,uintmax_t oldfilesize,const pair<uintmax_t, BYTE>& wpl_size):isFolder(isfolder),fileName(name),oldFileSize(oldfilesize),WPL_Size(wpl_size){}
};

struct SelectedFileInfo : public FileInfo {
	//ȫ�ַ���-Ƶ�ʱ�
	static unordered_map<BYTE, size_t> * globalSymbolFrequency;
	//���ļ���(�������ļ���)
	static size_t selectedFileAmount;
	//�ļ����
	size_t fileID = 0;
	//�ļ�����·��
	path filePath;
	//��ǰ�ļ��ķ���-Ƶ�ʱ�
	unordered_map<BYTE, size_t> symbolFrequency;
	//�ļ������ֵ����ݿ�Ļ�׼��С
	size_t dataBlockSize;
	//���ݿ����
	UINT dataBlockAmount;
	//�ļ���ÿ���ݿ�ķ���Ƶ�ʱ�
	vector<unordered_map<BYTE, size_t>> dataBlocksymbolFrequency;
	//�ļ���ÿ���ݿ� ѹ�����ļ���С(�ֽ�)���Լ���ı�����(bit)
	vector<pair<uintmax_t, BYTE>> dataBlockWPL_Size;
	//�߳���,������������
	unique_ptr<std::mutex> threadLock;
	//���캯��
	SelectedFileInfo(const path& name, const path& filepath, bool isfolder, uintmax_t oldfilesize) :
		FileInfo(isfolder,name, oldfilesize,{0,0}), filePath(filepath), threadLock(std::make_unique<std::mutex>()) {
		//�����ļ���С�������ݿ��С
		size_t temp = 1024 * 1024;//��ʼ�趨1MB
		if (oldFileSize <= temp) { dataBlockSize = 128 * 1024; }//0-1MB �趨���ݿ�Ϊ128KB
		else if (oldFileSize <= (128 * temp)) { dataBlockSize = 4 * temp; }//��1MB��128MB֮���趨���ݿ�Ϊ4MB
		else if (oldFileSize <= (1024 * temp)) { dataBlockSize = 32 * temp; }//��128MB��1GB֮���趨���ݿ�Ϊ32MB
		else { dataBlockSize = 64 * temp; }//����1GB�趨���ݿ�Ϊ64MB
		//�������ݿ����
		dataBlockAmount = oldFileSize / dataBlockSize;
		//���ʣ���ֽ����������ݿ��С��һ��,������һ�����ݿ�
		if (dataBlockAmount == 0 || ((oldFileSize % dataBlockSize) >= (dataBlockSize / 2)))++dataBlockAmount;
		//��ʼ��������ݿ���Ϣ�� vector ����
		dataBlocksymbolFrequency.resize(dataBlockAmount);
		dataBlockWPL_Size.resize(dataBlockAmount);
		//�趨�ļ����
		if (!isFolder){
			++selectedFileAmount;
			fileID = selectedFileAmount;
		}
	}
	//mutex �������޷����ƺ��ƶ�,ʹ�� unique_ptr ����ָ���װ�����ƶ�
	//�ƶ����캯��
	SelectedFileInfo(SelectedFileInfo&& selectedFileInfo) noexcept
			:FileInfo(selectedFileInfo.isFolder,selectedFileInfo.fileName, selectedFileInfo.oldFileSize, selectedFileInfo.WPL_Size),
			fileID (selectedFileInfo.fileID),
			filePath(move(selectedFileInfo.filePath)),
			dataBlockSize(selectedFileInfo.dataBlockSize),
			dataBlockAmount(selectedFileInfo.dataBlockAmount),
			dataBlocksymbolFrequency(move(selectedFileInfo.dataBlocksymbolFrequency)),
			symbolFrequency(move(selectedFileInfo.symbolFrequency)),
			dataBlockWPL_Size(move(selectedFileInfo.dataBlockWPL_Size)),
			threadLock(move(selectedFileInfo.threadLock)){}
	//�ƶ���ֵ�����
	SelectedFileInfo& operator=(SelectedFileInfo&& selectedFileInfo) noexcept {
		if (this != &selectedFileInfo) {
			fileID = selectedFileInfo.fileID;
			fileName = move(selectedFileInfo.fileName);
			filePath = move(selectedFileInfo.filePath);
			isFolder = selectedFileInfo.isFolder;
			oldFileSize = selectedFileInfo.oldFileSize;
			dataBlockSize = selectedFileInfo.dataBlockSize;
			dataBlockAmount = selectedFileInfo.dataBlockAmount;
			dataBlocksymbolFrequency = move(selectedFileInfo.dataBlocksymbolFrequency);
			symbolFrequency = move(selectedFileInfo.symbolFrequency);
			WPL_Size = selectedFileInfo.WPL_Size;
			dataBlockWPL_Size = move(selectedFileInfo.dataBlockWPL_Size);
			threadLock = move(selectedFileInfo.threadLock);
		}
		return *this;
	}
	//��ֹ��������͸�ֵ����
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
};

struct InternalFileInfo : public FileInfo {
	//���ļ�����
	static size_t folderAmount;
	//�ļ������
	size_t folderID = 0;
	//��ѹ���ļ��ڵ�ƫ����
    size_t fileOffset;
	InternalFileInfo(){};
	InternalFileInfo(bool isfolder,const path& name, uintmax_t oldfilesize, const pair<uintmax_t, BYTE>& wpl_size ,size_t offset,size_t folderid = 0,bool isreturnfolder = false):FileInfo(isfolder,name,oldfilesize,wpl_size),fileOffset(offset) {
		if (isFolder && !isreturnfolder) {//�Զ������ļ������
            ++folderAmount;
			folderID = folderAmount;
		}
		else {
			folderID = folderid;
		}
	}
};