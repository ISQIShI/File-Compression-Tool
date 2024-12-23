#pragma once
#include"MainWnd.h"
#include<vector>
#include<filesystem>
using namespace std;
using namespace filesystem;

enum class ZipFuncWndChildID :unsigned char {
	buttonSelectFileID,
	buttonSelectFolderID,
	buttonDeleteID,
	selectedFileListID,
	buttonStartID,
	buttonCancelID,
	buttonBrowseID,
	editFileNameID
};
enum class selectedFileListColumnID :unsigned char {
	columnNameID,
	columnTypeID,
	columnSizeID,
	columnPathID
};

class ZipFunc :public MyWnds{
	//�洢ѡ����ļ�·��
	vector<path> filePathArr;
	//�洢ѹ������·��
	path zipFileName;
	//--------------------------ִ��ѹ�����ܵĺ���----------------------------
	void StartZip(bool openMultiThread = false);

	//--------------------------������д�Ĵ��ں���----------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass() override;
	//��������
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_NOTIFY_WndProc();
	LRESULT WM_PAINT_WndProc() override;
	//��������ʱ˳��ִ�еĲ���
	LRESULT WM_CREATE_WndProc() override;
	//�رմ���
	LRESULT WM_CLOSE_WndProc() override;
	//���ٴ���
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================

	//���õ�����������������ֻ��ʵ����һ������
	ZipFunc() {
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;

	} //��ֹ�ⲿ����
	~ZipFunc() = default; //��ֹ�ⲿ����
	ZipFunc(const ZipFunc& mainWnd) = delete;//��ֹ�ⲿ��������
	const ZipFunc& operator=(const ZipFunc& mainWnd) = delete;//��ֹ�ⲿ��ֵ����
public:
	static ZipFunc& GetZipFunc() {
		static ZipFunc zipFunc;
		return zipFunc;
	}
};