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
	//存储选择的文件路径
	vector<path> filePathArr;
	//存储压缩包的路径
	path zipFileName;
	//--------------------------执行压缩功能的函数----------------------------
	void StartZip(bool openMultiThread = false);

	//--------------------------子类重写的窗口函数----------------------------
	//注册窗口类
	ATOM RegisterWndClass() override;
	//创建窗口
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_NOTIFY_WndProc();
	LRESULT WM_PAINT_WndProc() override;
	//创建窗口时顺带执行的操作
	LRESULT WM_CREATE_WndProc() override;
	//关闭窗口
	LRESULT WM_CLOSE_WndProc() override;
	//销毁窗口
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================

	//采用单例设计理念，主窗口类只能实例化一个对象
	ZipFunc() {
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;

	} //禁止外部构造
	~ZipFunc() = default; //禁止外部析构
	ZipFunc(const ZipFunc& mainWnd) = delete;//禁止外部拷贝构造
	const ZipFunc& operator=(const ZipFunc& mainWnd) = delete;//禁止外部赋值操作
public:
	static ZipFunc& GetZipFunc() {
		static ZipFunc zipFunc;
		return zipFunc;
	}
};