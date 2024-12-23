#include"ZipFunc.h"
#include <shobjidl.h> 
#include "FileService.h"
#include <chrono>
#include "HuffmanCode.h"

void ZipFunc::StartZip(bool openMultiThread)
{
	//ɾ���ļ�
	if (exists(zipFileName))remove(zipFileName);
	//��ʼѹ�����м�ʱ
	auto startTime = std::chrono::high_resolution_clock::now();

	//-----------------------------------���߳�-----------------------------
	//���� ����-Ƶ�ʱ�
	auto symbolFrequency = new unordered_map<BYTE, size_t>;
	
	for (auto &filePath : filePathArr)
	{
		//��ȡ��ͨ�ļ��� ����-Ƶ��
		if (is_regular_file(filePath)) {
			HuffmanCode::GetSymbolFrequency(*symbolFrequency, filePath);
			continue;
		}
		//�����ļ����������ļ�����ȡ ����-Ƶ��
		for (const auto& entry : filesystem::recursive_directory_iterator(filePath))
		{
			if (entry.is_regular_file())
			{
				HuffmanCode::GetSymbolFrequency(*symbolFrequency, entry);
			}
		}
	}
	//���� ����-���볤�ȱ�
	auto codeLength = new vector<pair<BYTE, BYTE>>;
	HuffmanNode* rootNode = HuffmanCode::BuildHuffmanTree(*symbolFrequency);
	//�ݹ������������,��ȡ�����ŵı��볤��
	HuffmanCode::EncodeHuffmanTree(*codeLength, rootNode);
	//�ݹ����ٹ�������
	HuffmanCode::DestroyHuffmanTree(rootNode);
	//���� ����-��ʽ�����
	auto * symbolCode = new unordered_map<BYTE, string>;
	HuffmanCode::GetNormalSymbolCode(*codeLength, *symbolCode);
	//��ʱ1
	auto middleTime = std::chrono::high_resolution_clock::now();
	//д��ѹ�����ײ�
	//FileService::WriteZipFileHeader(zipFileName, *codeLength);
	//д���ļ�
	for (auto& filePath : filePathArr) {
		if (is_regular_file(filePath)) {
			//д���ļ��ײ�
			FileService::ZipFile(filePath, zipFileName, *symbolCode);
			continue;
		}
		for (const auto& entry : filesystem::recursive_directory_iterator(filePath))
		{
			if (entry.is_regular_file())
			{
				FileService::ZipFile(entry, zipFileName, *symbolCode);
			}
		}
	}
	//������Դ
	delete symbolFrequency;
	delete codeLength;
	delete symbolCode;
	//���ѹ��������ʱ
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(middleTime - startTime);
	auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime - middleTime);
	TCHAR tempTCHAR[50];
	_stprintf_s (tempTCHAR, _T("ѹ��ʱ�䣺\n��ȡ����� %f ��\nд���ļ� %f ��"), duration1.count()/1000000.0,duration2.count()/1000000.0);
	TestMessageBox(hwnd_WndProc, tempTCHAR, _T("ѹ�����"));
}

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

LRESULT ZipFunc::WM_COMMAND_WndProc()
{
	//֪ͨ�����ǵ����ť
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//���ݵ���İ�ť��ִͬ�в�ͬ����
		switch (LOWORD(wParam_WndProc)) {
		//���ѡ���ļ���ѡ���ļ��а�ť
		case (int)ZipFuncWndChildID::buttonSelectFileID:case (int)ZipFuncWndChildID::buttonSelectFolderID:
		{
			//��ʼ����Դ
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileOpenDialog* fileOpenDialog = nullptr;
			//�����Ի���ʵ��
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
			if (!SUCCEEDED(hr))break;
			//�����ļ��Ի���ѡ��
            DWORD dword;
            hr = fileOpenDialog ->GetOptions(&dword);
			if (LOWORD(wParam_WndProc)==(int)ZipFuncWndChildID::buttonSelectFileID)
			{
				//�����ļ���ѡ
				hr = fileOpenDialog ->SetOptions(dword | FOS_ALLOWMULTISELECT); 
				//���ù�����
				COMDLG_FILTERSPEC filter[] = { { L"�����ļ�", L"*.*" }, { L"�ı��ļ�", L"*.txt" }};
				hr = fileOpenDialog->SetFileTypes(ARRAYSIZE(filter), filter);
			}
			else{
				//�����ļ��ж�ѡ
				hr = fileOpenDialog ->SetOptions(dword | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT); 
			}
      		//��ʾ�Ի���
			hr = fileOpenDialog->Show(hwnd_WndProc);
            //��ȡ�û�ѡ����
            IShellItemArray* selectedIItemArray;
			hr = fileOpenDialog->GetResults(&selectedIItemArray);
			if (!SUCCEEDED(hr))break;
			//��ȡѡ����ļ���
			DWORD fileCount = 0;
			selectedIItemArray->GetCount(&fileCount);
			//��������ѡ����ļ�
			for (DWORD x = 0; x < fileCount; ++x) {
				IShellItem* selectedItem = nullptr;
				LPWSTR filePath = nullptr;
				//��ȡ����Ϊx���ļ�
				selectedIItemArray->GetItemAt(x,&selectedItem);
				//��ȡ�ļ�·��
				selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
				//�������в����ڸ��ļ�·��ʱ�������ļ�·������
				if (find_if(filePathArr.begin(), filePathArr.end(), [&filePath](path& temp) -> bool {return temp.wstring() == wstring(filePath); }) == filePathArr.end()){
					filePathArr.push_back(filePath);
				}
				//������Դ
				selectedItem->Release();
				CoTaskMemFree(filePath);
			}
			//������Դ
			selectedIItemArray->Release();
			fileOpenDialog->Release();
			CoUninitialize();
			//׼��ˢ���б�����
			//ɾ��������
			ListView_DeleteAllItems(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID));
			//��������ļ��б�
			LVITEM item;
			wstring tempWString;
			TCHAR tempTCHAR[21];
			for (size_t x = 0; x < filePathArr.size();++x) {
				item.mask = LVIF_TEXT;
				item.iItem = x;//������
				item.iSubItem = 0;//��������
				tempWString = filePathArr[x].filename().wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//����
				ListView_InsertItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//��������
				if (is_directory(filePathArr[x]))item.pszText = (LPTSTR)_T("�ļ���");//����
				else item.pszText = (LPTSTR)_T("�ļ�");
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//��������
				uintmax_t size = FileService::GetFileSize(filePathArr[x]);
				if(size >= (uintmax_t(2048*1024)*1024))_stprintf_s(tempTCHAR, _T("%.2f GB"), double(size)/(1024*1024*1024));
				else if(size >= 2048*1024)_stprintf_s(tempTCHAR, _T("%.2f MB"), double(size)/(1024*1024));
				else if(size >= 2048)_stprintf_s(tempTCHAR, _T("%.2f KB"), double(size)/1024);
				else _stprintf_s(tempTCHAR, _T("%.2f B"), double(size));
				item.pszText = tempTCHAR;//��С
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//��������
				tempWString = filePathArr[x].wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//·��
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);
			}
			break;
		}
		case (int)ZipFuncWndChildID::buttonDeleteID: {
			HWND hwnd = GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID);
			for (size_t x = 0; x < filePathArr.size(); ++x) {
				//���ɾ��ʱ����������
				UINT state = ListView_GetItemState(hwnd, x, LVIS_SELECTED);
				//����ѡ����ɾ��
				if (state & LVIS_SELECTED) {
					filePathArr.erase(filePathArr.begin() + x);
					ListView_DeleteItem(hwnd, x);
					--x;//��������ֵ
				}
			}
			break;
		}
		case (int)ZipFuncWndChildID::buttonStartID:{
			//�������ѹ���ļ����Ƿ���ȷ
			if (filePathArr.empty()) {
				MessageBox(hwnd_WndProc, _T("û��Ҫѹ�����ļ�(�ļ���),��������ļ�(�ļ���)"), _T("Ѽһѹ"), MB_OK | MB_TASKMODAL);
				break;
			}
			int length = GetWindowTextLength(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID));
			if (!length) {
				MessageBox(hwnd_WndProc, _T("�ļ���Ϊ��,����ѹ���ļ���"), _T("Ѽһѹ"), MB_OK | MB_TASKMODAL);
				break;
			}
			LPTSTR fileNameStr = new TCHAR[length + 1];
			GetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), fileNameStr,length + 1);
			//����Ϊ����·��
			path fileName = absolute(fileNameStr);
			//���ñ༭���ı�
			SetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), fileName.c_str());
			if (!exists(fileName.root_path())) {	
				MessageBox(hwnd_WndProc, _T("�ļ�·���и�Ŀ¼������,���޸�ѹ���ļ�·��"), _T("Ѽһѹ"), MB_OK | MB_TASKMODAL);
				break;
			}
			if (!exists(fileName.parent_path())||!is_directory(fileName.parent_path())) {
				if (MessageBox(hwnd_WndProc, (_T("ָ���ĸ����ļ��в�����,�Ƿ��½��ļ���?\n-") + fileName.parent_path().native()).c_str(), _T("Ѽһѹ"), MB_YESNO | MB_TASKMODAL) == IDYES) {
					create_directories(fileName.parent_path());
				}
				else break;
			}
			if (MessageBox(hwnd_WndProc, (_T("�Ƿ�ȷ������ѡ�ļ�(�ļ���)ѹ������ǰ·��?\n-") + fileName.native()).c_str(),_T("Ѽһѹ") ,MB_OKCANCEL | MB_TASKMODAL) != IDOK) {
				break;
			}
			zipFileName = fileName;
			//��ʼѹ��
			StartZip();
			DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
			break;
		}
		case (int)ZipFuncWndChildID::buttonCancelID:{
			DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
			break;
		}
		case (int)ZipFuncWndChildID::buttonBrowseID: {
			//��ʼ����Դ
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileSaveDialog* fileSaveDialog = nullptr;
			//�����Ի���ʵ��
			hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileSaveDialog));
			if (!SUCCEEDED(hr))break;
			//����Ĭ���ļ���
			fileSaveDialog->SetFileName(L"�½�ѹ����");
			//�����ļ���׺
			COMDLG_FILTERSPEC rgSpec[] = { { L"ѹ�����ļ� (*.ya)", L"*.ya" }};
            fileSaveDialog->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
			fileSaveDialog->SetDefaultExtension(L"ya");
			//�����ļ��Ի���ѡ��
			//DWORD dword;
			//hr = fileSaveDialog->GetOptions(&dword);
			//hr = fileSaveDialog->SetOptions(dword);
      		//��ʾ�Ի���
			hr = fileSaveDialog->Show(hwnd_WndProc);
            //��ȡ�û�ѡ����
            IShellItem* selectedItem = nullptr;
			hr = fileSaveDialog->GetResult(&selectedItem);
			if (!SUCCEEDED(hr))break;
			LPWSTR filePath = nullptr;
			//��ȡ�ļ���·��
			selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
			//���ñ༭���ı�
			SetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), filePath);
			//������Դ
			selectedItem->Release();
			CoTaskMemFree(filePath);
			fileSaveDialog->Release();
			CoUninitialize();
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

LRESULT ZipFunc::WM_NOTIFY_WndProc()
{	
	//��ȡ��ϢЯ������Ϣ
	LPNMHDR lpnmhdr = (LPNMHDR)lParam_WndProc;
	//����ѹ���ļ����Ʊ༭�����Ϣ
	if (lpnmhdr->idFrom == (UINT)ZipFuncWndChildID::editFileNameID){
		//�ж�֪ͨ����
		switch (lpnmhdr->code) {
			case NM_CLICK:case NM_RETURN:{//����
				
			}
		}
	}
	else {
		//����δ�������Ϣʹ��Ĭ�ϴ��ڹ��̴���
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT ZipFunc::WM_PAINT_WndProc(){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd_WndProc, &ps);
	SelectObject(hdc, lTSObject[0]);
	//SetTextColor(hdc, RGB(46, 134, 193));//����ǰ��ɫ
	TextOut(hdc, 20, 430, _T("ѹ���ļ�����"), wcslen(_T("ѹ���ļ�����")));
	TextOut(hdc, 20, 480, _T("�ļ���"), wcslen(_T("�ļ���")));
	TextOut(hdc, 20, 530, _T("ѹ����ʽ        .ya"), wcslen(_T("ѹ����ʽ        .ya")));
	POINT points1[2] = { {20,425},{965,425} };
	Polyline(hdc, points1, 2);
	POINT points2[2] = { {20,555},{965,555} };
	Polyline(hdc, points2, 2);
	EndPaint(hwnd_WndProc, &ps);
	return 0;
}

LRESULT ZipFunc::WM_CREATE_WndProc(){
	//��ʼ���ļ��б�����
	vector<path> temptemp;
	filePathArr.swap(temptemp);
	//��������
	lTSObject[0] = CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("����")); // ����һ�� ���� ���壬20px ��С
	//������ť
	CreateWindowEx(
		0, WC_BUTTON, _T("����ļ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		605, 380, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFileID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("����ļ���"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		725, 380, 120, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFolderID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("ɾ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 380, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonDeleteID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("��ʼѹ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		745, 560 ,100 ,40 ,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonStartID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("ȡ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 560, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonCancelID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("���"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 475, 100, 30,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonBrowseID), hInstance, this
	);
	//��������
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFileID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFolderID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonDeleteID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonStartID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonBrowseID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//�����༭��
	CreateWindowEx(
		WS_EX_CLIENTEDGE, WC_EDIT,(current_path().native() + _T("\\�½�ѹ����") + _T(SUFFIX)).c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
		100,475,740,30,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::editFileNameID), hInstance, this
	);
	//���ñ༭�ؼ��е��ı���ʾ
	Edit_SetCueBannerText(GetDlgItem(hwnd_WndProc,(int)ZipFuncWndChildID::editFileNameID), _T("��ѡ��ѹ���ļ�·�����趨ѹ���ļ���"));
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//������ѡ���ļ��б�
	HWND selectedFileListHwnd = CreateWindowEx(
		0, WC_LISTVIEW, _T("��ѡ����ļ�"), WS_CHILD | WS_VISIBLE |WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS,
		20, 20, 945, 350,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::selectedFileListID), hInstance, this
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


