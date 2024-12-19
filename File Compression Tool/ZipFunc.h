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
	//采用单例设计理念，主窗口类只能实例化一个对象
	ZipFunc() {
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;
	} //禁止外部构造
	~ZipFunc() = default; //禁止外部析构
	ZipFunc(const ZipFunc& mainWnd) = delete;//禁止外部拷贝构造
	const ZipFunc& operator=(const ZipFunc& mainWnd) = delete;//禁止外部赋值操作
	//----------------------子类重写的函数--------------------------------
	//注册窗口类
	ATOM RegisterWndClass() override;
	//创建窗口
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	//创建窗口时顺带执行的操作
	LRESULT WM_CREATE_WndProc() override;
	//关闭窗口
	LRESULT WM_CLOSE_WndProc() override;
	//销毁窗口
	LRESULT WM_DESTROY_WndProc() override;

public:
	static ZipFunc& GetZipFunc() {
		static ZipFunc zipFunc;
		return zipFunc;
	}
};