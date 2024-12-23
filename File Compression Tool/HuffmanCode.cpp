﻿#include "FileService.h"
#include"HuffmanCode.h"


void HuffmanCode::GetSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency, const path& fileName, size_t fileOffset, size_t fileMapSize)
{
	MapFileInfo* mapFileInfo = new MapFileInfo((LPTSTR)fileName.c_str(), fileOffset, fileMapSize);
	//进行文件映射
	FileService::MapFileReader(*mapFileInfo);
	//获取文件指针
	BYTE* filePointer = (BYTE*)mapFileInfo->mapViewPointer;
	//文件指针逐字节遍历整个映射的区域
	for (size_t x = 0; x < mapFileInfo->fileMapSize; x++) {
		//字节对应的符号频率增加
		++symbolFrequency[filePointer[x]];
	}
	delete mapFileInfo;
}

void HuffmanCode::MergeSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency1, unordered_map<BYTE, size_t>& symbolFrequency2)
{
	for (auto& [symbol, frequence] : symbolFrequency2) {
		symbolFrequency1[symbol] += frequence;
	}
}

HuffmanNode* HuffmanCode::BuildHuffmanTree(const unordered_map<BYTE, size_t>& symbolFrequency, pair<uintmax_t, BYTE>* WPL_Size)
{
	//将符号-频率表导入优先队列
	priority_queue < HuffmanNode*, vector<HuffmanNode*>, HuffmanNodeCompare > frequencyQueue;
	for (auto& [symbol, frequence] : symbolFrequency) {
		frequencyQueue.push(new HuffmanNode(symbol, frequence));
	}
	//构建哈夫曼树
	HuffmanNode* leftTree, * rightTree, * parentTree;
	while (frequencyQueue.size() > 1) {
		leftTree = frequencyQueue.top();
		frequencyQueue.pop();
		rightTree = frequencyQueue.top();
		frequencyQueue.pop();
		parentTree = new HuffmanNode(0, leftTree->frequency + rightTree->frequency, leftTree, rightTree);
		frequencyQueue.push(parentTree);
		//如果传参，记录WPL
		if (WPL_Size) {
			//如果新增加非叶子节点的权值小于等于填充的比特数，则用于"还债"，字节数不增加
			if (parentTree->frequency <= WPL_Size->second) {
				WPL_Size->second -= parentTree->frequency;
				continue;
			}
			//增加字节计数
			WPL_Size->first += ((parentTree->frequency - WPL_Size->second) / 8) + 1;
			//更新记录填充的比特数
			WPL_Size->second = 8 - ((parentTree->frequency - WPL_Size->second) % 8);
		}
	}
	return frequencyQueue.top();
}



void HuffmanCode::DestroyHuffmanTree(HuffmanNode* rootNode)
{
	if (!rootNode)return;
	//销毁左子树
	DestroyHuffmanTree(rootNode->leftNode);
	//销毁右子树
	DestroyHuffmanTree(rootNode->rightNode);
	//销毁根节点
	delete rootNode;
}

void HuffmanCode::EncodeHuffmanTree(vector<pair<BYTE, BYTE>>& codeLength,HuffmanNode* rootNode,string code, unordered_map<BYTE, string>* symbolCode)
{
	if (!rootNode)return;
	//当rootNode为叶子节点时，将符号-编码长度存入符号-编码长度表
	if (!rootNode->leftNode && !rootNode->rightNode) {
		codeLength.emplace_back(rootNode->symbol, code.length());
		if (symbolCode) {
			(*symbolCode)[rootNode->symbol] = code;
		}
		return;
	}
	//遍历左子树
	EncodeHuffmanTree(codeLength,rootNode->leftNode, code + "0");
	//遍历右子树
	EncodeHuffmanTree(codeLength,rootNode->rightNode, code + "1");
}


void HuffmanCode::GetWPL(unordered_map<BYTE, size_t>& symbolFrequency,const vector<pair<BYTE, BYTE>>& codeLength, pair<uintmax_t, BYTE>& WPL_Size)
{
	//考虑到可能的多线程情况，不直接操作传进来的实参
	pair<uintmax_t, BYTE> wpl_size = { 0,0 };
	size_t temp;
	for (auto& [symbol,clength]: codeLength) {
		temp = symbolFrequency[symbol] * clength;
		//如果新增加的权值小于等于填充的比特数，则用于"还债"，字节数不增加
		if (temp <= wpl_size.second) {
			wpl_size.second -= temp;
			continue;
		}
		//增加字节计数
		wpl_size.first += ((temp - wpl_size.second) / 8) + 1;
		//更新记录填充的比特数
		wpl_size.second = 8 - ((temp - wpl_size.second) % 8);
    }
	//将结果叠加到实参当中
    WPL_Size.first += wpl_size.first;
    WPL_Size.second += wpl_size.second;
	//若填充比特数超过1字节
	if (WPL_Size.second >= 8) {
		WPL_Size.first -= WPL_Size.second / 8;
        WPL_Size.second = WPL_Size.second % 8;
	}
}

void HuffmanCode::GetNormalSymbolCode(vector<pair<BYTE, BYTE>>& codeLength, unordered_map<BYTE, string>& symbolCode)
{
	//先对符号-编码长度表进行排序
	sort(codeLength.begin(), codeLength.end(), [](const pair<BYTE, BYTE>& x, const pair<BYTE, BYTE>& y) {
		//编码长度越小越靠前，编码长度相同时，符号值越小越靠前
		return (x.second < y.second) || ((x.second == y.second) && (x.first < y.first)); 
		});
	//处理只有一个符号时，编码长度为0的情况
	if (codeLength.size() == 1 && codeLength[0].second == 0) { codeLength[0].second = 1; }
	//上一个进行编码的符号的编码长度
	BYTE lastLength = codeLength[0].second;
	//上一个符号的编码
	string lastCode(lastLength,'0');
	symbolCode[codeLength[0].first] = lastCode;
	for (size_t i = 1; i < codeLength.size();++i) {
		//在前一个编码的基础上加1
		for (size_t x = lastLength - 1; x >= 0; --x) {
			if (lastCode[x] == '0') {
				lastCode[x] = '1';
				break;
			}
			lastCode[x] = '0';
		}
		//编码长度比前一个的大k，加1之后左移k位(即追加k个0)
		if (codeLength[i].second > lastLength) {
			lastCode.append(codeLength[i].second - lastLength, '0');
		}
		lastLength = codeLength[i].second;
		symbolCode[codeLength[i].first] = lastCode;
	}
}

