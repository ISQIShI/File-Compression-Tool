#include"UnpackFunc.h"
#include<ShObjIdl_core.h>
#include"ThreadPool.h"
#include "FileService.h"
#include "HuffmanCode.h"

void UnpackFunc::StartUnpack()
{
	//��ʼ���̳߳�
	size_t maxThreadAmount = 20;//��ʱռλ ֮�����Ϊ�û��趨������߳���
	size_t availableThreadAmount = thread::hardware_concurrency();
	ThreadPool threadPool(availableThreadAmount < maxThreadAmount ? availableThreadAmount : maxThreadAmount);
	//��ʼ��ѹ���м�ʱ
	auto startTime = std::chrono::high_resolution_clock::now();
	//��ȡ��ʽ����-�����
	HuffmanCode::GetNormalSymbolCode(zipFile->codeLength, zipFile->symbolCode);
	if (!zipFile->symbolCode.empty()) {
		BYTE tempCodeLength = 0;
		//��ȡ ���볤��-����-���� ��
		for (auto& codeLength : zipFile->codeLength) {
			if (codeLength.second != tempCodeLength) {
				codeSymbol->emplace_back().first = codeLength.second;
				tempCodeLength = codeLength.second;
			}
			codeSymbol->back().second.emplace(zipFile->symbolCode.at(codeLength.first), codeLength.first);
		}
	}
	//�м��ʱ1
	auto middleTime = std::chrono::high_resolution_clock::now();
	//���ܽ�ѹ��ش���
	if (openIntelligentUnpack) {
		targetPath /= zipFile->zipFilePath.stem();
		while (exists(targetPath))targetPath += "-��ѹ�ļ�";
		create_directories(targetPath);
	}
	//��ѹ�����ļ�
	if (willUnpackAllFiles) {
		for (auto& internalFile : internalFileArr->at(0)) {
			UnpackObject(internalFile,threadPool);
		}
	}
	else {//��ѹѡ�е��ļ�
		size_t x = 0;
		if (folderIndex) ++x;
		UINT state;
		while (x < internalFileArr->at(folderIndex).size()) {
			//�������е�ǰ����ļ��е���
			state = ListView_GetItemState(GetDlgItem(MainWnd::GetMainWnd().GetWndHwnd(),fileListID), x, LVIS_SELECTED);
			//����ѡ�����ѹ����
			if (state & LVIS_SELECTED) UnpackObject(internalFileArr->at(folderIndex).at(x),threadPool);
			++x;
		}
	}
	//�ȴ���ѹ���
	threadPool.WaitTask(1);

	//��ɽ�ѹ������ʱ
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(middleTime - startTime);
	auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime - middleTime);
	TCHAR tempTCHAR[50];
	_stprintf_s(tempTCHAR, _T("ѹ��ʱ�䣺\n׼������ %f ��\n��ѹ�ļ� %f ��"), duration1.count() / 1000000.0, duration2.count() / 1000000.0);
	TestMessageBox(hwnd_WndProc, tempTCHAR, _T("��ѹ���"));
}

void UnpackFunc::UnpackObject(const InternalFileInfo & internalFile,ThreadPool & threadPool)
{
	//�ļ���
	if (internalFile.isFolder) {
		//��·���������򴴽�
        if (!exists(targetPath/internalFile.filePath)) {
            create_directories(targetPath/internalFile.filePath);
		}
		//��������ѹ���ļ�����������
		for (size_t i = 1; i < internalFileArr->at(internalFile.folderID).size(); ++i) {
			UnpackObject(internalFileArr->at(internalFile.folderID).at(i),threadPool);
		}
	}
	else {//�ļ�
		threadPool.SubmitTask(1, true, &UnpackFunc::UnpackFile, this, internalFile);
	}
}

void UnpackFunc::UnpackFile(const InternalFileInfo& internalFile)
{
	//����Ҫ��ѹ���ļ�
	if (exists(targetPath / internalFile.filePath)) {
		ErrorMessageBox(hwnd_WndProc, ((targetPath / internalFile.filePath).wstring() + L" �Ѵ���\n���ƶ����ļ����޸Ľ�ѹ·��").c_str(), false);
	}
	FileService::ExtendFile(targetPath / internalFile.filePath, internalFile.oldFileSize, true);
	if (internalFile.oldFileSize == 0)return;//���ļ�����
	//ӳ��ѹ���ļ��ж�Ӧ������
	MapFileInfo* mapZipFileInfo = new MapFileInfo((LPTSTR)zipFile->zipFilePath.c_str(), internalFile.fileOffset, internalFile.WPL_Size.first);
	//�����ļ�ӳ��
	FileService::MapFile(*mapZipFileInfo, true);
	//��ȡ�ļ�ӳ��ָ��
	BYTE* zipFilePointer = (BYTE*)mapZipFileInfo->mapViewPointer;
	//ӳ�佫Ҫ��ѹ��Ŀ���ļ�
	path tempPath = targetPath / internalFile.filePath;
	MapFileInfo* mapTargetFileInfo = new MapFileInfo((LPTSTR)tempPath.c_str(), 0, 0);
	//�����ļ�ӳ��
	FileService::MapFile(*mapTargetFileInfo);
	//��ȡ�ļ�ӳ��ָ��
	BYTE* targetFilePointer = (BYTE*)mapTargetFileInfo->mapViewPointer;
	//ÿ�ζ�ȡ����С����λ��
	BYTE tempIndex = 0;
	bitset<256> buffer = 0;
	bitset<8> tempbuffer;
	BYTE bitCount = 0;
	for (size_t reader = 0; reader < mapZipFileInfo->fileMapSize - 1; ++reader, ++zipFilePointer) {
		tempbuffer = *zipFilePointer;
		//���¶�ȡ��������ӵ�buffer��
		for (BYTE i = 0; i < 8; ++i) {
			buffer <<= 1;
			buffer[0] = tempbuffer[7 - i];
			++bitCount;
			if (bitCount == codeSymbol->at(tempIndex).first ) {
				if(codeSymbol->at(tempIndex).second.find(buffer)!= codeSymbol->at(tempIndex).second.end() )
				{
					*targetFilePointer = codeSymbol->at(tempIndex).second.at(buffer);
					++targetFilePointer;
					bitCount = 0;
					buffer.reset();
					tempIndex = 0;
				}
				else {
					++tempIndex;
				}
			}
		}
	}
	tempbuffer = *zipFilePointer;
	//���¶�ȡ��������ӵ�buffer��
	for (BYTE i = 0; i < 8 - internalFile.WPL_Size.second; ++i) {
		buffer <<= 1;
		buffer[0] = tempbuffer[7 - i];
		++bitCount;
		if (bitCount == codeSymbol->at(tempIndex).first) {
			if (codeSymbol->at(tempIndex).second.find(buffer) != codeSymbol->at(tempIndex).second.end())
			{
				*targetFilePointer = codeSymbol->at(tempIndex).second.at(buffer);
				++targetFilePointer;
				bitCount = 0;
				buffer.reset();
				tempIndex = 0;
			}
			else {
				++tempIndex;
			}
		}
	}

	delete mapZipFileInfo;
	delete mapTargetFileInfo;
}

ATOM UnpackFunc::RegisterWndClass()
{
	//ʵ�������������
	WNDCLASSEX unpackFuncWndClass = { 0 };
	unpackFuncWndClass.cbSize = sizeof(WNDCLASSEX);
	unpackFuncWndClass.style = CS_DBLCLKS;//����ʽ
	unpackFuncWndClass.lpfnWndProc = StaticWndProc;//���ڹ���
	unpackFuncWndClass.hInstance = hInstance;//����ʵ��
	unpackFuncWndClass.hbrBackground = HBRUSH(6);//�౳����ˢ
	unpackFuncWndClass.lpszClassName = _T("unpackFuncWndClassName");//��������
	unpackFuncWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//����ͼ��
	return RegisterClassEx(&unpackFuncWndClass);
}

HWND UnpackFunc::CreateWnd()
{
	RECT rect;
	GetWindowRect(MainWnd::GetMainWnd().GetWndHwnd(), &rect);
	//��������
	HWND unpackFuncWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("unpackFuncWndClassName"), _T("ѡ���ѹ·��"), WS_TILED | WS_CAPTION | WS_SYSMENU,
		0.5 * (MainWnd::GetMainWnd().GetWndWidth() - wndWidth) + rect.left, 0.5 * (MainWnd::GetMainWnd().GetWndHeight() - wndHeight) + rect.top, wndWidth, wndHeight,
		isModalDialog, NULL, hInstance, this
	);
	//��ʾ����
	if (unpackFuncWndHwnd) ShowWindow(unpackFuncWndHwnd, SW_SHOW);
	return unpackFuncWndHwnd;
}

LRESULT UnpackFunc::WM_COMMAND_WndProc()
{
	//֪ͨ�����ǵ����ť
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//���ݵ���İ�ť��ִͬ�в�ͬ����
		switch (LOWORD(wParam_WndProc)) {
		case (int)UnpackFuncWndChildID::radioButtonAllFilesID: {//��� ȫ���ļ� ��ѡ��
			willUnpackAllFiles = true;
			break;
		}
		case (int)UnpackFuncWndChildID::radioButtonSelectedFilesID: {//��� �����ļ� ��ѡ��
			willUnpackAllFiles = false;
			break;
		}
		case (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID: {//��� ���ܽ�ѹ ��ѡ��
			// ��ȡ��ѡ���״̬
			LRESULT state = SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID), BM_GETCHECK, 0, 0);
			if (state == BST_CHECKED)openIntelligentUnpack = true;
			else openIntelligentUnpack = false;
			break;
		}
		case (int)UnpackFuncWndChildID::buttonBrowseID: {
			ClickBrowseButton();
			break;
		}
		case (int)UnpackFuncWndChildID::buttonConfirmID: {
			ClickConfirmButton();
			break;
		}
		case (int)UnpackFuncWndChildID::buttonCancelID: {
			DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
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

LRESULT UnpackFunc::WM_PAINT_WndProc()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd_WndProc, &ps);
	SelectObject(hdc, lTSObject[0]);
	TextOut(hdc, 20, 20, _T("Ŀ��·��"), wcslen(_T("Ŀ��·��")));
	TextOut(hdc, 20, 70, _T("��ѹ��:"), wcslen(_T("��ѹ��:")));
	EndPaint(hwnd_WndProc, &ps);
	return 0;
}

LRESULT UnpackFunc::WM_CTLCOLORSTATIC_WndProc()
{
	HDC hdc = (HDC)wParam_WndProc;
	SetBkColor(hdc, RGB(255, 255, 255)); // ���ñ���Ϊ��ɫ
	return (LRESULT)GetStockObject(WHITE_BRUSH);
}

LRESULT UnpackFunc::WM_CREATE_WndProc()
{
	//��������
	lTSObject[0] = CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("����")); // ����һ�� ���� ���壬20px ��С

	//������ť
	CreateWindowEx(
		0, WC_BUTTON, _T("���"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 15, 100, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonBrowseID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonBrowseID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("ȷ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		745, 115, 100, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonConfirmID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonConfirmID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("ȡ��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 115, 100, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonCancelID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//������ѡ��ť
	CreateWindowEx(
		0, WC_BUTTON, _T("ȫ���ļ�"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
		170, 65, 110, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::radioButtonAllFilesID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonAllFilesID), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonAllFilesID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("ѡ�����ļ�"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		320, 65, 130, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::radioButtonSelectedFilesID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonSelectedFilesID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	if(!ListView_GetSelectedCount(GetDlgItem(MainWnd::GetMainWnd().GetWndHwnd(),fileListID)))EnableWindow(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonSelectedFilesID), FALSE);
	//������ѡ��ť
	CreateWindowEx(
		0, WC_BUTTON, _T("���ܽ�ѹ"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		20, 115, 110, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::checkBoxIntelligentUnpackID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID), BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//�����༭��
	CreateWindowEx(
		WS_EX_CLIENTEDGE, WC_EDIT, zipFile->zipFilePath.parent_path().c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
		115, 15, 725, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::editFileNameID), hInstance, this
	);
	//���ñ༭�ؼ��е��ı���ʾ
	Edit_SetCueBannerText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), _T("��ѡ���ѹ·��"));
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);

	return 0;
}

LRESULT UnpackFunc::WM_CLOSE_WndProc()
{
	DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
	return 0;
}

LRESULT UnpackFunc::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//����WM_QUIT��Ϣ
	return 0;
}

void UnpackFunc::ClickConfirmButton()
{
	int length = GetWindowTextLength(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID));
	//���ѹ���ļ�·���Ƿ���ȷ
	if (!length) {
		MessageBox(hwnd_WndProc, _T("Ŀ��·��Ϊ��,�����ѹ·��"), _T("Ѽһѹ"), MB_OK | MB_TASKMODAL);
		return;
	}
	LPTSTR fileNameStr = new TCHAR[length + 1];
	GetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), fileNameStr, length + 1);
	//����Ϊ����·��
	path fileName = absolute(fileNameStr);
	//���ñ༭���ı�
	SetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), fileName.c_str());
	if (!exists(fileName.root_path())) {
		MessageBox(hwnd_WndProc, _T("Ŀ��·���и�Ŀ¼������,���޸Ľ�ѹ·��"), _T("Ѽһѹ"), MB_OK | MB_TASKMODAL);
		return;
	}
	if (!exists(fileName)) {
		if (MessageBox(hwnd_WndProc, (_T("ָ����·��������,�Ƿ��½��ļ���?\n-") + fileName.native()).c_str(), _T("Ѽһѹ"), MB_YESNO | MB_TASKMODAL) == IDYES) {
			create_directories(fileName);
		}
		else return;
	}
	if (MessageBox(hwnd_WndProc, (_T("�Ƿ�ȷ�����ļ�(�ļ���)��ѹ����ǰ·��?\n-") + fileName.native()).c_str(), _T("Ѽһѹ"), MB_OKCANCEL | MB_TASKMODAL) != IDOK) {
		return;
	}
	targetPath = fileName;
	StartUnpack();
	DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
}

void UnpackFunc::ClickBrowseButton()
{
	//��ʼ����Դ
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	IFileOpenDialog* fileOpenDialog = nullptr;
	//�����Ի���ʵ��
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
	if (!SUCCEEDED(hr))throw runtime_error("�޷������Ի���");
	//�����ļ��Ի���ѡ��
	DWORD dword;
	//�����ļ��ж�ѡ
	hr = fileOpenDialog->GetOptions(&dword);
	hr = fileOpenDialog->SetOptions(dword | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT);
	//��ʾ�Ի���
	hr = fileOpenDialog->Show(MainWnd::GetMainWnd().GetWndHwnd());
	//��ȡ�û�ѡ����
	IShellItem* selectedItem = nullptr;
	hr = fileOpenDialog->GetResult(&selectedItem);
	if (!SUCCEEDED(hr))
	{
		fileOpenDialog->Release();
		CoUninitialize();
		return;
	}
	LPWSTR filePath = nullptr;
	//��ȡ�ļ���·��
	selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	//���ñ༭���ı�
	SetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), filePath);
	//������Դ
	selectedItem->Release();
	CoTaskMemFree(filePath);
	fileOpenDialog->Release();
	CoUninitialize();
}

bool UnpackFunc::OpenZipFile(){
	//��ʼ����Դ
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	IFileOpenDialog* fileOpenDialog = nullptr;
	//�����Ի���ʵ��
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
	if (!SUCCEEDED(hr))throw runtime_error("�޷������Ի���");
	//���ù�����
	COMDLG_FILTERSPEC filter[] = { { L"ѹ�����ļ�", L"*.ya" } };
	hr = fileOpenDialog->SetFileTypes(ARRAYSIZE(filter), filter);
	//��ʾ�Ի���
	hr = fileOpenDialog->Show(MainWnd::GetMainWnd().GetWndHwnd());
	LPWSTR filePath = nullptr;
	//��ȡ�û�ѡ����
	IShellItem* selectedItem;
	hr = fileOpenDialog->GetResult(&selectedItem);
	if (!SUCCEEDED(hr))
	{
		fileOpenDialog->Release();
		CoUninitialize();
		return false;
	}
	//��ȡ�ļ�·��
	selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	/*if (zipFile->zipFilePath == filePath)
	{
		selectedItem->Release();
		CoTaskMemFree(filePath);
		fileOpenDialog->Release();
		CoUninitialize();
		return false;
	}*/
	//��ʼ���ļ��б�����
	if (internalFileArr) {
		delete internalFileArr;
		internalFileArr = new vector<vector<InternalFileInfo>>;
		internalFileArr->resize(1);
		InternalFileInfo::folderAmount = 0;
	}
	//��ʼ��������ű�
	if (codeSymbol && !codeSymbol->empty()) {
		codeSymbol->clear();
	}
	//��ʼ���ļ������
	folderIndex = 0;
	willUnpackAllFiles = true;
	openIntelligentUnpack = true;
	targetPath.clear();
	//��ʼ��ѹ���ļ���Ϣ
	if (zipFile) {
		delete zipFile; 
		zipFile = new ZipFileInfo;
	}
	
	zipFile->tempZipFilePath = zipFile->zipFilePath;
	zipFile->zipFilePath = filePath;
	//������Դ
	selectedItem->Release();
	CoTaskMemFree(filePath);
	fileOpenDialog->Release();
	CoUninitialize();
	return true;
}

void UnpackFunc::GetFileInfo()
{
	//��ѹ�����ļ�
	HANDLE fileHandle = CreateFile(zipFile->zipFilePath.c_str(), GENERIC_READ , FILE_SHARE_READ , nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("�޷����ļ�"));
	}
	//�����ļ�ָ��
	LARGE_INTEGER tempL_I;
	tempL_I.QuadPart = 0;
	SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_BEGIN);
	//-----------------------------------��ȡѹ�����ײ�--------------------------------------
	DWORD readbyte;
	//��ȡ ��ʶǩ��
	char identification[2];
	ReadFile(fileHandle,identification, 2, &readbyte, nullptr);
    if (identification[0] != 'y' || identification[1] != 'a') {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("δ֪���ļ���ʽ"),false);
	}
	//��ȡ ѹ�����ײ� ����
	unsigned short headerSize;
	ReadFile(fileHandle, &headerSize, 2, &readbyte, nullptr);
	zipFile->codeLength.resize((headerSize - 4) / 2);
	//��ȡ ����-���볤�� ��
	for (size_t i = 0; i < zipFile->codeLength.size(); ++i)
	{
		ReadFile(fileHandle, &zipFile->codeLength[i], 2, &readbyte, nullptr);
	}
	zipFile->newFileSize += headerSize;
	//------------------------------------��ȡ���ļ��ײ�-------------------------------------------
	InternalFileInfo internalFile;
	//�ײ���С
	unsigned short fileheaderSize;
	while (ReadFile(fileHandle, &fileheaderSize, 2, &readbyte, nullptr) && readbyte != 0) {
		//ԭʼ�ļ���С
		ReadFile(fileHandle, &internalFile.oldFileSize, 8, &readbyte, nullptr);
		//ѹ�����С
		ReadFile(fileHandle, &internalFile.WPL_Size.first, 8, &readbyte, nullptr);
		//��������
		ReadFile(fileHandle, &internalFile.WPL_Size.second, 1, &readbyte, nullptr);
		//�ļ���
		LPWSTR tempWCHAR = new WCHAR[fileheaderSize - 18]{};
		ReadFile(fileHandle, tempWCHAR, fileheaderSize - 19, &readbyte, nullptr);
		internalFile.fileName = tempWCHAR;
		internalFile.filePath = *internalFile.fileName.begin();
		internalFile.isFolder = false;
		zipFile->newFileSize += fileheaderSize;
		internalFile.fileOffset = zipFile->newFileSize;
		//�ƶ��ļ�ָ��
		tempL_I.QuadPart = internalFile.WPL_Size.first;
		SetFilePointerEx(fileHandle, tempL_I, nullptr, FILE_CURRENT);
		zipFile->newFileSize += internalFile.WPL_Size.first;
		StoreFileInfo(internalFile,0);
	}
	//����У��
    if (zipFile->newFileSize != file_size(zipFile->zipFilePath)) {
		ErrorMessageBox(MainWnd::GetMainWnd().GetWndHwnd(), _T("ѹ������Ϣ��ȡ����"),false);
	}
	//�Դ洢ѹ�����ڲ��ļ���Ϣ��������������
	vector<InternalFileInfo>::iterator beginIt;
	for (size_t i = 0; i < internalFileArr->size();++i) {
		if (internalFileArr->at(i).size() > 1) {
			beginIt = internalFileArr->at(i).begin();
			if (i)++beginIt;
			sort(beginIt, internalFileArr->at(i).end(), [](const InternalFileInfo& a, const InternalFileInfo& b) {return a.isFolder > b.isFolder; });
		}
	}
	CloseHandle(fileHandle);
}

void UnpackFunc::StoreFileInfo(InternalFileInfo& internalFile, size_t folderID)
{
	//׼�������ļ���
	if (internalFile.fileName.has_parent_path()) {
		//���Ȳ����������Ƿ��Ѿ����ڸ��ļ���
		auto it = find_if(internalFileArr->at(folderID).begin(), internalFileArr->at(folderID).end(),
			[&internalFile](const InternalFileInfo& info){return info.fileName == *internalFile.fileName.begin(); });
		if (it == internalFileArr->at(folderID).end()) {//�������������
			InternalFileInfo& temp = internalFileArr->at(folderID).emplace_back(true, *internalFile.fileName.begin(), internalFile.filePath,internalFile.oldFileSize,internalFile.WPL_Size,0 );
			internalFileArr->emplace_back();//�����µ�һά����
			internalFileArr->at(temp.folderID).emplace_back(true, _T("..(������һ���ļ���)"), _T(""), 0, pair<uintmax_t, BYTE>(0, 0), 0, folderID, true);
			folderID = temp.folderID;
		}
		else {//��������������Ϣ����
			 it->oldFileSize += internalFile.oldFileSize;
			 it->WPL_Size.first += internalFile.WPL_Size.first;
			 folderID = it->folderID;
		}
		internalFile.fileName = relative(internalFile.fileName, *internalFile.fileName.begin());
		internalFile.filePath /= *internalFile.fileName.begin();
		StoreFileInfo(internalFile, folderID);
	}
	else {//׼�������ļ�
		internalFileArr->at(folderID).emplace_back(false, internalFile.fileName, internalFile.filePath,internalFile.oldFileSize, internalFile.WPL_Size, internalFile.fileOffset, folderID);
	}
}

