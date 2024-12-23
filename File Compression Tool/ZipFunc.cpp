#include"ZipFunc.h"
#include <shobjidl.h> 
#include "FileService.h"
#include <chrono>
#include "HuffmanCode.h"

void ZipFunc::StartZip(bool openMultiThread)
{
	//删除文件
	if (exists(zipFileName))remove(zipFileName);
	//开始压缩进行计时
	auto startTime = std::chrono::high_resolution_clock::now();

	//-----------------------------------单线程-----------------------------
	//创建 符号-频率表
	auto symbolFrequency = new unordered_map<BYTE, size_t>;
	
	for (auto &filePath : filePathArr)
	{
		//获取普通文件中 符号-频率
		if (is_regular_file(filePath)) {
			HuffmanCode::GetSymbolFrequency(*symbolFrequency, filePath);
			continue;
		}
		//遍历文件夹中所有文件，获取 符号-频率
		for (const auto& entry : filesystem::recursive_directory_iterator(filePath))
		{
			if (entry.is_regular_file())
			{
				HuffmanCode::GetSymbolFrequency(*symbolFrequency, entry);
			}
		}
	}
	//创建 符号-编码长度表
	auto codeLength = new vector<pair<BYTE, BYTE>>;
	HuffmanNode* rootNode = HuffmanCode::BuildHuffmanTree(*symbolFrequency);
	//递归遍历哈夫曼树,获取各符号的编码长度
	HuffmanCode::EncodeHuffmanTree(*codeLength, rootNode);
	//递归销毁哈夫曼树
	HuffmanCode::DestroyHuffmanTree(rootNode);
	//创建 符号-范式编码表
	auto * symbolCode = new unordered_map<BYTE, string>;
	HuffmanCode::GetNormalSymbolCode(*codeLength, *symbolCode);
	//计时1
	auto middleTime = std::chrono::high_resolution_clock::now();
	//写入压缩包首部
	//FileService::WriteZipFileHeader(zipFileName, *codeLength);
	//写入文件
	for (auto& filePath : filePathArr) {
		if (is_regular_file(filePath)) {
			//写入文件首部
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
	//销毁资源
	delete symbolFrequency;
	delete codeLength;
	delete symbolCode;
	//完成压缩结束计时
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(middleTime - startTime);
	auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime - middleTime);
	TCHAR tempTCHAR[50];
	_stprintf_s (tempTCHAR, _T("压缩时间：\n获取编码表 %f 秒\n写入文件 %f 秒"), duration1.count()/1000000.0,duration2.count()/1000000.0);
	TestMessageBox(hwnd_WndProc, tempTCHAR, _T("压缩完成"));
}

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

LRESULT ZipFunc::WM_COMMAND_WndProc()
{
	//通知代码是点击按钮
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//根据点击的按钮不同执行不同功能
		switch (LOWORD(wParam_WndProc)) {
		//点击选择文件和选择文件夹按钮
		case (int)ZipFuncWndChildID::buttonSelectFileID:case (int)ZipFuncWndChildID::buttonSelectFolderID:
		{
			//初始化资源
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileOpenDialog* fileOpenDialog = nullptr;
			//创建对话框实例
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
			if (!SUCCEEDED(hr))break;
			//配置文件对话框选项
            DWORD dword;
            hr = fileOpenDialog ->GetOptions(&dword);
			if (LOWORD(wParam_WndProc)==(int)ZipFuncWndChildID::buttonSelectFileID)
			{
				//设置文件多选
				hr = fileOpenDialog ->SetOptions(dword | FOS_ALLOWMULTISELECT); 
				//设置过滤器
				COMDLG_FILTERSPEC filter[] = { { L"所有文件", L"*.*" }, { L"文本文件", L"*.txt" }};
				hr = fileOpenDialog->SetFileTypes(ARRAYSIZE(filter), filter);
			}
			else{
				//设置文件夹多选
				hr = fileOpenDialog ->SetOptions(dword | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT); 
			}
      		//显示对话框
			hr = fileOpenDialog->Show(hwnd_WndProc);
            //获取用户选择结果
            IShellItemArray* selectedIItemArray;
			hr = fileOpenDialog->GetResults(&selectedIItemArray);
			if (!SUCCEEDED(hr))break;
			//获取选择的文件数
			DWORD fileCount = 0;
			selectedIItemArray->GetCount(&fileCount);
			//遍历所有选择的文件
			for (DWORD x = 0; x < fileCount; ++x) {
				IShellItem* selectedItem = nullptr;
				LPWSTR filePath = nullptr;
				//获取索引为x的文件
				selectedIItemArray->GetItemAt(x,&selectedItem);
				//获取文件路径
				selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
				//当数组中不存在该文件路径时，插入文件路径数组
				if (find_if(filePathArr.begin(), filePathArr.end(), [&filePath](path& temp) -> bool {return temp.wstring() == wstring(filePath); }) == filePathArr.end()){
					filePathArr.push_back(filePath);
				}
				//销毁资源
				selectedItem->Release();
				CoTaskMemFree(filePath);
			}
			//销毁资源
			selectedIItemArray->Release();
			fileOpenDialog->Release();
			CoUninitialize();
			//准备刷新列表数据
			//删除所有行
			ListView_DeleteAllItems(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID));
			//将项插入文件列表
			LVITEM item;
			wstring tempWString;
			TCHAR tempTCHAR[21];
			for (size_t x = 0; x < filePathArr.size();++x) {
				item.mask = LVIF_TEXT;
				item.iItem = x;//项索引
				item.iSubItem = 0;//子项索引
				tempWString = filePathArr[x].filename().wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//名称
				ListView_InsertItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//子项索引
				if (is_directory(filePathArr[x]))item.pszText = (LPTSTR)_T("文件夹");//类型
				else item.pszText = (LPTSTR)_T("文件");
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//子项索引
				uintmax_t size = FileService::GetFileSize(filePathArr[x]);
				if(size >= (uintmax_t(2048*1024)*1024))_stprintf_s(tempTCHAR, _T("%.2f GB"), double(size)/(1024*1024*1024));
				else if(size >= 2048*1024)_stprintf_s(tempTCHAR, _T("%.2f MB"), double(size)/(1024*1024));
				else if(size >= 2048)_stprintf_s(tempTCHAR, _T("%.2f KB"), double(size)/1024);
				else _stprintf_s(tempTCHAR, _T("%.2f B"), double(size));
				item.pszText = tempTCHAR;//大小
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);

				++item.iSubItem;//子项索引
				tempWString = filePathArr[x].wstring();
				item.pszText = (LPTSTR)tempWString.c_str();//路径
				ListView_SetItem(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID), &item);
			}
			break;
		}
		case (int)ZipFuncWndChildID::buttonDeleteID: {
			HWND hwnd = GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::selectedFileListID);
			for (size_t x = 0; x < filePathArr.size(); ++x) {
				//点击删除时遍历所有项
				UINT state = ListView_GetItemState(hwnd, x, LVIS_SELECTED);
				//如果项被选中则删除
				if (state & LVIS_SELECTED) {
					filePathArr.erase(filePathArr.begin() + x);
					ListView_DeleteItem(hwnd, x);
					--x;//修正索引值
				}
			}
			break;
		}
		case (int)ZipFuncWndChildID::buttonStartID:{
			//首先审查压缩文件名是否正确
			if (filePathArr.empty()) {
				MessageBox(hwnd_WndProc, _T("没有要压缩的文件(文件夹),请先添加文件(文件夹)"), _T("鸭一压"), MB_OK | MB_TASKMODAL);
				break;
			}
			int length = GetWindowTextLength(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID));
			if (!length) {
				MessageBox(hwnd_WndProc, _T("文件名为空,请检查压缩文件名"), _T("鸭一压"), MB_OK | MB_TASKMODAL);
				break;
			}
			LPTSTR fileNameStr = new TCHAR[length + 1];
			GetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), fileNameStr,length + 1);
			//解析为绝对路径
			path fileName = absolute(fileNameStr);
			//设置编辑框文本
			SetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), fileName.c_str());
			if (!exists(fileName.root_path())) {	
				MessageBox(hwnd_WndProc, _T("文件路径中根目录不存在,请修改压缩文件路径"), _T("鸭一压"), MB_OK | MB_TASKMODAL);
				break;
			}
			if (!exists(fileName.parent_path())||!is_directory(fileName.parent_path())) {
				if (MessageBox(hwnd_WndProc, (_T("指定的父级文件夹不存在,是否新建文件夹?\n-") + fileName.parent_path().native()).c_str(), _T("鸭一压"), MB_YESNO | MB_TASKMODAL) == IDYES) {
					create_directories(fileName.parent_path());
				}
				else break;
			}
			if (MessageBox(hwnd_WndProc, (_T("是否确定将所选文件(文件夹)压缩到当前路径?\n-") + fileName.native()).c_str(),_T("鸭一压") ,MB_OKCANCEL | MB_TASKMODAL) != IDOK) {
				break;
			}
			zipFileName = fileName;
			//开始压缩
			StartZip();
			DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
			break;
		}
		case (int)ZipFuncWndChildID::buttonCancelID:{
			DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
			break;
		}
		case (int)ZipFuncWndChildID::buttonBrowseID: {
			//初始化资源
			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			IFileSaveDialog* fileSaveDialog = nullptr;
			//创建对话框实例
			hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileSaveDialog));
			if (!SUCCEEDED(hr))break;
			//设置默认文件名
			fileSaveDialog->SetFileName(L"新建压缩包");
			//设置文件后缀
			COMDLG_FILTERSPEC rgSpec[] = { { L"压缩包文件 (*.ya)", L"*.ya" }};
            fileSaveDialog->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
			fileSaveDialog->SetDefaultExtension(L"ya");
			//配置文件对话框选项
			//DWORD dword;
			//hr = fileSaveDialog->GetOptions(&dword);
			//hr = fileSaveDialog->SetOptions(dword);
      		//显示对话框
			hr = fileSaveDialog->Show(hwnd_WndProc);
            //获取用户选择结果
            IShellItem* selectedItem = nullptr;
			hr = fileSaveDialog->GetResult(&selectedItem);
			if (!SUCCEEDED(hr))break;
			LPWSTR filePath = nullptr;
			//获取文件夹路径
			selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
			//设置编辑框文本
			SetWindowText(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), filePath);
			//销毁资源
			selectedItem->Release();
			CoTaskMemFree(filePath);
			fileSaveDialog->Release();
			CoUninitialize();
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

LRESULT ZipFunc::WM_NOTIFY_WndProc()
{	
	//获取消息携带的信息
	LPNMHDR lpnmhdr = (LPNMHDR)lParam_WndProc;
	//来自压缩文件名称编辑框的消息
	if (lpnmhdr->idFrom == (UINT)ZipFuncWndChildID::editFileNameID){
		//判断通知代码
		switch (lpnmhdr->code) {
			case NM_CLICK:case NM_RETURN:{//单击
				
			}
		}
	}
	else {
		//其他未处理的消息使用默认窗口过程处理
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT ZipFunc::WM_PAINT_WndProc(){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd_WndProc, &ps);
	SelectObject(hdc, lTSObject[0]);
	//SetTextColor(hdc, RGB(46, 134, 193));//文字前景色
	TextOut(hdc, 20, 430, _T("压缩文件设置"), wcslen(_T("压缩文件设置")));
	TextOut(hdc, 20, 480, _T("文件名"), wcslen(_T("文件名")));
	TextOut(hdc, 20, 530, _T("压缩格式        .ya"), wcslen(_T("压缩格式        .ya")));
	POINT points1[2] = { {20,425},{965,425} };
	Polyline(hdc, points1, 2);
	POINT points2[2] = { {20,555},{965,555} };
	Polyline(hdc, points2, 2);
	EndPaint(hwnd_WndProc, &ps);
	return 0;
}

LRESULT ZipFunc::WM_CREATE_WndProc(){
	//初始化文件列表数组
	vector<path> temptemp;
	filePathArr.swap(temptemp);
	//创建字体
	lTSObject[0] = CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("楷体")); // 创建一个 楷体 字体，20px 大小
	//创建按钮
	CreateWindowEx(
		0, WC_BUTTON, _T("添加文件"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		605, 380, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFileID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("添加文件夹"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		725, 380, 120, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonSelectFolderID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("删除"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 380, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonDeleteID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("开始压缩"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		745, 560 ,100 ,40 ,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonStartID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("取消"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 560, 100, 40,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonCancelID), hInstance, this
	);
	CreateWindowEx(
		0, WC_BUTTON, _T("浏览"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 475, 100, 30,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::buttonBrowseID), hInstance, this
	);
	//设置字体
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFileID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonSelectFolderID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonDeleteID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonStartID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::buttonBrowseID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//创建编辑框
	CreateWindowEx(
		WS_EX_CLIENTEDGE, WC_EDIT,(current_path().native() + _T("\\新建压缩包") + _T(SUFFIX)).c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
		100,475,740,30,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::editFileNameID), hInstance, this
	);
	//设置编辑控件中的文本提示
	Edit_SetCueBannerText(GetDlgItem(hwnd_WndProc,(int)ZipFuncWndChildID::editFileNameID), _T("请选择压缩文件路径并设定压缩文件名"));
	SendMessage(GetDlgItem(hwnd_WndProc, (int)ZipFuncWndChildID::editFileNameID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//创建已选择文件列表
	HWND selectedFileListHwnd = CreateWindowEx(
		0, WC_LISTVIEW, _T("已选择的文件"), WS_CHILD | WS_VISIBLE |WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS,
		20, 20, 945, 350,
		hwnd_WndProc, HMENU(ZipFuncWndChildID::selectedFileListID), hInstance, this
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


