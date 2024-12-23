#pragma once
#include<unordered_map>
#include<queue>


class HuffmanNode {
public:
	BYTE symbol;//符号
	size_t frequency;//频率
	HuffmanNode* leftNode;//左节点
	HuffmanNode* rightNode;//右节点
	HuffmanNode(BYTE s, size_t f, HuffmanNode* left = nullptr, HuffmanNode* right = nullptr) :symbol(s), frequency(f), leftNode(left), rightNode(right) {}
};

//用于优先队列中哈夫曼节点比较的仿函数
struct HuffmanNodeCompare {
	bool operator()(const HuffmanNode* x, const HuffmanNode* y) const
    {
		//频率越小优先级越高，频率相同时，符号值越小优先级越高
		return (x->frequency > y->frequency) || ((x->frequency == y->frequency) && (x->symbol > y->symbol)); 
    }
};

class HuffmanCode {
public:
	/*--------各重要参数解释----[out]代表作为输出参数 [in]代表作为输入参数 [optional]代表作为可选参数------
	符号-频率表,用于存储文件中各个符号出现的频率
	unordered_map<BYTE, size_t> symbolFrequency; 
	[out]GetSymbolFrequency [in]BuildHuffmanTree 

	符号-编码表,用于存储各符号对应的编码
	unordered_map<BYTE, string> symbolCode;
	[out]GetNormalSymbolCode [out,optional]EncodeHuffmanTree 

	符号-编码长度表
	vector<pair<BYTE,BYTE>> codeLength;
	//priority_queue<pair<BYTE,BYTE>,vector<pair<BYTE, BYTE>>, CodeLengthCompare> codeLength;//已经弃用
	[out]EncodeHuffmanTree [in]GetNormalSymbolCode GetWPL

	一颗哈夫曼树的带权路径长度WPL,即使用该树进行编码压缩后数据的总长度,first参数为字节数，second参数为不足1字节填充的比特数
	如果使用分块处理，可用于计算块的大小
	pair<uintmax_t, BYTE>* WPL_Size
	*/
	
	//获取文件某区域中 符号-频率表 (后两个参数用于控制文件读取范围,可以实现大文件分块读取)
	static void GetSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency, const path& fileName, size_t fileOffset = 0, size_t fileMapSize = 0);
	//合并两个 符号-频率表 (表2合并到表1)
	static void MergeSymbolFrequency(unordered_map<BYTE, size_t>& symbolFrequency1, unordered_map<BYTE, size_t>& symbolFrequency2);
	//根据 符号-频率表 构建一颗哈夫曼树并返回根节点指针 输出WPL(即根据传入的符号-频率表进行压缩后数据的总长度)(可选)
	static HuffmanNode* BuildHuffmanTree(const unordered_map<BYTE, size_t>& symbolFrequency,pair<uintmax_t, BYTE>* WPL_Size = nullptr);
	//递归销毁一颗哈夫曼树
    static void DestroyHuffmanTree(HuffmanNode* rootNode);
	//递归遍历哈夫曼树,可以根据一颗已经存在的哈夫曼树得到 符号-编码长度表 和 符号-编码表(可选)
    static void EncodeHuffmanTree(vector<pair<BYTE, BYTE>>& codeLength,HuffmanNode* rootNode,string code = "", unordered_map<BYTE, string>* symbolCode = nullptr);
	//根据 符号-频率表 和 符号-编码长度表 求得WPL即压缩数据总大小
	static void GetWPL( unordered_map<BYTE, size_t>& symbolFrequency,const vector<pair<BYTE, BYTE>>& codeLength, pair<uintmax_t, BYTE>& WPL_Size);
	//根据 符号-编码长度表 构建 符号-范式编码表
	static void GetNormalSymbolCode(vector<pair<BYTE, BYTE>>& codeLength, unordered_map<BYTE, string>& symbolCode);

};