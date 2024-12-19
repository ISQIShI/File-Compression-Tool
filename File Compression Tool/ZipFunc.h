#pragma once
#include"MainWnd.h"
#include<vector>
#include<filesystem>
using std::vector;
using namespace std::filesystem;

enum class ZipFuncWndChildID :unsigned char {
	buttonSelectFileID,
	selectedFileListID,
	buttonStartID,
	buttonCancelID
};
enum class selectedFileListColumnID :unsigned char {
	columnNameID,
	columnTypeID,
	columnSizeID,
	columnPathID
};

class ZipFunc :public MyWnds{
	vector<path> filePathArr;
	//���õ�����������������ֻ��ʵ����һ������
	ZipFunc() {
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;
	} //��ֹ�ⲿ����
	~ZipFunc() = default; //��ֹ�ⲿ����
	ZipFunc(const ZipFunc& mainWnd) = delete;//��ֹ�ⲿ��������
	const ZipFunc& operator=(const ZipFunc& mainWnd) = delete;//��ֹ�ⲿ��ֵ����
	//----------------------������д�ĺ���--------------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass() override;
	//��������
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	//��������ʱ˳��ִ�еĲ���
	LRESULT WM_CREATE_WndProc() override;
	//�رմ���
	LRESULT WM_CLOSE_WndProc() override;
	//���ٴ���
	LRESULT WM_DESTROY_WndProc() override;

public:
	static ZipFunc& GetZipFunc() {
		static ZipFunc zipFunc;
		return zipFunc;
	}
};