#pragma once
#include <filesystem>
#include <unordered_map>

using BYTE = unsigned char;
using namespace std;
using namespace filesystem;

struct ZipFileInfo
{
	//文件临时路径(处理同路径文件被选择压缩的情况)
	path tempZipFilePath;
	//文件绝对路径
	path zipFilePath;
	//压缩后的压缩包的大小(瞬时值,随压缩过程实时更新)
    uintmax_t newFileSize = 0;
	//全局 符号-编码长度表
	vector<pair<BYTE, BYTE>> codeLength;
	//全局 符号-编码表
    unordered_map<BYTE, string> symbolCode;
};