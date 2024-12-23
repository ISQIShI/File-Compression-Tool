#pragma once
#include<Windows.h>
#include<CommCtrl.h>
#include<tchar.h>

//编译器使用Win XP的新式控件风格
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//压缩包后缀名
#define SUFFIX ".ya"

//抽象类MyWnds作为其他窗口类的统一父类
class MyWnds {
protected:
	//接受应用程序的当前实例的句柄
	static HINSTANCE hInstance;
	//屏幕最大宽度
	static int maxScreenWidth;
	//屏幕最大高度
	static int maxScreenHeight;
	//临时对象资源
	HGDIOBJ tempObject[10] = {};
	//长期对象资源
	HGDIOBJ lTSObject[10] = {};
	//窗口宽度
	UINT wndWidth = NULL;
	//窗口高度
	UINT wndHeight = NULL;
	//模态对话框参数
	HWND isModalDialog = NULL;
	//从窗口过程获取的参数
	HWND hwnd_WndProc = NULL;
	UINT uMsg_WndProc = NULL;
	WPARAM wParam_WndProc = NULL;
	LPARAM lParam_WndProc = NULL;
	
	//错误信息弹窗
	void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);
	//测试信息弹窗
	void TestMessageBox(const HWND& hwnd = NULL, const TCHAR* text = _T("测试"), const TCHAR* title = _T("测试"),UINT type = MB_OK);
	//注册窗口类
	virtual ATOM RegisterWndClass() = 0;
	//创建窗口
	virtual HWND CreateWnd() = 0;
	//构建消息循环
	virtual WPARAM MessageLoop(const HWND& hwnd_IsDialogMessage);

	//静态窗口过程
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//窗口过程
	virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//--------------------处理窗口消息的函数---------------------
	virtual LRESULT WM_COMMAND_WndProc();
	virtual LRESULT WM_NOTIFY_WndProc();
	//绘制static和edit(ES_READONLY)控件
	virtual LRESULT WM_CTLCOLORSTATIC_WndProc();
	//绘制窗口更新区域
	virtual LRESULT WM_PAINT_WndProc();
	//按下鼠标左键
	virtual LRESULT WM_LBUTTONDOWN_WndProc();
	//松开鼠标左键
	virtual LRESULT	WM_LBUTTONUP_WndProc();
	//处理窗口大小/位置发生改变后收到的消息
	virtual LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//处理设定好窗口大小/位置后的反馈信息
	virtual LRESULT WM_WINDOWPOSCHANGED_WndProc();
	//获取窗口新的宽度和高度
	virtual LRESULT WM_SIZE_WndProc();
	//创建窗口时发送的消息
	virtual LRESULT WM_CREATE_WndProc();
	//点击关闭按钮
	virtual LRESULT WM_CLOSE_WndProc();
	//销毁窗口
	virtual LRESULT WM_DESTROY_WndProc();
	//----------------------------------------------------------
	
	//枚举遍历子窗口时调用的回调函数
	static BOOL CALLBACK StaticEnumChildProc(HWND hwndChild, LPARAM lParam);
	virtual BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	//获取应用程序的当前实例的句柄
	static HINSTANCE GethInstance(){ return hInstance; }
	//设定应用程序的当前实例的句柄
	static void SethInstance(HINSTANCE hIn) { hInstance = hIn; }
	//获取窗口的句柄
	HWND GetWndHwnd() const { return hwnd_WndProc; }
	UINT GetWndWidth()const { return wndWidth; }
	UINT GetWndHeight()const { return wndHeight; }
	//创建窗口
	virtual WPARAM Wnd(bool needMessageLoop = false);
};
