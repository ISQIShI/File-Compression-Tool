#pragma once
#include"MainWnd.h"
#include"FileInfo.h"
#include"ZipFileInfo.h"
#include <stack>

class UnpackFunc :public MyWnds{
	//�洢ѹ�����ڲ��ļ���Ϣ
	vector<vector<InternalFileInfo>>* internalFileArr = new vector<vector<InternalFileInfo>>;
	//�洢ѹ��������Ϣ
	ZipFileInfo* zipFile = new ZipFileInfo;
	//--------------------------ִ�н�ѹ�����ܵĺ���----------------------------

	//--------------------------������д�Ĵ��ں���----------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass() override;
	//��������
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_PAINT_WndProc() override;
	//��������ʱ˳��ִ�еĲ���
	LRESULT WM_CREATE_WndProc() override;
	//�رմ���
	LRESULT WM_CLOSE_WndProc() override;
	//���ٴ���
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================

	//���õ���������������ֻ��ʵ����һ������
	UnpackFunc() {
		internalFileArr->resize(1);
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;

	} //��ֹ�ⲿ����
	~UnpackFunc() {
		//������Դ
		if (internalFileArr)delete internalFileArr;
		if (zipFile)delete zipFile;
	}; //��ֹ�ⲿ����
	UnpackFunc(const UnpackFunc& mainWnd) = delete;//��ֹ�ⲿ��������
	const UnpackFunc& operator=(const UnpackFunc& mainWnd) = delete;//��ֹ�ⲿ��ֵ����
public:
	//��ǰ�ļ������
	size_t folderIndex = 0;
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