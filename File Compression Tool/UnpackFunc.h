#pragma once
#include"FileInfo.h"
#include"MainWnd.h"
#include"ZipFileInfo.h"
#include <bitset>

class ThreadPool;
enum class UnpackFuncWndChildID :unsigned char {
	buttonConfirmID,
	buttonCancelID,
	buttonBrowseID,
	radioButtonAllFilesID,
	radioButtonSelectedFilesID,
	checkBoxIntelligentUnpackID,
	editFileNameID
};

class UnpackFunc :public MyWnds{
	friend class MainWnd;
	//�洢ѹ�����ڲ��ļ���Ϣ
	vector<vector<InternalFileInfo>>* internalFileArr = new vector<vector<InternalFileInfo>>;
	//�洢ѹ��������Ϣ
	ZipFileInfo* zipFile = new ZipFileInfo;
	//���볤��-����-���ű�
	vector<std::pair<BYTE, unordered_map<std::bitset<256>, BYTE>>>* codeSymbol = new vector<std::pair<BYTE, unordered_map<std::bitset<256>, BYTE>>>;
	//��ѹ��ȫ���ļ�
	bool willUnpackAllFiles = true;
	//��ǰ�ļ������
	size_t folderIndex = 0;
	//�����ܽ�ѹ
	bool openIntelligentUnpack = true;
	//�洢Ŀ��ѹ��·��
	path targetPath;
	
	//--------------------------ִ�н�ѹ�����ܵĺ���----------------------------
	void StartUnpack();
	void UnpackObject(const InternalFileInfo & internalFile,ThreadPool & threadPool);
	void UnpackFile(const InternalFileInfo& internalFile);

	template <size_t N>
	int compareBitset(const std::bitset<N>& bitset1, const std::bitset<N>& bitset2)
	{
		for (int i = N - 1; i >= 0; --i)
		{
			if (bitset1[i] < bitset2[i]) return -1;//bitset1 < bitset2
			if (bitset1[i] > bitset2[i]) return 1;//bitset1 > bitset2
		}
		return 0;//bitset1 == bitset2
	}
	//--------------------------������д�Ĵ��ں���----------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass() override;
	//��������
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_PAINT_WndProc() override;
	//���Ƶ�ѡ��ؼ�
	LRESULT WM_CTLCOLORSTATIC_WndProc()override;
	//��������ʱ˳��ִ�еĲ���
	LRESULT WM_CREATE_WndProc() override;
	//�رմ���
	LRESULT WM_CLOSE_WndProc() override;
	//���ٴ���
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================

	void ClickConfirmButton();
	void ClickBrowseButton();

	//���õ���������������ֻ��ʵ����һ������
	UnpackFunc() {
		internalFileArr->resize(1);
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 205;

	} //��ֹ�ⲿ����
	~UnpackFunc() {
		//������Դ
		if (internalFileArr)delete internalFileArr;
		if (zipFile)delete zipFile;
		if(codeSymbol)delete codeSymbol;
	}; //��ֹ�ⲿ����
	UnpackFunc(const UnpackFunc& mainWnd) = delete;//��ֹ�ⲿ��������
	const UnpackFunc& operator=(const UnpackFunc& mainWnd) = delete;//��ֹ�ⲿ��ֵ����
public:
	static UnpackFunc& GetUnpackFunc() {
		static UnpackFunc unpackFunc;
		return unpackFunc;
	}
	//��ѹ����
	bool OpenZipFile();
	//��ȡѹ�����ڵ��ļ���Ϣ
	void GetFileInfo();
	//�洢ѹ�����ڰ������ļ�����Ϣ
	void StoreFileInfo(InternalFileInfo & internalFile,size_t folderID);
	//��ȡ�򿪵�ѹ�����ļ�·��
	const path& GetZipFilePath()const { return zipFile->zipFilePath; }
	//��ȡѹ�����ڲ��ļ���Ϣ����
    const vector<vector<InternalFileInfo>>& GetInternalFileInfo()const{ return *internalFileArr; }
};