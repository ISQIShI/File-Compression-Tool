#include"MyWnds.h"

HINSTANCE MyWnds::hInstance;
int MyWnds::maxScreenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
int MyWnds::maxScreenHeight = GetSystemMetrics(SM_CYMAXIMIZED);


void MyWnds::ErrorMessageBox(const HWND& hwnd,const TCHAR* msg,bool showErrorCode){
	//创建字符串用于接收错误代码
	TCHAR errorCode[20] = _T("");
	if (showErrorCode) _stprintf_s(errorCode, _T("\n错误代码:%lu"), GetLastError());
	//将msg内容与错误代码拼接起来
	size_t length = _tcslen(msg) + _tcslen(errorCode) + 1;
	TCHAR* finalMsg = new TCHAR[length];
	_stprintf_s(finalMsg, length ,_T("%s%s"), msg, errorCode);
	//弹出错误弹窗
	MessageBox(hwnd, finalMsg, _T("错误信息"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
	//销毁使用new开辟的空间
	delete[]finalMsg;
	//退出程序
	exit(GetLastError());
}

WPARAM MyWnds::MessageLoop(const HWND& hwnd_IsDialogMessage, const HWND& hwnd_GetMessage){
	MSG Msg = { 0 };
	BOOL bRet = 1;
	while ((bRet = GetMessage(&Msg, hwnd_GetMessage, 0, 0)) != 0) {
		if (bRet == -1) ErrorMessageBox(NULL, _T("消息检索失败"));
		else
		{
			if (!IsDialogMessage(hwnd_IsDialogMessage, &Msg)) //该函数处理键盘消息(如TAB切换控件焦点)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}
	return Msg.wParam;
}

LRESULT CALLBACK MyWnds::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	//创建父类(MyWnds)指针并置空，用于之后指向子类(继承自MyWnds的类)对象
	MyWnds* myWnds = nullptr;
	if (uMsg == WM_NCCREATE){
		//收到 WM_NCCREATE 消息时，lParam 是指向 CREATESTRUCT 结构的指针，该结构包含有关所创建的窗口的信息。
		CREATESTRUCT* temp = (CREATESTRUCT*)lParam;
		/*
		CREATESTRUCT 结构中的 lpCreateParams 成员变量存储的是调用 CreateWindowEx 创建窗口时传入的 lpParam 参数值
		我传入的是子类(继承自MyWnds的类)对象调用普通成员函数 CreateWnd 时隐含的 this 指针
		因而它实际是指向创建窗口的子类(继承自MyWnds的类)对象的指针，可以被强转为父类(MyWnds)指针并被父类(MyWnds)指针接收
		*/
		myWnds = (MyWnds*)temp->lpCreateParams;
		//构建子类对象指针与窗口句柄之间的绑定关系
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)myWnds);
	}
	else{
		//通过窗口句柄查找到创建该窗口的子类(继承自MyWnds的类)对象的指针，并用父类(MyWnds)指针指向子类(继承自MyWnds的类)对象
		myWnds = (MyWnds*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (myWnds){
		return myWnds->WndProc(hwnd, uMsg, wParam, lParam);
	}
	else{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK MyWnds::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	hwnd_WndProc = hwnd;
	uMsg_WndProc = uMsg;
	wParam_WndProc = wParam;
	lParam_WndProc = lParam;
	switch (uMsg) {
	case WM_COMMAND:
		return WM_COMMAND_WndProc();
	case WM_NOTIFY:
		return WM_NOTIFY_WndProc();
	case WM_CTLCOLORSTATIC://绘制static和edit(ES_READONLY)控件
		return WM_CTLCOLORSTATIC_WndProc();
	case WM_PAINT://绘制窗口更新区域
		return WM_PAINT_WndProc();
	case WM_WINDOWPOSCHANGING://正在更改窗口大小
		return WM_WINDOWPOSCHANGING_WndProc();
	case WM_WINDOWPOSCHANGED://窗口大小完成更改
		return WM_WINDOWPOSCHANGED_WndProc();
	case WM_CREATE:
		return WM_CREATE_WndProc();
	case WM_CLOSE://关闭窗口
		return WM_CLOSE_WndProc();
	case WM_DESTROY://销毁窗口
		return WM_DESTROY_WndProc();
	default://未自定义的其他消息
		return DefWindowProc(hwnd, uMsg, wParam, lParam);//调用默认窗口过程函数
	}
}

LRESULT MyWnds::WM_COMMAND_WndProc()
{
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_NOTIFY_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CTLCOLORSTATIC_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

//处理窗口消息的函数
LRESULT MyWnds::WM_PAINT_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_WINDOWPOSCHANGING_WndProc() {
	//设定临时指针接收附加信息
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	if (temp->cx < 960) {
		temp->cx = 960;
	}//如果新的窗口宽度小于960（像素），设定为960
	if (temp->cy < 540) {
		temp->cy = 540;
	}//如果新的窗口高度小于540（像素），设定为540
	return 0;
}

LRESULT MyWnds::WM_WINDOWPOSCHANGED_WndProc() {
	//设定临时指针接收附加信息
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	//创建lParam用于在枚举子窗口时给回调函数传参
	RECT rect = { wndWidth,wndHeight,temp->cx ,temp->cy };//使用RECT类型变量存储窗口变化前后的宽和高
	LPARAM lParam = (LPARAM)&rect;//使lParam存储rect地址值，从而之后强转为RECT指针获取数据
	EnumChildWindows(hwnd_WndProc, EnumChildProc, lParam);
	wndWidth = temp->cx;//更新窗口宽度
	wndHeight = temp->cy;//更新窗口高度
	return 0;
}

LRESULT MyWnds::WM_CREATE_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CLOSE_WndProc() {
	DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
	return 0;
}

LRESULT MyWnds::WM_DESTROY_WndProc() {
	PostQuitMessage(0);//发布WM_QUIT消息
	return 0;
}

BOOL CALLBACK MyWnds::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	//设定临时指针接收附加信息
	RECT* temp = (RECT*)lParam;
	//获取子窗口在屏幕上的坐标
	RECT rect;
	GetWindowRect(hwndChild, &rect);
	//将子窗口坐标转换为相对于父窗口的坐标
	POINT point = { rect.left,rect.top };
	ScreenToClient(GetParent(hwndChild), &point);
	//保持子窗口在父窗口中相对位置
	MoveWindow(hwndChild, double(point.x * temp->right) / temp->left , double(point.y * temp->bottom) / temp->top , rect.right - rect.left, rect.bottom - rect.top ,TRUE);
	return TRUE;
}

WPARAM MyWnds::Wnd(bool needMessageLoop){
	//首先直接创建窗口，防止是因为之前销毁窗口重新创建从而导致重复注册窗口类报错
	HWND hwnd = CreateWnd();
	if (hwnd) return 0;//窗口创建成功直接返回
	//窗口创建失败且错误代码为1407(窗口类不存在)时注册窗口类再次创建窗口
	if (GetLastError()==1407) {
		//注册窗口类
		if (!RegisterWndClass()) ErrorMessageBox(NULL, _T("注册窗口类失败"));
		//创建窗口
		hwnd = CreateWnd();
		if (!hwnd)ErrorMessageBox(NULL, _T("创建窗口失败"));
		//构建消息循环
		if (needMessageLoop)return MessageLoop(hwnd);
		else return 0;
	}
	else ErrorMessageBox(NULL, _T("创建窗口失败"));
}

//------------------------------------MainWnd----------------------------------------------
ATOM MainWnd::RegisterWndClass()
{
    //实例化窗口类对象---主窗口
    WNDCLASSEX mainWndClass = { 0 };
    mainWndClass.cbSize = sizeof(WNDCLASSEX);
    mainWndClass.style = CS_HREDRAW | CS_VREDRAW;//类样式
    mainWndClass.lpfnWndProc = StaticWndProc;//窗口过程
    mainWndClass.hInstance = hInstance;//程序实例
    mainWndClass.hbrBackground = defBrush;//类背景画刷
    mainWndClass.lpszClassName = _T("mainWndClassName");//窗口类名
    mainWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//窗口图标
    return RegisterClassEx(&mainWndClass);
}


HWND MainWnd::CreateWnd()
{
	//创建窗口---主窗口
	HWND mainHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT, _T("mainWndClassName"), _T("鸭一压"), WS_TILEDWINDOW,
		int(0.15 * wndWidth), int(0.15 * wndHeight), wndWidth, wndHeight,
		NULL, NULL, hInstance, this
	);
	//显示窗口
	if(mainHwnd) ShowWindow(mainHwnd, SW_SHOW);
	return mainHwnd;
}

LRESULT MainWnd::WM_CREATE_WndProc()
{
	return LRESULT();
}

