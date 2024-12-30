#pragma once
#include <filesystem>
#include <unordered_map>
#include <mutex>

using BYTE = unsigned char;
using UINT = unsigned int;
using namespace std;
using namespace filesystem;


struct SelectedFileInfo {
	//全局符号-频率表
	static unordered_map<BYTE, size_t> * globalSymbolFrequency;
	//总文件数(不计入文件夹)
	static size_t selectedFileAmount;
	//文件序号
	size_t fileID = 0;
	//文件相对路径(用于还原文件夹结构)
	path fileName;
	//文件绝对路径
	path filePath;
	//文件类型(0为文件,1为文件夹)
	bool isFolder;
	//原文件大小(单位为字节)
	uintmax_t oldFileSize;
	//当前文件的符号-频率表
	unordered_map<BYTE, size_t> symbolFrequency;
	//文件被划分的数据块的基准大小
	size_t dataBlockSize;
	//数据块个数
	UINT dataBlockAmount;
	//文件中每数据块的符号频率表
	vector<unordered_map<BYTE, size_t>> dataBlocksymbolFrequency;
	//压缩后文件大小(字节)，以及填补的比特数(bit)
	pair<uintmax_t, BYTE> WPL_Size;
	//文件中每数据块 压缩后文件大小(字节)，以及填补的比特数(bit)
	vector<pair<uintmax_t, BYTE>> dataBlockWPL_Size;
	//线程锁,保护共享数据
	unique_ptr<std::mutex> threadLock;
	//构造函数
	SelectedFileInfo(const path& name, const path& filepath, bool isfolder, uintmax_t oldfilesize) :fileName(name), filePath(filepath), isFolder(isfolder), oldFileSize(oldfilesize), WPL_Size(0, 0), threadLock(std::make_unique<std::mutex>()) {
		//根据文件大小划分数据块大小
		size_t temp = 1024 * 1024;//初始设定1MB
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
			:fileID(selectedFileInfo.fileID),
			fileName(move(selectedFileInfo.fileName)), 
			filePath(move(selectedFileInfo.filePath)), 
			isFolder(selectedFileInfo.isFolder), 
			oldFileSize(selectedFileInfo.oldFileSize),
			dataBlockSize(selectedFileInfo.dataBlockSize),
			dataBlockAmount(selectedFileInfo.dataBlockAmount),
			dataBlocksymbolFrequency(move(selectedFileInfo.dataBlocksymbolFrequency)),
			symbolFrequency(move(selectedFileInfo.symbolFrequency)),
			WPL_Size(selectedFileInfo.WPL_Size),
			dataBlockWPL_Size(move(selectedFileInfo.dataBlockWPL_Size)),
			threadLock(move(selectedFileInfo.threadLock)){}
	//移动赋值运算符
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
	//禁止拷贝构造和赋值操作
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
	~SelectedFileInfo() {}
};

