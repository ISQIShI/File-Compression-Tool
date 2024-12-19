#include"ZipFunc.h"
#include <shobjidl.h> 

ATOM ZipFunc::RegisterWndClass()
{
	//实例化窗口类对象
	WNDCLASSEX zipFuncWndClass = { 0 };
	zipFuncWndClass.cbSize = sizeof(WNDCLASSEX);
	zipFuncWndClass.style = CS_DBLCLKS;//类样式
	zipFuncWndClass.lpfnWndProc = StaticWndProc;//窗口过程
	zipFuncWndClass.hInstance = hInstance;//程序实例
	zipFuncWndClass.hbrBackground = HBRUSH(6);//类背景画刷
	zipFuncWndClass.lpszClassName = _T("zipFuncWndClassName");//窗口类名
	zipFuncWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//窗口图标
	return RegisterClassEx(&zipFuncWndClass);
}

HWND ZipFunc::CreateWnd()
{
	RECT rect;
	GetWindowRect(MainWnd::GetMainWnd().GetWndHwnd(), &rect);
	//创建窗口
	HWND zipFuncWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("zipFuncWndClassName"), _T("新建压缩文件"), WS_TILED | WS_CAPTION | WS_SYSMENU,
		0.5 * (MainWnd::GetMainWnd().GetWndWidth() - wndWidth) + rect.left, 0.5 * (MainWnd::GetMainWnd().GetWndHeight() - wndHeight) + rect.top, wndWidth, wndHeight,
		isModalDialog, NULL, hInstance, this
	);
	//显示窗口
	if (zipFuncWndHwnd) ShowWindow(zipFuncWndHwnd, SW_SHOW);
	return zipFuncWndHwnd;
}


class FileDialogEventHandler : public IFileDialogEvents
{
public:
    // IUnknown 接口实现
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IUnknown || riid == IID_IFileDialogEvents)
        {
            *ppvObject = static_cast<IFileDialogEvents*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ref = InterlockedDecrement(&m_refCount);
        if (ref == 0)
            delete this;
        return ref;
    }

    // IFileDialogEvents 接口实现
    HRESULT STDMETHODCALLTYPE OnFileOk(IFileDialog* pfd) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnFolderChange(IFileDialog* pfd) { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnFolderChanging(IFileDialog* pfd, IShellItem* psiFolder) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnSelectionChange(IFileDialog* pfd)
    {
        IShellItem* pItem = nullptr;
        if (SUCCEEDED(pfd->GetCurrentSelection(&pItem)))
        {
            PWSTR pszPath = nullptr;
            if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
            {
                // 判断是否为文件夹
                DWORD attr = GetFileAttributes(pszPath);
                if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
                {
                    // 是文件夹，阻止打开
                    wprintf(L"Folder Selected: %s\n", pszPath);
                }
                else
                {
                    // 是文件
                    wprintf(L"File Selected: %s\n", pszPath);
                }
                CoTaskMemFree(pszPath);
            }
            pItem->Release();
        }
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE OnShareViolation(IFileDialog* pfd, IShellItem* psi, FDE_SHAREVIOLATION_RESPONSE* pResponse) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnTypeChange(IFileDialog* pfd) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnOverwrite(IFileDialog* pfd, IShellItem* psi, FDE_OVERWRITE_RESPONSE* pResponse) { return S_OK; }

    FileDialogEventHandler() : m_refCount(1) {}

private:
    LONG m_refCount;
};


LRESULT ZipFunc::WM_COMMAND_WndProc()
{
	//通知代码是点击按钮
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//根据点击的按钮不同执行不同功能
		switch (LOWORD(wParam_WndProc)) {
			//点击选择文件按钮
		case (int)ZipFuncWndChildID::buttonSelectFileID:
		{
			CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileDialog* ifileDialog = nullptr;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ifileDialog));
			if (!SUCCEEDED(hr))break;
			//配置文件对话框选项
            DWORD dword;
            hr = ifileDialog ->GetOptions(&dword);
			//同时支持文件夹选择和多选
            hr = ifileDialog ->SetOptions(dword  | FOS_ALLOWMULTISELECT); 

			 // 注册事件处理器
            IFileDialogEvents* pDialogEvents = new FileDialogEventHandler();
            DWORD dwCookie;
            hr = ifileDialog->Advise(pDialogEvents, &dwCookie);
			COMDLG_FILTERSPEC filter[] = {
			 { _T("All Files"), _T("*.*") },
			 { _T("Text Files"), _T("*.txt") },
			 // 其他文件类型...
			};	
			hr = ifileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
			//显示对话框
			hr = ifileDialog->Show(hwnd_WndProc);
            //使用IFileOpenDialog接口获取用户选择结果
            IShellItemArray* selectedIItemArray;
			IFileOpenDialog *ifileOpenDialog;

			hr = ifileDialog->QueryInterface(IID_PPV_ARGS(&ifileOpenDialog));
			hr = ifileOpenDialog->GetResults(&selectedIItemArray);
			if (!SUCCEEDED(hr))break;
			//获取选择的文件数
			DWORD fileCount = 0;
			selectedIItemArray->GetCount(&fileCount);
			//遍历所有选择的文件
			LPWSTR fillPath = nullptr;
			LVITEM item;
			std::wstring tempWString;
			TCHAR tempTCHAR[21];
			for (DWORD x = 0; x < fileCount; ++x) {
				IShellItem* selectedItem = nullptr;
				//获取索引为x的文件
				selectedIItemArray->GetItemAt(x,&selectedItem);
				//获取文件路径
				selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &fillPath);
				//插入文件路径数组
				filePathArr.push_back(path(fillPath));
				//插入文件列表
				item.mask = LVIF_TEXT;
				item.iItem = x;//项索引
				item.iSubItem = 0;//子项索引
				tempWString = filePathArr[x].filename().wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//名称
				ListView_InsertItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);
				
				++item.iSubItem;//子项索引
				if (is_directory(filePathArr[x]))item.pszText = (LPTSTR)_T("文件夹");
				else item.pszText = (LPTSTR)_T("文件");
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);

				++item.iSubItem;//子项索引
				_stprintf_s(tempTCHAR, _T("%llu"),file_size(filePathArr[x]));

				item.pszText = tempTCHAR;//大小
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);

				++item.iSubItem;//子项索引
				tempWString = filePathArr[x].wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//路径
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);
				
				//销毁资源
				selectedItem->Release();
			}
			// 取消注册事件
			ifileDialog->Unadvise(dwCookie);
            pDialogEvents->Release();
			//销毁资源
			selectedIItemArray->Release();
			ifileOpenDialog->Release();
			ifileDialog->Release();
			CoUninitialize();
			break;
		}
		case (int)ZipFuncWndChildID::buttonStartID:
		{
			break;
		}
		case (int)ZipFuncWndChildID::buttonCancelID:
		{
			break;
		}
		}
	}
	else {
		//其他未处理的消息使用默认窗口过程处理
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT ZipFunc::WM_CREATE_WndProc()
{
	//创建字体
	defFont = CreateFont(
		23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("楷体")); // 创建一个 楷体 字体，23px 大小
	//创建按钮
	CreateWindowEx(
		0, WC_BUTTON, _T("选择文件"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		175, 510, 130, 50,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFileID), MyWnds::hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("开始压缩"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		435,510 ,130 ,50 ,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonStartID), MyWnds::hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("取消"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		695, 510, 130, 50,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonCancelID), MyWnds::hInstance, this
	);
	//设置字体
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFileID), WM_SETFONT, (WPARAM)defFont, TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonStartID), WM_SETFONT, (WPARAM)defFont, TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)defFont, TRUE);
	//创建已选择文件列表
	HWND selectedFileListHwnd = CreateWindowEx(
		0, WC_LISTVIEW, _T("已选择的文件"), WS_CHILD | WS_VISIBLE |WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS,
		50, 20, 900, 450,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::selectedFileListID), MyWnds::hInstance, this
	);
	if (!selectedFileListHwnd)ErrorMessageBox(hwnd_WndProc, _T("创建文件列表框selectedFileList失败"));
	//设置扩展样式
	ListView_SetExtendedListViewStyle(selectedFileListHwnd, LVS_EX_COLUMNSNAPPOINTS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
	//插入列
	LVCOLUMN column = { 0 };
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_MINWIDTH;
	column.cx = 260;
	column.pszText = (LPTSTR)_T("名称");
	column.iSubItem = (int)selectedFileListColumnID::columnNameID;
	column.cxMin = 260;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnNameID, &column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 90;
	column.pszText = (LPTSTR)_T("类型");
	column.iSubItem = (int)selectedFileListColumnID::columnTypeID;
	column.cxMin = 90;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnTypeID, &column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 100;
	column.pszText = (LPTSTR)_T("大小");
	column.iSubItem = (int)selectedFileListColumnID::columnSizeID;
	column.cxMin = 100;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnSizeID, &column);

	column.fmt = LVCFMT_LEFT; 
	column.cx = 440;
	column.pszText = (LPTSTR)_T("文件路径");
	column.iSubItem = (int)selectedFileListColumnID::columnPathID;
	column.cxMin = 440;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnPathID, &column);
	return 0;
}



LRESULT ZipFunc::WM_CLOSE_WndProc()
{
	DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
	return 0;
}

LRESULT ZipFunc::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//发布WM_QUIT消息
	return 0;
}


