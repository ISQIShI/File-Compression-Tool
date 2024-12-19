#include"ZipFunc.h"
#include <shobjidl.h> 

ATOM ZipFunc::RegisterWndClass()
{
	//ʵ�������������
	WNDCLASSEX zipFuncWndClass = { 0 };
	zipFuncWndClass.cbSize = sizeof(WNDCLASSEX);
	zipFuncWndClass.style = CS_DBLCLKS;//����ʽ
	zipFuncWndClass.lpfnWndProc = StaticWndProc;//���ڹ���
	zipFuncWndClass.hInstance = hInstance;//����ʵ��
	zipFuncWndClass.hbrBackground = HBRUSH(6);//�౳����ˢ
	zipFuncWndClass.lpszClassName = _T("zipFuncWndClassName");//��������
	zipFuncWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//����ͼ��
	return RegisterClassEx(&zipFuncWndClass);
}

HWND ZipFunc::CreateWnd()
{
	RECT rect;
	GetWindowRect(MainWnd::GetMainWnd().GetWndHwnd(), &rect);
	//��������
	HWND zipFuncWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("zipFuncWndClassName"), _T("�½�ѹ���ļ�"), WS_TILED | WS_CAPTION | WS_SYSMENU,
		0.5 * (MainWnd::GetMainWnd().GetWndWidth() - wndWidth) + rect.left, 0.5 * (MainWnd::GetMainWnd().GetWndHeight() - wndHeight) + rect.top, wndWidth, wndHeight,
		isModalDialog, NULL, hInstance, this
	);
	//��ʾ����
	if (zipFuncWndHwnd) ShowWindow(zipFuncWndHwnd, SW_SHOW);
	return zipFuncWndHwnd;
}


class FileDialogEventHandler : public IFileDialogEvents
{
public:
    // IUnknown �ӿ�ʵ��
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

    // IFileDialogEvents �ӿ�ʵ��
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
                // �ж��Ƿ�Ϊ�ļ���
                DWORD attr = GetFileAttributes(pszPath);
                if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
                {
                    // ���ļ��У���ֹ��
                    wprintf(L"Folder Selected: %s\n", pszPath);
                }
                else
                {
                    // ���ļ�
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
	//֪ͨ�����ǵ����ť
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//���ݵ���İ�ť��ִͬ�в�ͬ����
		switch (LOWORD(wParam_WndProc)) {
			//���ѡ���ļ���ť
		case (int)ZipFuncWndChildID::buttonSelectFileID:
		{
			CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileDialog* ifileDialog = nullptr;
			HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ifileDialog));
			if (!SUCCEEDED(hr))break;
			//�����ļ��Ի���ѡ��
            DWORD dword;
            hr = ifileDialog ->GetOptions(&dword);
			//ͬʱ֧���ļ���ѡ��Ͷ�ѡ
            hr = ifileDialog ->SetOptions(dword  | FOS_ALLOWMULTISELECT); 

			 // ע���¼�������
            IFileDialogEvents* pDialogEvents = new FileDialogEventHandler();
            DWORD dwCookie;
            hr = ifileDialog->Advise(pDialogEvents, &dwCookie);
			COMDLG_FILTERSPEC filter[] = {
			 { _T("All Files"), _T("*.*") },
			 { _T("Text Files"), _T("*.txt") },
			 // �����ļ�����...
			};	
			hr = ifileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
			//��ʾ�Ի���
			hr = ifileDialog->Show(hwnd_WndProc);
            //ʹ��IFileOpenDialog�ӿڻ�ȡ�û�ѡ����
            IShellItemArray* selectedIItemArray;
			IFileOpenDialog *ifileOpenDialog;

			hr = ifileDialog->QueryInterface(IID_PPV_ARGS(&ifileOpenDialog));
			hr = ifileOpenDialog->GetResults(&selectedIItemArray);
			if (!SUCCEEDED(hr))break;
			//��ȡѡ����ļ���
			DWORD fileCount = 0;
			selectedIItemArray->GetCount(&fileCount);
			//��������ѡ����ļ�
			LPWSTR fillPath = nullptr;
			LVITEM item;
			std::wstring tempWString;
			TCHAR tempTCHAR[21];
			for (DWORD x = 0; x < fileCount; ++x) {
				IShellItem* selectedItem = nullptr;
				//��ȡ����Ϊx���ļ�
				selectedIItemArray->GetItemAt(x,&selectedItem);
				//��ȡ�ļ�·��
				selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &fillPath);
				//�����ļ�·������
				filePathArr.push_back(path(fillPath));
				//�����ļ��б�
				item.mask = LVIF_TEXT;
				item.iItem = x;//������
				item.iSubItem = 0;//��������
				tempWString = filePathArr[x].filename().wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//����
				ListView_InsertItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);
				
				++item.iSubItem;//��������
				if (is_directory(filePathArr[x]))item.pszText = (LPTSTR)_T("�ļ���");
				else item.pszText = (LPTSTR)_T("�ļ�");
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);

				++item.iSubItem;//��������
				_stprintf_s(tempTCHAR, _T("%llu"),file_size(filePathArr[x]));

				item.pszText = tempTCHAR;//��С
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);

				++item.iSubItem;//��������
				tempWString = filePathArr[x].wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//·��
				ListView_SetItem(GetDlgItem(hwnd_WndProc,(int) ZipFuncWndChildID::selectedFileListID),&item);
				
				//������Դ
				selectedItem->Release();
			}
			// ȡ��ע���¼�
			ifileDialog->Unadvise(dwCookie);
            pDialogEvents->Release();
			//������Դ
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
		//����δ�������Ϣʹ��Ĭ�ϴ��ڹ��̴���
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT ZipFunc::WM_CREATE_WndProc()
{
	//��������
	defFont = CreateFont(
		23, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("����")); // ����һ�� ���� ���壬23px ��С
	//������ť
	CreateWindowEx(
		0, WC_BUTTON, _T("ѡ���ļ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		175, 510, 130, 50,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFileID), MyWnds::hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("��ʼѹ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		435,510 ,130 ,50 ,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonStartID), MyWnds::hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("ȡ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		695, 510, 130, 50,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonCancelID), MyWnds::hInstance, this
	);
	//��������
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFileID), WM_SETFONT, (WPARAM)defFont, TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonStartID), WM_SETFONT, (WPARAM)defFont, TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)defFont, TRUE);
	//������ѡ���ļ��б�
	HWND selectedFileListHwnd = CreateWindowEx(
		0, WC_LISTVIEW, _T("��ѡ����ļ�"), WS_CHILD | WS_VISIBLE |WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS,
		50, 20, 900, 450,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::selectedFileListID), MyWnds::hInstance, this
	);
	if (!selectedFileListHwnd)ErrorMessageBox(hwnd_WndProc, _T("�����ļ��б��selectedFileListʧ��"));
	//������չ��ʽ
	ListView_SetExtendedListViewStyle(selectedFileListHwnd, LVS_EX_COLUMNSNAPPOINTS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
	//������
	LVCOLUMN column = { 0 };
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_MINWIDTH;
	column.cx = 260;
	column.pszText = (LPTSTR)_T("����");
	column.iSubItem = (int)selectedFileListColumnID::columnNameID;
	column.cxMin = 260;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnNameID, &column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 90;
	column.pszText = (LPTSTR)_T("����");
	column.iSubItem = (int)selectedFileListColumnID::columnTypeID;
	column.cxMin = 90;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnTypeID, &column);

	column.fmt = LVCFMT_LEFT;
	column.cx = 100;
	column.pszText = (LPTSTR)_T("��С");
	column.iSubItem = (int)selectedFileListColumnID::columnSizeID;
	column.cxMin = 100;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnSizeID, &column);

	column.fmt = LVCFMT_LEFT; 
	column.cx = 440;
	column.pszText = (LPTSTR)_T("�ļ�·��");
	column.iSubItem = (int)selectedFileListColumnID::columnPathID;
	column.cxMin = 440;
	ListView_InsertColumn(selectedFileListHwnd, selectedFileListColumnID::columnPathID, &column);
	return 0;
}



LRESULT ZipFunc::WM_CLOSE_WndProc()
{
	DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
	return 0;
}

LRESULT ZipFunc::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//����WM_QUIT��Ϣ
	return 0;
}


