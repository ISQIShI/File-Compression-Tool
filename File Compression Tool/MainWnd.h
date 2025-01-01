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
	fourthPopUpMenuOptionID_ButtonUnpack_ToolBar,
	//文件列表
	fileListID,
};

enum class FileListColumnID :unsigned char {
	columnNameID,
	columnTypeID,
	columnOriginalSizeID,
    columnZipSizeID,
};

//主窗口类
class MainWnd :public MyWnds {
	//采用单例设计理念，主窗口类只能实例化一个对象
	MainWnd(){
		wndWidth = 0.53 * maxScreenWidth;
		wndHeight = 0.62 * maxScreenHeight;
		if (wndWidth < 960) wndWidth = 960;
		if (wndHeight < 540)wndHeight = 540;
	} //禁止外部构造
	~MainWnd()= default; //禁止外部析构
	MainWnd(const MainWnd& mainWnd) = delete;//禁止外部拷贝构造
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//禁止外部赋值操作

	//----------------------子类重写的函数--------------------------------
	//注册窗口类
	ATOM RegisterWndClass() override;
	//创建窗口
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_NOTIFY_WndProc() override;
	//处理窗口大小/位置发生改变后收到的消息，在这里限制了窗口的最小尺寸
	LRESULT WM_WINDOWPOSCHANGING_WndProc() override;
	//处理设定好窗口大小/位置后的反馈信息，在这里用于同步调整属于该窗口的所有子窗口/控件的位置或大小，实现自适应变化
	LRESULT WM_WINDOWPOSCHANGED_WndProc() override;
	//创建窗口时顺带执行的操作
	LRESULT WM_CREATE_WndProc() override;
	//关闭窗口
	LRESULT WM_CLOSE_WndProc() override;
	//销毁窗口
	LRESULT WM_DESTROY_WndProc() override;
	//重写的枚举子窗口过程函数
	BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam) override;

	void ClickPreviewButton();

public:
	static MainWnd& GetMainWnd() {
		static MainWnd mainWnd;
		return mainWnd;
	}
};
