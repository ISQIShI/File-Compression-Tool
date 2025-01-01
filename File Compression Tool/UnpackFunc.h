#pragma once
#include"MainWnd.h"
#include"FileInfo.h"
#include"ZipFileInfo.h"

enum class UnpackFuncWndChildID :unsigned char {
	buttonConfirmID,
	buttonCancelID,
	buttonBrowseID,
	radioButtonAllFilesID,
	radioButtonSelectedFilesID,
	editFileNameID
};

class UnpackFunc :public MyWnds{
	//存储压缩包内部文件信息
	vector<vector<InternalFileInfo>>* internalFileArr = new vector<vector<InternalFileInfo>>;
	//存储压缩包的信息
	ZipFileInfo* zipFile = new ZipFileInfo;
	//解压缩全部文件
	bool willUnpackAllFiles = true;
	//--------------------------执行解压缩功能的函数----------------------------
	void StartUnpack();
	//--------------------------子类重写的窗口函数----------------------------
	//注册窗口类
	ATOM RegisterWndClass() override;
	//创建窗口
	HWND CreateWnd() override;

	LRESULT WM_COMMAND_WndProc() override;
	LRESULT WM_PAINT_WndProc() override;
	//绘制单选框控件
	LRESULT WM_CTLCOLORSTATIC_WndProc()override;
	//创建窗口时顺带执行的操作
	LRESULT WM_CREATE_WndProc() override;
	//关闭窗口
	LRESULT WM_CLOSE_WndProc() override;
	//销毁窗口
	LRESULT WM_DESTROY_WndProc() override;
	//======================================================================

	void ClickConfirmButton();
	void ClickBrowseButton();

	//采用单例设计理念，窗口类只能实例化一个对象
	UnpackFunc() {
		internalFileArr->resize(1);
		isModalDialog = MainWnd::GetMainWnd().GetWndHwnd();
		wndWidth = 1000;
		wndHeight = 205;

	} //禁止外部构造
	~UnpackFunc() {
		//销毁资源
		if (internalFileArr)delete internalFileArr;
		if (zipFile)delete zipFile;
	}; //禁止外部析构
	UnpackFunc(const UnpackFunc& mainWnd) = delete;//禁止外部拷贝构造
	const UnpackFunc& operator=(const UnpackFunc& mainWnd) = delete;//禁止外部赋值操作
public:
	//当前文件夹深度
	size_t folderIndex = 0;
	//存储目标压缩路径
	path targetPath;
	static UnpackFunc& GetUnpackFunc() {
		static UnpackFunc unpackFunc;
		return unpackFunc;
	}
	//打开压缩包
	bool OpenZipFile();
	//获取压缩包内的文件信息
	void GetFileInfo();
	//存储压缩包内包含的文件的信息
	void StoreFileInfo(InternalFileInfo & internalFile,size_t folderID);
	//获取打开的压缩包文件路径
	const path& GetZipFilePath()const { return zipFile->zipFilePath; }
	//获取压缩包内部文件信息数组
    const vector<vector<InternalFileInfo>>& GetInternalFileInfo()const{ return *internalFileArr; }
};