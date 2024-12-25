#pragma once
#include <filesystem>
#include <unordered_map>
#include <mutex>

using BYTE = unsigned char;
using namespace std;
using namespace filesystem;

struct SelectedFileInfo {
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
	//压缩后文件大小(字节)，以及填补的比特数(bit)
	pair<uintmax_t, BYTE> WPL_Size;
	//线程锁,保护共享数据
	unique_ptr<std::mutex> threadLock;
	//构造函数
	SelectedFileInfo(const path& name,const path& filepath, bool isfolder, uintmax_t oldfilesize) :fileName(name), filePath(filepath), isFolder(isfolder), oldFileSize(oldfilesize), WPL_Size(0, 0),threadLock(std::make_unique<std::mutex>()){}
	//mutex互斥锁无法复制和移动,使用unique_ptr智能指针包装可以移动
	//移动构造函数
	SelectedFileInfo(SelectedFileInfo&& selectedFileInfo) noexcept
		: fileName(move(selectedFileInfo.fileName)), filePath(move(selectedFileInfo.filePath)), isFolder(selectedFileInfo.isFolder), oldFileSize(selectedFileInfo.oldFileSize),
		symbolFrequency(move(selectedFileInfo.symbolFrequency)), WPL_Size(selectedFileInfo.WPL_Size),threadLock(move(selectedFileInfo.threadLock)){}
	//移动赋值运算符
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
	//禁止拷贝构造和赋值操作
	SelectedFileInfo(const SelectedFileInfo&) = delete;
	SelectedFileInfo& operator=(const SelectedFileInfo&) = delete;
};