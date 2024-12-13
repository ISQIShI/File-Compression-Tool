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

enum FileListColumnID :unsigned char {
	columnNameID,
	columnTypeID
};

//��������
class MainWnd :public MyWnds {
	//���õ�����������������ֻ��ʵ����һ������
	MainWnd(){} //��ֹ�ⲿ����
	~MainWnd(){} //��ֹ�ⲿ����
	MainWnd(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��������
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��ֵ����

	//----------------------������д�ĺ���--------------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass();
	//��������
	HWND CreateWnd();

	LRESULT WM_COMMAND_WndProc();
	LRESULT WM_NOTIFY_WndProc();
	//�����ڴ�С/λ�÷����ı���յ�����Ϣ�������������˴��ڵ���С�ߴ�
	LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//�����趨�ô��ڴ�С/λ�ú�ķ�����Ϣ������������ͬ���������ڸô��ڵ������Ӵ���/�ؼ���λ�û��С��ʵ������Ӧ�仯
	LRESULT WM_WINDOWPOSCHANGED_WndProc();
	LRESULT WM_CREATE_WndProc();
	LRESULT WM_CLOSE_WndProc();
	LRESULT WM_DESTROY_WndProc();
	//��д��ö���Ӵ��ڹ��̺���
	BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	static MainWnd& GetMainWnd() {
		static MainWnd mainWnd;
		return mainWnd;
	}
};
