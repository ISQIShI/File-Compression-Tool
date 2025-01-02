#pragma once
//
//#ifdef _HAS_STD_BYTE
//#undef _HAS_STD_BYTE
//#endif
//#define _HAS_STD_BYTE 0

#include <filesystem>
#include <unordered_map>
#include <mutex>

using BYTE = unsigned char;
using UINT = unsigned int;

using namespace std::filesystem;


struct FileInfo {
	//�ļ�����(0Ϊ�ļ�,1Ϊ�ļ���)
	bool isFolder;
	//�ļ����·��(���ڻ�ԭ�ļ��нṹ)
	path fileName;
	//�ļ�����·��
	path filePath;
	//ԭ�ļ���С(��λΪ�ֽ�)
	uintmax_t oldFileSize;
	//ѹ�����ļ���С(�ֽ�)���Լ���ı�����(bit)
	std::pair<uintmax_t, BYTE> WPL_Size;
	FileInfo(){}
	FileInfo(bool isfolder,const path& name, const path& filepath,uintmax_t oldfilesize,const std::pair<uintmax_t, BYTE>& wpl_size):isFolder(isfolder),fileName(name),filePath(filepath), oldFileSize(oldfilesize), WPL_Size(wpl_size) {}
};

struct SelectedFileInfo : public FileInfo {
	//ȫ�ַ���-Ƶ�ʱ�
	static std::unordered_map<BYTE, std::size_t> * globalSymbolFrequency;
	//���ļ���(�������ļ���)
	static std::size_t selectedFileAmount;
	//�ļ����
	std::size_t fileID = 0;
	//��ǰ�ļ��ķ���-Ƶ�ʱ�
	std::unordered_map<BYTE, std::size_t> symbolFrequency;
	//�ļ������ֵ����ݿ�Ļ�׼��С
	std::size_t dataBlockSize;
	//���ݿ����
	UINT dataBlockAmount;
	//�ļ���ÿ���ݿ�ķ���Ƶ�ʱ�
	std::vector<std::unordered_map<BYTE, std::size_t>> dataBlocksymbolFrequency;
	//�ļ���ÿ���ݿ� ѹ�����ļ���С(�ֽ�)���Լ���ı�����(bit)
	std::vector<std::pair<uintmax_t, BYTE>> dataBlockWPL_Size;
	//�߳���,������������
	std::unique_ptr<std::mutex> threadLock;
	//���캯��
	SelectedFileInfo(const path& name, const path& filepath, bool isfolder, uintmax_t oldfilesize) :
		FileInfo(isfolder,name, filepath, oldfilesize,{0,0}), threadLock(std::make_unique<std::mutex>()) {
		//�����ļ���С�������ݿ��С
		std::size_t temp = 1024 * 1024;//��ʼ�趨1MB
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
			:FileInfo(selectedFileInfo.isFolder,selectedFileInfo.fileName, selectedFileInfo.filePath,selectedFileInfo.oldFileSize, selectedFileInfo.WPL_Size),
			fileID (selectedFileInfo.fileID),
			dataBlockSize(selectedFileInfo.dataBlockSize),
			dataBlockAmount(selectedFileInfo.dataBlockAmount),
			dataBlocksymbolFrequency(std::move(selectedFileInfo.dataBlocksymbolFrequency)),
			symbolFrequency(std::move(selectedFileInfo.symbolFrequency)),
			dataBlockWPL_Size(std::move(selectedFileInfo.dataBlockWPL_Size)),
			threadLock(std::move(selectedFileInfo.threadLock)){}
	//�ƶ���ֵ�����
	SelectedFileInfo& operator=(SelectedFileInfo&& selectedFileInfo) noexcept {
		if (this != &selectedFileInfo) {
			fileID = selectedFileInfo.fileID;
			fileName = std::move(selectedFileInfo.fileName);
			filePath = std::move(selectedFileInfo.filePath);
			isFolder = selectedFileInfo.isFolder;
			oldFileSize = selectedFileInfo.oldFileSize;
			dataBlockSize = selectedFileInfo.dataBlockSize;
			dataBlockAmount = selectedFileInfo.dataBlockAmount;
			dataBlocksymbolFrequency = std::move(selectedFileInfo.dataBlocksymbolFrequency);
			symbolFrequency = std::move(selectedFileInfo.symbolFrequency);
			WPL_Size = selectedFileInfo.WPL_Size;
			dataBlockWPL_Size = std::move(selectedFileInfo.dataBlockWPL_Size);
			threadLock = std::move(selectedFileInfo.threadLock);
		}
		return *this;
	}
	//��ֹ��������͸�ֵ����
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
};

struct InternalFileInfo : public FileInfo {
	//���ļ�����
	static std::size_t folderAmount;
	//�ļ������
	std::size_t folderID = 0;
	//��ѹ���ļ��ڵ�ƫ����
	std::size_t fileOffset;
	InternalFileInfo(){};
	InternalFileInfo(bool isfolder,const path& name, const path& filepath,uintmax_t oldfilesize, const std::pair<uintmax_t, BYTE>& wpl_size , std::size_t offset,size_t folderid = 0,bool isreturnfolder = false):FileInfo(isfolder,name,filepath,oldfilesize,wpl_size),fileOffset(offset) {
		if (isFolder && !isreturnfolder) {//�Զ������ļ������
            ++folderAmount;
			folderID = folderAmount;
		}
		else {
			folderID = folderid;
		}
	}
};