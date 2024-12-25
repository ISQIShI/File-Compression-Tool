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
	std::mutex* threadLock = new std::mutex;
	//构造函数
	SelectedFileInfo(const path& name,const path& filepath, bool isfolder, uintmax_t oldfilesize) :fileName(name), filePath(filepath), isFolder(isfolder), oldFileSize(oldfilesize), WPL_Size(0, 0) {}
	~SelectedFileInfo() { delete threadLock; }
};