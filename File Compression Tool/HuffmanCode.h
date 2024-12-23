#pragma once
#include<unordered_map>
#include<queue>


class HuffmanNode {
public:
	BYTE symbol;//����
	size_t frequency;//Ƶ��
	HuffmanNode* leftNode;//��ڵ�
	HuffmanNode* rightNode;//�ҽڵ�
	HuffmanNode(BYTE s, size_t f, HuffmanNode* left = nullptr, HuffmanNode* right = nullptr) :symbol(s), frequency(f), leftNode(left), rightNode(right) {}
};

//�������ȶ����й������ڵ�Ƚϵķº���
struct HuffmanNodeCompare {
	bool operator()(const HuffmanNode* x, const HuffmanNode* y) const
    {
		//Ƶ��ԽС���ȼ�Խ�ߣ�Ƶ����ͬʱ������ֵԽС���ȼ�Խ��
		return (x->frequency > y->frequency) || ((x->frequency == y->frequency) && (x->symbol > y->symbol)); 
    }
};

class HuffmanCode {
public:
	/*--------����Ҫ��������----[out]������Ϊ������� [in]������Ϊ������� [optional]������Ϊ��ѡ����------
	����-Ƶ�ʱ�,���ڴ洢�ļ��и������ų��ֵ�Ƶ��
	unordered_map<BYTE, size_t> symbolFrequency; 
	[out]GetSymbolFrequency [in]BuildHuffmanTree 

	����-�����,���ڴ洢�����Ŷ�Ӧ�ı���
	unordered_map<BYTE, string> symbolCode;
	[out]GetNormalSymbolCode [out,optional]EncodeHuffmanTree 

	����-���볤�ȱ�
	vector<pair<BYTE,BYTE>> codeLength;
	//priority_queue<pair<BYTE,BYTE>,vector<pair<BYTE, BYTE>>, CodeLengthCompare> codeLength;//�Ѿ�����
	[out]EncodeHuffmanTree [in]GetNormalSymbolCode GetWPL

	һ�Ź��������Ĵ�Ȩ·������WPL,��ʹ�ø������б���ѹ�������ݵ��ܳ���,first����Ϊ�ֽ�����second����Ϊ����1�ֽ����ı�����
	���ʹ�÷ֿ鴦�������ڼ����Ĵ�С
	pair<uintmax_t, BYTE>* WPL_Size
	*/
	
	//��ȡ�ļ�ĳ������ ����-Ƶ�ʱ� (�������������ڿ����ļ���ȡ��Χ,����ʵ�ִ��ļ��ֿ��ȡ)
	static void GetSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency, const path& fileName, size_t fileOffset = 0, size_t fileMapSize = 0);
	//�ϲ����� ����-Ƶ�ʱ� (��2�ϲ�����1)
	static void MergeSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency1, unordered_map<BYTE, size_t>& symbolFrequency2);
	//���� ����-Ƶ�ʱ� ����һ�Ź������������ظ��ڵ�ָ�� ���WPL(�����ݴ���ķ���-Ƶ�ʱ����ѹ�������ݵ��ܳ���)(��ѡ)
	static HuffmanNode* BuildHuffmanTree(const unordered_map<BYTE, size_t>& symbolFrequency,pair<uintmax_t, BYTE>* WPL_Size = nullptr);
	//�ݹ�����һ�Ź�������
    static void DestroyHuffmanTree(HuffmanNode* rootNode);
	//�ݹ������������,���Ը���һ���Ѿ����ڵĹ��������õ� ����-���볤�ȱ� �� ����-�����(��ѡ)
    static void EncodeHuffmanTree(vector<pair<BYTE, BYTE>>& codeLength,HuffmanNode* rootNode,string code = "", unordered_map<BYTE, string>* symbolCode = nullptr);
	//���� ����-Ƶ�ʱ� �� ����-���볤�ȱ� ���WPL��ѹ�������ܴ�С
	static void GetWPL( unordered_map<BYTE, size_t>& symbolFrequency,const vector<pair<BYTE, BYTE>>& codeLength, pair<uintmax_t, BYTE>& WPL_Size);
	//���� ����-���볤�ȱ� ���� ����-��ʽ�����
	static void GetNormalSymbolCode(vector<pair<BYTE, BYTE>>& codeLength, unordered_map<BYTE, string>& symbolCode);

};