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
	//文件类型(0为文件,1为文件夹)
	bool isFolder;
	//文件相对路径(用于还原文件夹结构)
	path fileName;
	//文件绝对路径
	path filePath;
	//原文件大小(单位为字节)
	uintmax_t oldFileSize;
	//压缩后文件大小(字节)，以及填补的比特数(bit)
	std::pair<uintmax_t, BYTE> WPL_Size;
	FileInfo(){}
	FileInfo(bool isfolder,const path& name, const path& filepath,uintmax_t oldfilesize,const std::pair<uintmax_t, BYTE>& wpl_size):isFolder(isfolder),fileName(name),filePath(filepath), oldFileSize(oldfilesize), WPL_Size(wpl_size) {}
};

struct SelectedFileInfo : public FileInfo {
	//全局符号-频率表
	static std::unordered_map<BYTE, std::size_t> * globalSymbolFrequency;
	//总文件数(不计入文件夹)
	static std::size_t selectedFileAmount;
	//文件序号
	std::size_t fileID = 0;
	//当前文件的符号-频率表
	std::unordered_map<BYTE, std::size_t> symbolFrequency;
	//文件被划分的数据块的基准大小
	std::size_t dataBlockSize;
	//数据块个数
	UINT dataBlockAmount;
	//文件中每数据块的符号频率表
	std::vector<std::unordered_map<BYTE, std::size_t>> dataBlocksymbolFrequency;
	//文件中每数据块 压缩后文件大小(字节)，以及填补的比特数(bit)
	std::vector<std::pair<uintmax_t, BYTE>> dataBlockWPL_Size;
	//线程锁,保护共享数据
	std::unique_ptr<std::mutex> threadLock;
	//构造函数
	SelectedFileInfo(const path& name, const path& filepath, bool isfolder, uintmax_t oldfilesize) :
		FileInfo(isfolder,name, filepath, oldfilesize,{0,0}), threadLock(std::make_unique<std::mutex>()) {
		//根据文件大小划分数据块大小
		std::size_t temp = 1024 * 1024;//初始设定1MB
		if (oldFileSize <= temp) { dataBlockSize = 128 * 1024; }//0-1MB 设定数据块为128KB
		else if (oldFileSize <= (128 * temp)) { dataBlockSize = 4 * temp; }//在1MB到128MB之间设定数据块为4MB
		else if (oldFileSize <= (1024 * temp)) { dataBlockSize = 32 * temp; }//在128MB到1GB之间设定数据块为32MB
		else { dataBlockSize = 64 * temp; }//大于1GB设定数据块为64MB
		//计算数据块个数
		dataBlockAmount = oldFileSize / dataBlockSize;
		//如果剩余字节数超过数据块大小的一半,则增加一个数据块
		if (dataBlockAmount == 0 || ((oldFileSize % dataBlockSize) >= (dataBlockSize / 2)))++dataBlockAmount;
		//初始化存放数据块信息的 vector 容器
		dataBlocksymbolFrequency.resize(dataBlockAmount);
		dataBlockWPL_Size.resize(dataBlockAmount);
		//设定文件序号
		if (!isFolder){
			++selectedFileAmount;
			fileID = selectedFileAmount;
		}
	}
	//mutex 互斥锁无法复制和移动,使用 unique_ptr 智能指针包装可以移动
	//移动构造函数
	SelectedFileInfo(SelectedFileInfo&& selectedFileInfo) noexcept
			:FileInfo(selectedFileInfo.isFolder,selectedFileInfo.fileName, selectedFileInfo.filePath,selectedFileInfo.oldFileSize, selectedFileInfo.WPL_Size),
			fileID (selectedFileInfo.fileID),
			dataBlockSize(selectedFileInfo.dataBlockSize),
			dataBlockAmount(selectedFileInfo.dataBlockAmount),
			dataBlocksymbolFrequency(std::move(selectedFileInfo.dataBlocksymbolFrequency)),
			symbolFrequency(std::move(selectedFileInfo.symbolFrequency)),
			dataBlockWPL_Size(std::move(selectedFileInfo.dataBlockWPL_Size)),
			threadLock(std::move(selectedFileInfo.threadLock)){}
	//移动赋值运算符
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
	//禁止拷贝构造和赋值操作
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
};

struct InternalFileInfo : public FileInfo {
	//总文件夹数
	static std::size_t folderAmount;
	//文件夹序号
	std::size_t folderID = 0;
	//在压缩文件内的偏移量
	std::size_t fileOffset;
	InternalFileInfo(){};
	InternalFileInfo(bool isfolder,const path& name, const path& filepath,uintmax_t oldfilesize, const std::pair<uintmax_t, BYTE>& wpl_size , std::size_t offset,size_t folderid = 0,bool isreturnfolder = false):FileInfo(isfolder,name,filepath,oldfilesize,wpl_size),fileOffset(offset) {
		if (isFolder && !isreturnfolder) {//自动分配文件夹序号
            ++folderAmount;
			folderID = folderAmount;
		}
		else {
			folderID = folderid;
		}
	}
};