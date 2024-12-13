#pragma once
#include"MyWnds.h"

enum MainWndChildID :unsigned char {
	//工具栏
	toolBarID = 1,
	//工具栏的按钮
	buttonOpenID_ToolBar,
	buttonPreviewID_ToolBar,
	buttonZipID_ToolBar,
	buttonUnpackID_ToolBar,
	buttonSetID_ToolBar,
	//工具栏的按钮的下拉菜单的选项
	firstPopUpMenuOptionID_ButtonUnpack_ToolBar,
	secondPopUpMenuOptionID_ButtonUnpack_ToolBar,
	thirdPopUpMenuOptionID_ButtonUnpack_ToolBar,
	//文件列表
	fileListID,
};

enum FileListColumnID :unsigned char {
	columnNameID,
	columnTypeID
};

//主窗口类
class MainWnd :public MyWnds {
	//采用单例设计理念，主窗口类只能实例化一个对象
	MainWnd(){} //禁止外部构造
	~MainWnd(){} //禁止外部析构
	MainWnd(const MainWnd& mainWnd) = delete;//禁止外部拷贝构造
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//禁止外部赋值操作

	//----------------------子类重写的函数--------------------------------
	//注册窗口类
	ATOM RegisterWndClass();
	//创建窗口
	HWND CreateWnd();

	LRESULT WM_COMMAND_WndProc();
	LRESULT WM_NOTIFY_WndProc();
	//处理窗口大小/位置发生改变后收到的消息，在这里限制了窗口的最小尺寸
	LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//处理设定好窗口大小/位置后的反馈信息，在这里用于同步调整属于该窗口的所有子窗口/控件的位置或大小，实现自适应变化
	LRESULT WM_WINDOWPOSCHANGED_WndProc();
	LRESULT WM_CREATE_WndProc();
	LRESULT WM_CLOSE_WndProc();
	LRESULT WM_DESTROY_WndProc();
	//重写的枚举子窗口过程函数
	BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	static MainWnd& GetMainWnd() {
		static MainWnd mainWnd;
		return mainWnd;
	}
};
