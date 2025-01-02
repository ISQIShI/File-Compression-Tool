#include "FileService.h"
#include"HuffmanCode.h"
#include<queue>

void HuffmanCode::GetSymbolFrequency(SelectedFileInfo& selectedFile, size_t dataBlockIndex,size_t fileOffset, size_t fileMapSize)
{
	MapFileInfo* mapFileInfo = new MapFileInfo((LPTSTR)selectedFile.filePath.c_str(), fileOffset, fileMapSize);
	//进行文件映射
	FileService::MapFile(*mapFileInfo,true);
	//获取文件指针
	BYTE* filePointer = (BYTE*)mapFileInfo->mapViewPointer;
	
	//文件指针逐字节遍历整个映射的区域
	for (size_t x = 0; x < mapFileInfo->fileMapSize; ++x,++filePointer) {
		//符号对应的频率增加
		++(selectedFile.dataBlocksymbolFrequency[dataBlockIndex])[*filePointer];
	}
	{//上锁
		std::lock_guard<std::mutex> lock(*selectedFile.threadLock);
		//将局部统计的 符号-频率表 合并到 文件的 符号-频率表中
		MergeSymbolFrequency(selectedFile.symbolFrequency,selectedFile.dataBlocksymbolFrequency[dataBlockIndex]);
	}//自动解锁
	delete mapFileInfo;
}

void HuffmanCode::MergeSymbolFrequency(unordered_map<BYTE, size_t>& targetSymbolFrequency,const unordered_map<BYTE, size_t>& sourceSymbolFrequency)
{
	for (auto& [symbol, frequence] : sourceSymbolFrequency) {
		targetSymbolFrequency[symbol] += frequence;
	}
}

HuffmanNode* HuffmanCode::BuildHuffmanTree(const unordered_map<BYTE, size_t>& symbolFrequency, pair<uintmax_t, BYTE>* WPL_Size)
{
	if (symbolFrequency.empty())return nullptr;
	//将符号-频率表导入优先队列
	priority_queue < HuffmanNode*, vector<HuffmanNode*>, HuffmanNodeCompare > frequencyQueue;
	for (auto& [symbol, frequence] : symbolFrequency) {
		frequencyQueue.push(new HuffmanNode(symbol, frequence));
	}
	//构建哈夫曼树
	HuffmanNode* leftTree, * rightTree, * parentTree;
	BYTE tempBits;
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
			WPL_Size->first += (parentTree->frequency - WPL_Size->second) / 8;
			//更新记录填充的比特数
			tempBits = (parentTree->frequency - WPL_Size->second) % 8;
			if (tempBits) {
				WPL_Size->first += 1;
				WPL_Size->second = 8 - tempBits;
			}
			else {
				WPL_Size->second = 0;
			}
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


void HuffmanCode::GetWPL(SelectedFileInfo& selectedFile, size_t dataBlockIndex, const vector<pair<BYTE, BYTE>>& codeLength)
{
	//空文件直接返回
	if (selectedFile.oldFileSize == 0)return;
	size_t temp;
	BYTE tempBits;
	for (auto& [symbol,clength]: codeLength) {
		temp = selectedFile.dataBlocksymbolFrequency[dataBlockIndex][symbol] * clength;
		//如果新增加的比特个数小于等于填充的比特数，则全部用于"还债"，字节数不增加
		if (temp <= selectedFile.dataBlockWPL_Size[dataBlockIndex].second) {
			selectedFile.dataBlockWPL_Size[dataBlockIndex].second -= temp;
			continue;
		}
		//增加字节计数
		selectedFile.dataBlockWPL_Size[dataBlockIndex].first += (temp - selectedFile.dataBlockWPL_Size[dataBlockIndex].second) / 8;
		//更新记录填充的比特数
		tempBits = (temp - selectedFile.dataBlockWPL_Size[dataBlockIndex].second) % 8;
		if (tempBits) {
			selectedFile.dataBlockWPL_Size[dataBlockIndex].first += 1;
			selectedFile.dataBlockWPL_Size[dataBlockIndex].second = 8 - tempBits;
		}
		else {
			selectedFile.dataBlockWPL_Size[dataBlockIndex].second = 0;
		}
    }
	{//上锁
		std::lock_guard<std::mutex> lock(*selectedFile.threadLock);
		//将局部统计的 WPL_Size 合并到 文件的 WPL_Size 中
		MergeWPL_Size(selectedFile.WPL_Size, selectedFile.dataBlockWPL_Size[dataBlockIndex]);
	}//自动解锁
}

void HuffmanCode::MergeWPL_Size(pair<uintmax_t, BYTE>& targetWPL_Size, const pair<uintmax_t, BYTE>& sourceWPL_Size)
{
	targetWPL_Size.first += sourceWPL_Size.first;
	targetWPL_Size.second += sourceWPL_Size.second;
	//若填充比特数超过1字节
	if (targetWPL_Size.second >= 8) {
		targetWPL_Size.first -= targetWPL_Size.second / 8;
		targetWPL_Size.second = targetWPL_Size.second % 8;
	}
}

void HuffmanCode::GetNormalSymbolCode(vector<pair<BYTE, BYTE>>& codeLength, unordered_map<BYTE, string>& symbolCode)
{
	if (codeLength.empty())return;
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

