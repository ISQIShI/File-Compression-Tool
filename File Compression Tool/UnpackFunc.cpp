#include "UnpackFunc.h"
#include <ShObjIdl_core.h>

ATOM UnpackFunc::RegisterWndClass()
{
	//实例化窗口类对象
	WNDCLASSEX unpackFuncWndClass = { 0 };
	unpackFuncWndClass.cbSize = sizeof(WNDCLASSEX);
	unpackFuncWndClass.style = CS_DBLCLKS;//类样式
	unpackFuncWndClass.lpfnWndProc = StaticWndProc;//窗口过程
	unpackFuncWndClass.hInstance = hInstance;//程序实例
	unpackFuncWndClass.hbrBackground = HBRUSH(6);//类背景画刷
	unpackFuncWndClass.lpszClassName = _T("unpackFuncWndClassName");//窗口类名
	unpackFuncWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//窗口图标
	return RegisterClassEx(&unpackFuncWndClass);
}

HWND UnpackFunc::CreateWnd()
{
	RECT rect;
	GetWindowRect(MainWnd::GetMainWnd().GetWndHwnd(), &rect);
	//创建窗口
	HWND unpackFuncWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("unpackFuncWndClassName"), _T("选择解压路径"), WS_TILED | WS_CAPTION | WS_SYSMENU,
		0.5 * (MainWnd::GetMainWnd().GetWndWidth() - wndWidth) + rect.left, 0.5 * (MainWnd::GetMainWnd().GetWndHeight() - wndHeight) + rect.top, wndWidth, wndHeight,
		isModalDialog, NULL, hInstance, this
	);
	//显示窗口
	if (unpackFuncWndHwnd) ShowWindow(unpackFuncWndHwnd, SW_SHOW);
	return unpackFuncWndHwnd;
}

LRESULT UnpackFunc::WM_COMMAND_WndProc()
{
	//其他未处理的消息使用默认窗口过程处理
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}


LRESULT UnpackFunc::WM_PAINT_WndProc()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd_WndProc, &ps);
	EndPaint(hwnd_WndProc, &ps);
	return 0;
}

LRESULT UnpackFunc::WM_CREATE_WndProc()
{
	
	return 0;
}

LRESULT UnpackFunc::WM_CLOSE_WndProc()
{
	DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
	return 0;
}

LRESULT UnpackFunc::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//发布WM_QUIT消息
	return 0;
}

bool UnpackFunc::OpenZipFile(){
	//初始化资源
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	IFileOpenDialog* fileOpenDialog = nullptr;
	//创建对话框实例
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
	if (!SUCCEEDED(hr))throw runtime_error("无法创建对话框");
	//设置过滤器
	COMDLG_FILTERSPEC filter[] = { { L"压缩包文件", L"*.ya" } };
	hr = fileOpenDialog->SetFileTypes(ARRAYSIZE(filter), filter);
	//显示对话框
	hr = fileOpenDialog->Show(MainWnd::GetMainWnd().GetWndHwnd());
	LPWSTR filePath = nullptr;
	//获取用户选择结果
	IShellItem* selectedItem;
	hr = fileOpenDialog->GetResult(&selectedItem);
	if (!SUCCEEDED(hr))
	{
		fileOpenDialog->Release();
		CoUninitialize();
		return false;
	}
	//获取文件路径
	selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	if (zipFile->zipFilePath == filePath)
	{
		selectedItem->Release();
		CoTaskMemFree(filePath);
		fileOpenDialog->Release();
		CoUninitialize();
		return false;
	}
	//初始化文件列表数组
	if (internalFileArr) {
		delete internalFileArr;
		internalFileArr = new vector<vector<InternalFileInfo>>;
		internalFileArr->resize(1);
		InternalFileInfo::folderAmount = 0;
	}
	//初始化文件夹深度
	folderIndex = 0;
	//初始化压缩文件信息
	if (zipFile) {
		delete zipFile; 
		zipFile = new ZipFileInfo;
	}
	
	zipFile->tempZipFilePath = zipFile->zipFilePath;
	zipFile->zipFilePath = filePath;
	//销毁资源
	selectedItem->Release();
	CoTaskMemFree(filePath);
	fileOpenDialog->Release();
	CoUninitialize();
	return true;
}

void UnpackFunc::GetFileInfo()
{
	//打开压缩包文件
	HANDLE fileHandle = CreateFile(zipFile->zipFilePath.c_str(), GENERIC_READ , FILE_SHARE_READ , nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("无法打开文件"));
	}
	//设置文件指针
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_BEGIN);
	//-----------------------------------读取压缩包首部--------------------------------------
	DWORD readbyte;
	//读取 标识签名
	char identification[2];
	ReadFile(fileHandle,identification, 2, &readbyte, nullptr);
    if (identification[0] != 'y' || identification[1] != 'a') {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("未知的文件格式"),false);
	}
	//读取 压缩包首部 长度
	unsigned short headerSize;
	ReadFile(fileHandle, &headerSize, 2, &readbyte, nullptr);
	zipFile->codeLength.resize((headerSize - 4) / 2);
	//读取 符号-编码长度 表
	for (size_t i = 0; i < zipFile->codeLength.size(); ++i)
	{
		ReadFile(fileHandle, &zipFile->codeLength[i], 2, &readbyte, nullptr);
	}
	zipFile->newFileSize += headerSize;
	//------------------------------------读取各文件首部-------------------------------------------
	InternalFileInfo internalFile;
	//首部大小
	unsigned short fileheaderSize;
	while (ReadFile(fileHandle, &fileheaderSize, 2, &readbyte, nullptr) && readbyte != 0) {
		//原始文件大小
		ReadFile(fileHandle, &internalFile.oldFileSize, 8, &readbyte, nullptr);
		//压缩后大小
		ReadFile(fileHandle, &internalFile.WPL_Size.first, 8, &readbyte, nullptr);
		//填充比特数
		ReadFile(fileHandle, &internalFile.WPL_Size.second, 1, &readbyte, nullptr);
		//文件名
		LPWSTR tempWCHAR = new WCHAR[fileheaderSize - 18]{};
		ReadFile(fileHandle, tempWCHAR, fileheaderSize - 19, &readbyte, nullptr);
		internalFile.fileName = tempWCHAR;
		internalFile.isFolder = false;
		zipFile->newFileSize += fileheaderSize;
		internalFile.fileOffset = zipFile->newFileSize;
		//移动文件指针
		tempL_I.QuadPart = internalFile.WPL_Size.first;
		SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_CURRENT);
		zipFile->newFileSize += internalFile.WPL_Size.first;
		StoreFileInfo(internalFile,0);
	}
	//进行校验
    if (zipFile->newFileSize != file_size(zipFile->zipFilePath)) {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("压缩包信息读取错误"),false);
	}
	//对存储压缩包内部文件信息的容器进行排序
	vector<InternalFileInfo>::iterator beginIt;
	for (size_t i = 0; i < internalFileArr->size();++i) {
		if (internalFileArr->at(i).size() > 1) {
			beginIt = internalFileArr->at(i).begin();
			if (i)++beginIt;
			sort(beginIt, internalFileArr->at(i).end(), [](const InternalFileInfo& a, const InternalFileInfo& b) {return a.isFolder >= b.isFolder; });
		}
	}
	CloseHandle(fileHandle);
}

void UnpackFunc::StoreFileInfo(InternalFileInfo& internalFile, size_t folderID)
{
	//准备存入文件夹
	if (internalFile.fileName.has_parent_path()) {
		//首先查找容器内是否已经存在该文件夹
		auto it = find_if(internalFileArr->at(folderID).begin(), internalFileArr->at(folderID).end(),
			[&internalFile](const InternalFileInfo& info){return info.fileName == *internalFile.fileName.begin(); });
		if (it == internalFileArr->at(folderID).end()) {//若不存在则插入
			InternalFileInfo& temp = internalFileArr->at(folderID).emplace_back(true, *internalFile.fileName.begin(),internalFile.oldFileSize,internalFile.WPL_Size,0 );
			internalFileArr->emplace_back();//创建新的一维容器
			internalFileArr->at(temp.folderID).emplace_back(true, _T("..(返回上一级文件夹)"), 0, pair<uintmax_t, BYTE>(0, 0), 0,folderID,true);
			folderID = temp.folderID;
		}
		else {//若存在则增加信息计数
			 it->oldFileSize += internalFile.oldFileSize;
			 it->WPL_Size.first += internalFile.WPL_Size.first;
			 folderID = it->folderID;
		}
		internalFile.fileName = relative(internalFile.fileName, *internalFile.fileName.begin());
		StoreFileInfo(internalFile, folderID);
	}
	else {//准备存入文件
		internalFileArr->at(folderID).emplace_back(false, internalFile.fileName, internalFile.oldFileSize, internalFile.WPL_Size, internalFile.fileOffset, folderID);
	}
}

