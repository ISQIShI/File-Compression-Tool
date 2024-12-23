#include "WinFileProc.h"
#include"HuffmanCode.h"


void HuffmanCode::GetSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency, const path& fileName, size_t fileOffset, size_t fileMapSize)
{
	MapFileInfo* mapFileInfo = new MapFileInfo((LPTSTR)fileName.c_str(), fileOffset, fileMapSize);
	//�����ļ�ӳ��
	WinFileProc::MapFileReader(*mapFileInfo);
	//��ȡ�ļ�ָ��
	BYTE* filePointer = (BYTE*)mapFileInfo->mapViewPointer;
	//�ļ�ָ�����ֽڱ�������ӳ�������
	for (size_t x = 0; x < mapFileInfo->fileMapSize; x++) {
		//�ֽڶ�Ӧ�ķ���Ƶ������
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
	//������-Ƶ�ʱ������ȶ���
	priority_queue < HuffmanNode*, vector<HuffmanNode*>, HuffmanNodeCompare > frequencyQueue;
	for (auto& [symbol, frequence] : symbolFrequency) {
		frequencyQueue.push(new HuffmanNode(symbol, frequence));
	}
	//������������
	HuffmanNode* leftTree, * rightTree, * parentTree;
	while (frequencyQueue.size() > 1) {
		leftTree = frequencyQueue.top();
		frequencyQueue.pop();
		rightTree = frequencyQueue.top();
		frequencyQueue.pop();
		parentTree = new HuffmanNode(0, leftTree->frequency + rightTree->frequency, leftTree, rightTree);
		frequencyQueue.push(parentTree);
		//������Σ���¼WPL
		if (WPL_Size) {
			//��������ӷ�Ҷ�ӽڵ��ȨֵС�ڵ������ı�������������"��ծ"���ֽ���������
			if (parentTree->frequency <= WPL_Size->second) {
				WPL_Size->second -= parentTree->frequency;
				continue;
			}
			//�����ֽڼ���
			WPL_Size->first += ((parentTree->frequency - WPL_Size->second) / 8) + 1;
			//���¼�¼���ı�����
			WPL_Size->second = 8 - ((parentTree->frequency - WPL_Size->second) % 8);
		}
	}
	return frequencyQueue.top();
}



void HuffmanCode::DestroyHuffmanTree(HuffmanNode* rootNode)
{
	if (!rootNode)return;
	//����������
	DestroyHuffmanTree(rootNode->leftNode);
	//����������
	DestroyHuffmanTree(rootNode->rightNode);
	//���ٸ��ڵ�
	delete rootNode;
}

void HuffmanCode::EncodeHuffmanTree(vector<pair<BYTE, BYTE>>& codeLength,HuffmanNode* rootNode,string code, unordered_map<BYTE, string>* symbolCode)
{
	if (!rootNode)return;
	//��rootNodeΪҶ�ӽڵ�ʱ��������-���볤�ȴ������-���볤�ȱ�
	if (!rootNode->leftNode && !rootNode->rightNode) {
		codeLength.emplace_back(rootNode->symbol, code.length());
		if (symbolCode) {
			(*symbolCode)[rootNode->symbol] = code;
		}
		return;
	}
	//����������
	EncodeHuffmanTree(codeLength,rootNode->leftNode, code + "0");
	//����������
	EncodeHuffmanTree(codeLength,rootNode->rightNode, code + "1");
}


void HuffmanCode::GetWPL(unordered_map<BYTE, size_t>& symbolFrequency,const vector<pair<BYTE, BYTE>>& codeLength, pair<uintmax_t, BYTE>& WPL_Size)
{
	//���ǵ����ܵĶ��߳��������ֱ�Ӳ�����������ʵ��
	pair<uintmax_t, BYTE> wpl_size = { 0,0 };
	size_t temp;
	for (auto& [symbol,clength]: codeLength) {
		temp = symbolFrequency[symbol] * clength;
		//��������ӵ�ȨֵС�ڵ������ı�������������"��ծ"���ֽ���������
		if (temp <= wpl_size.second) {
			wpl_size.second -= temp;
			continue;
		}
		//�����ֽڼ���
		wpl_size.first += ((temp - wpl_size.second) / 8) + 1;
		//���¼�¼���ı�����
		wpl_size.second = 8 - ((temp - wpl_size.second) % 8);
    }
	//��������ӵ�ʵ�ε���
    WPL_Size.first += wpl_size.first;
    WPL_Size.second += wpl_size.second;
	//��������������1�ֽ�
	if (WPL_Size.second >= 8) {
		WPL_Size.first -= WPL_Size.second / 8;
        WPL_Size.second = WPL_Size.second % 8;
	}
}

void HuffmanCode::GetNormalSymbolCode(vector<pair<BYTE, BYTE>>& codeLength, unordered_map<BYTE, string>& symbolCode)
{
	//�ȶԷ���-���볤�ȱ��������
	sort(codeLength.begin(), codeLength.end(), [](const pair<BYTE, BYTE>& x, const pair<BYTE, BYTE>& y) {
		//���볤��ԽСԽ��ǰ�����볤����ͬʱ������ֵԽСԽ��ǰ
		return (x.second < y.second) || ((x.second == y.second) && (x.first < y.first)); 
		});
	//����ֻ��һ������ʱ�����볤��Ϊ0�����
	if (codeLength.size() == 1 && codeLength[0].second == 0) { codeLength[0].second = 1; }
	//��һ�����б���ķ��ŵı��볤��
	BYTE lastLength = codeLength[0].second;
	//��һ�����ŵı���
	string lastCode(lastLength,'0');
	symbolCode[codeLength[0].first] = lastCode;
	for (size_t i = 1; i < codeLength.size();++i) {
		//��ǰһ������Ļ����ϼ�1
		for (size_t x = lastLength - 1; x >= 0; --x) {
			if (lastCode[x] == '0') {
				lastCode[x] = '1';
				break;
			}
			lastCode[x] = '0';
		}
		//���볤�ȱ�ǰһ���Ĵ�k����1֮������kλ(��׷��k��0)
		if (codeLength[i].second > lastLength) {
			lastCode.append(codeLength[i].second - lastLength, '0');
		}
		lastLength = codeLength[i].second;
		symbolCode[codeLength[i].first] = lastCode;
	}
}

