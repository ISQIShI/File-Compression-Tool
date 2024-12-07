#pragma once
#include<Windows.h>
#include<tchar.h>

//编译器使用Win XP的新式控件风格
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//压缩包后缀名
#define SUFFIX .ya

//抽象类MyWnds作为其他窗口类的统一父类
class MyWnds {
protected:
	//接受应用程序的当前实例的句柄
	static HINSTANCE hInstance;
	//屏幕最大宽度
	static int maxScreenWidth;
	//屏幕最大高度
	static int maxScreenHeight;
	//默认画刷
	HBRUSH defBrush = CreateSolidBrush(RGB(66, 149, 224));
	//窗口宽度
	int wndWidth = 0.53 * maxScreenWidth;
	//窗口高度
	int wndHeight = 0.62 * maxScreenHeight;

	//从窗口过程获取的参数
	HWND hwnd_WndProc;
	UINT uMsg_WndProc;
	WPARAM wParam_WndProc;
	LPARAM lParam_WndProc;

	~MyWnds(){
		DeleteObject(defBrush);//销毁临时画刷
	}
	//错误信息弹窗
	void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);

	//注册窗口类
	virtual ATOM RegisterWndClass() = 0;
	//创建窗口
	virtual HWND CreateWnd() = 0;
	//构建消息循环
	virtual WPARAM MessageLoop(const HWND& hwnd_IsDialogMessage, const HWND& hwnd_GetMessage =NULL);

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
	//处理窗口大小/位置发生改变后收到的消息，在这里限制了窗口的最小尺寸
	virtual LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//处理设定好窗口大小/位置后的反馈信息，在这里用于同步调整属于该窗口的所有子窗口/控件的位置或大小，实现自适应变化
	virtual LRESULT WM_WINDOWPOSCHANGED_WndProc();
	//创建窗口时发送的消息
	virtual LRESULT WM_CREATE_WndProc();
	//点击关闭按钮
	virtual LRESULT WM_CLOSE_WndProc();
	//销毁窗口
	virtual LRESULT WM_DESTROY_WndProc();
	//----------------------------------------------------------
	
	//枚举子窗口时调用的回调函数
	static BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	//获取应用程序的当前实例的句柄
	static HINSTANCE GethInstance() { return hInstance; }
	//设定应用程序的当前实例的句柄
	static void SethInstance(HINSTANCE hIn) { hInstance = hIn; }
	//创建窗口
	virtual WPARAM Wnd(bool needMessageLoop = false);
};


//主窗口类
class MainWnd :public MyWnds {
	//采用单例设计理念，主窗口类只能实例化一个对象
	MainWnd(){} //禁止外部构造
	~MainWnd() {} //禁止外部析构
	MainWnd(const MainWnd& mainWnd) = delete;//禁止外部拷贝构造
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//禁止外部赋值操作

	//----------------------子类重写的函数--------------------------------
	//注册窗口类
	ATOM RegisterWndClass();
	//创建窗口
	HWND CreateWnd();

	LRESULT WM_CREATE_WndProc();

public:
	static MainWnd& GetMainWnd() { 
		static MainWnd mainWnd;
		return mainWnd; 
	}
};
