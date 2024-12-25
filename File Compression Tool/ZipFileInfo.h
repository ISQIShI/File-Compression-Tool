#pragma once
#include <filesystem>
#include <unordered_map>

using BYTE = unsigned char;
using namespace std;
using namespace filesystem;

struct ZipFileInfo
{
	//�ļ���ʱ·��(����ͬ·���ļ���ѡ��ѹ�������)
	path tempZipFilePath;
	//�ļ�����·��
	path zipFilePath;
	//ѹ�����ѹ�����Ĵ�С(˲ʱֵ,��ѹ������ʵʱ����)
    uintmax_t newFileSize = 0;
	//ȫ�� ����-���볤�ȱ�
	vector<pair<BYTE, BYTE>> codeLength;
	//ȫ�� ����-�����
    unordered_map<BYTE, string> symbolCode;
};