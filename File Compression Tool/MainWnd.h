#pragma once
#include"MyWnds.h"

enum MainWndChildID :unsigned char {
	//������
	toolBarID = 1,
	//�������İ�ť
	buttonOpenID_ToolBar,
	buttonPreviewID_ToolBar,
	buttonZipID_ToolBar,
	buttonUnpackID_ToolBar,
	buttonSetID_ToolBar,
	//�������İ�ť�������˵���ѡ��
	firstPopUpMenuOptionID_ButtonUnpack_ToolBar,
	secondPopUpMenuOptionID_ButtonUnpack_ToolBar,
	thirdPopUpMenuOptionID_ButtonUnpack_ToolBar,
	//�ļ��б�
	fileListID,
};

enum class FileListColumnID :unsigned char {
	columnNameID,
	columnTypeID
};

//��������
class MainWnd :public MyWnds {
	//���õ�����������������ֻ��ʵ����һ������
	MainWnd(){
		wndWidth = 0.53 * maxScreenWidth;
		wndHeight = 0.62 * maxScreenHeight;
		if (wndWidth < 960) wndWidth = 960;
		if (wndHeight < 540)wndHeight = 540;
	} //��ֹ�ⲿ����
	~MainWnd()= default; //��ֹ�ⲿ����
	MainWnd(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��������
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��ֵ����

	//----------------------������д�ĺ���--------------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass() override;
	//��������
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_NOTIFY_WndProc() override;
	//�����ڴ�С/λ�÷����ı���յ�����Ϣ�������������˴��ڵ���С�ߴ�
	LRESULT WM_WINDOWPOSCHANGING_WndProc() override;
	//�����趨�ô��ڴ�С/λ�ú�ķ�����Ϣ������������ͬ���������ڸô��ڵ������Ӵ���/�ؼ���λ�û��С��ʵ������Ӧ�仯
	LRESULT WM_WINDOWPOSCHANGED_WndProc() override;
	//��������ʱ˳��ִ�еĲ���
	LRESULT WM_CREATE_WndProc() override;
	//�رմ���
	LRESULT WM_CLOSE_WndProc() override;
	//���ٴ���
	LRESULT WM_DESTROY_WndProc() override;
	//��д��ö���Ӵ��ڹ��̺���
	BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) override;

public:
	static MainWnd& GetMainWnd() {
		static MainWnd mainWnd;
		return mainWnd;
	}
};
