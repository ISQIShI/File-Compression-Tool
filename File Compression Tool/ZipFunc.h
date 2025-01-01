#pragma once
#include"MainWnd.h"
#include "FileInfo.h"
#include "ZipFileInfo.h"
#include<vector>

class ThreadPool;

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
	//存储选择的文件信息
	vector<SelectedFileInfo>* selectedFileArr = new vector<SelectedFileInfo>;
	//存储压缩包的信息
	ZipFileInfo *zipFile = new ZipFileInfo;
	//--------------------------执行压缩功能的函数----------------------------
	void StartZip(bool openMultiThread = false);
	void WriteSelectedFileData(SelectedFileInfo & selectedFile, ThreadPool & threadPool);
	//--------------------------子类重写的窗口函数----------------------------
	//注册窗口类
	ATOM RegisterWndClass() override;
	//创建窗口
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_NOTIFY_WndProc() override;
	LRESULT WM_PAINT_WndProc() override;
	//创建窗口时顺带执行的操作
	LRESULT WM_CREATE_WndProc() override;
	//关闭窗口
	LRESULT WM_CLOSE_WndProc() override;
	//销毁窗口
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================
	void ClickSelectFileButton();
	void ClickDeleteButton();
	void ClickStartButton();
	void ClickBrowseButton();

	//采用单例设计理念，窗口类只能实例化一个对象
	ZipFunc() {
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 650;

	} //禁止外部构造
	~ZipFunc() {
		//销毁资源
		if (selectedFileArr)delete selectedFileArr;
		if (zipFile)delete zipFile;
	}; //禁止外部析构
	ZipFunc(const ZipFunc& mainWnd) = delete;//禁止外部拷贝构造
	const ZipFunc& operator=(const ZipFunc& mainWnd) = delete;//禁止外部赋值操作
public:
	static ZipFunc& GetZipFunc() {
		static ZipFunc zipFunc;
		return zipFunc;
	}
};