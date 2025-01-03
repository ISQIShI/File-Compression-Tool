#include"UnpackFunc.h"
#include<ShObjIdl_core.h>
#include"ThreadPool.h"
#include "FileService.h"
#include "HuffmanCode.h"

void UnpackFunc::StartUnpack()
{
	//初始化线程池
	size_t maxThreadAmount = 20;//临时占位 之后更改为用户设定的最大线程数
	size_t availableThreadAmount = thread::hardware_concurrency();
	ThreadPool threadPool(availableThreadAmount < maxThreadAmount ? availableThreadAmount : maxThreadAmount);
	//开始解压进行计时
	auto startTime = std::chrono::high_resolution_clock::now();
	//获取范式符号-编码表
	HuffmanCode::GetNormalSymbolCode(zipFile->codeLength, zipFile->symbolCode);
	if (!zipFile->symbolCode.empty()) {
		BYTE tempCodeLength = 0;
		//获取 编码长度-编码-符号 表
		for (auto& codeLength : zipFile->codeLength) {
			if (codeLength.second != tempCodeLength) {
				codeSymbol->emplace_back().first = codeLength.second;
				tempCodeLength = codeLength.second;
			}
			codeSymbol->back().second.emplace(zipFile->symbolCode.at(codeLength.first), codeLength.first);
		}
	}
	//中间计时1
	auto middleTime = std::chrono::high_resolution_clock::now();
	//智能解压相关处理
	if (openIntelligentUnpack) {
		targetPath /= zipFile->zipFilePath.stem();
		while (exists(targetPath))targetPath += "-解压文件";
		create_directories(targetPath);
	}
	//解压所有文件
	if (willUnpackAllFiles) {
		for (auto& internalFile : internalFileArr->at(0)) {
			UnpackObject(internalFile,threadPool);
		}
	}
	else {//解压选中的文件
		size_t x = 0;
		if (folderIndex) ++x;
		UINT state;
		while (x < internalFileArr->at(folderIndex).size()) {
			//遍历所有当前深度文件夹的项
			state = ListView_GetItemState(GetDlgItem(MainWnd::GetMainWnd().GetWndHwnd(),fileListID), x, LVIS_SELECTED);
			//如果项被选中则解压该项
			if (state & LVIS_SELECTED) UnpackObject(internalFileArr->at(folderIndex).at(x),threadPool);
			++x;
		}
	}
	//等待解压完成
	threadPool.WaitTask(1);

	//完成解压结束计时
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(middleTime - startTime);
	auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(endTime - middleTime);
	TCHAR tempTCHAR[50];
	_stprintf_s(tempTCHAR, _T("压缩时间：\n准备工作 %f 秒\n解压文件 %f 秒"), duration1.count() / 1000000.0, duration2.count() / 1000000.0);
	TestMessageBox(hwnd_WndProc, tempTCHAR, _T("解压完成"));
}

void UnpackFunc::UnpackObject(const InternalFileInfo & internalFile,ThreadPool & threadPool)
{
	//文件夹
	if (internalFile.isFolder) {
		//若路径不存在则创建
        if (!exists(targetPath/internalFile.filePath)) {
            create_directories(targetPath/internalFile.filePath);
		}
		//遍历并解压该文件夹内所有项
		for (size_t i = 1; i < internalFileArr->at(internalFile.folderID).size(); ++i) {
			UnpackObject(internalFileArr->at(internalFile.folderID).at(i),threadPool);
		}
	}
	else {//文件
		threadPool.SubmitTask(1, true, &UnpackFunc::UnpackFile, this, internalFile);
	}
}

void UnpackFunc::UnpackFile(const InternalFileInfo& internalFile)
{
	//创建要解压的文件
	if (exists(targetPath / internalFile.filePath)) {
		ErrorMessageBox(hwnd_WndProc, ((targetPath / internalFile.filePath).wstring() + L" 已存在\n请移动该文件或修改解压路径").c_str(), false);
	}
	FileService::ExtendFile(targetPath / internalFile.filePath, internalFile.oldFileSize, true);
	if (internalFile.oldFileSize == 0)return;//空文件返回
	//映射压缩文件中对应的区域
	MapFileInfo* mapZipFileInfo = new MapFileInfo((LPTSTR)zipFile->zipFilePath.c_str(), internalFile.fileOffset, internalFile.WPL_Size.first);
	//进行文件映射
	FileService::MapFile(*mapZipFileInfo, true);
	//获取文件映射指针
	BYTE* zipFilePointer = (BYTE*)mapZipFileInfo->mapViewPointer;
	//映射将要解压的目的文件
	path tempPath = targetPath / internalFile.filePath;
	MapFileInfo* mapTargetFileInfo = new MapFileInfo((LPTSTR)tempPath.c_str(), 0, 0);
	//进行文件映射
	FileService::MapFile(*mapTargetFileInfo);
	//获取文件映射指针
	BYTE* targetFilePointer = (BYTE*)mapTargetFileInfo->mapViewPointer;
	//每次读取的最小比特位数
	BYTE tempIndex = 0;
	bitset<256> buffer = 0;
	bitset<8> tempbuffer;
	BYTE bitCount = 0;
	for (size_t reader = 0; reader < mapZipFileInfo->fileMapSize - 1; ++reader, ++zipFilePointer) {
		tempbuffer = *zipFilePointer;
		//将新读取的数据添加到buffer中
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
	//将新读取的数据添加到buffer中
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
	//通知代码是点击按钮
	if (HIWORD(wParam_WndProc) == BN_CLICKED) {
		//根据点击的按钮不同执行不同功能
		switch (LOWORD(wParam_WndProc)) {
		case (int)UnpackFuncWndChildID::radioButtonAllFilesID: {//点击 全部文件 单选框
			willUnpackAllFiles = true;
			break;
		}
		case (int)UnpackFuncWndChildID::radioButtonSelectedFilesID: {//点击 部分文件 单选框
			willUnpackAllFiles = false;
			break;
		}
		case (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID: {//点击 智能解压 复选框
			// 获取复选框的状态
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
			DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
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

LRESULT UnpackFunc::WM_PAINT_WndProc()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd_WndProc, &ps);
	SelectObject(hdc, lTSObject[0]);
	TextOut(hdc, 20, 20, _T("目标路径"), wcslen(_T("目标路径")));
	TextOut(hdc, 20, 70, _T("解压缩:"), wcslen(_T("解压缩:")));
	EndPaint(hwnd_WndProc, &ps);
	return 0;
}

LRESULT UnpackFunc::WM_CTLCOLORSTATIC_WndProc()
{
	HDC hdc = (HDC)wParam_WndProc;
	SetBkColor(hdc, RGB(255, 255, 255)); // 设置背景为白色
	return (LRESULT)GetStockObject(WHITE_BRUSH);
}

LRESULT UnpackFunc::WM_CREATE_WndProc()
{
	//创建字体
	lTSObject[0] = CreateFont(
		20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("楷体")); // 创建一个 楷体 字体，20px 大小

	//创建按钮
	CreateWindowEx(
		0, WC_BUTTON, _T("浏览"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 15, 100, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonBrowseID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonBrowseID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("确定"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		745, 115, 100, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonConfirmID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonConfirmID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("取消"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
		865, 115, 100, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::buttonCancelID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::buttonCancelID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//创建单选按钮
	CreateWindowEx(
		0, WC_BUTTON, _T("全部文件"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON ,
		170, 65, 110, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::radioButtonAllFilesID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonAllFilesID), BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonAllFilesID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	CreateWindowEx(
		0, WC_BUTTON, _T("选定的文件"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		320, 65, 130, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::radioButtonSelectedFilesID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonSelectedFilesID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	if(!ListView_GetSelectedCount(GetDlgItem(MainWnd::GetMainWnd().GetWndHwnd(),fileListID)))EnableWindow(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::radioButtonSelectedFilesID), FALSE);
	//创建复选按钮
	CreateWindowEx(
		0, WC_BUTTON, _T("智能解压"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		20, 115, 110, 40,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::checkBoxIntelligentUnpackID), hInstance, this
	);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID), BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::checkBoxIntelligentUnpackID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);
	//创建编辑框
	CreateWindowEx(
		WS_EX_CLIENTEDGE, WC_EDIT, zipFile->zipFilePath.parent_path().c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
		115, 15, 725, 30,
		hwnd_WndProc, HMENU(UnpackFuncWndChildID::editFileNameID), hInstance, this
	);
	//设置编辑控件中的文本提示
	Edit_SetCueBannerText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), _T("请选择解压路径"));
	SendMessage(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), WM_SETFONT, (WPARAM)lTSObject[0], TRUE);

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

void UnpackFunc::ClickConfirmButton()
{
	int length = GetWindowTextLength(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID));
	//审查压缩文件路径是否正确
	if (!length) {
		MessageBox(hwnd_WndProc, _T("目标路径为空,请检查解压路径"), _T("鸭一压"), MB_OK | MB_TASKMODAL);
		return;
	}
	LPTSTR fileNameStr = new TCHAR[length + 1];
	GetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), fileNameStr, length + 1);
	//解析为绝对路径
	path fileName = absolute(fileNameStr);
	//设置编辑框文本
	SetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), fileName.c_str());
	if (!exists(fileName.root_path())) {
		MessageBox(hwnd_WndProc, _T("目标路径中根目录不存在,请修改解压路径"), _T("鸭一压"), MB_OK | MB_TASKMODAL);
		return;
	}
	if (!exists(fileName)) {
		if (MessageBox(hwnd_WndProc, (_T("指定的路径不存在,是否新建文件夹?\n-") + fileName.native()).c_str(), _T("鸭一压"), MB_YESNO | MB_TASKMODAL) == IDYES) {
			create_directories(fileName);
		}
		else return;
	}
	if (MessageBox(hwnd_WndProc, (_T("是否确定将文件(文件夹)解压到当前路径?\n-") + fileName.native()).c_str(), _T("鸭一压"), MB_OKCANCEL | MB_TASKMODAL) != IDOK) {
		return;
	}
	targetPath = fileName;
	StartUnpack();
	DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
}

void UnpackFunc::ClickBrowseButton()
{
	//初始化资源
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	IFileOpenDialog* fileOpenDialog = nullptr;
	//创建对话框实例
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpenDialog));
	if (!SUCCEEDED(hr))throw runtime_error("无法创建对话框");
	//配置文件对话框选项
	DWORD dword;
	//设置文件夹多选
	hr = fileOpenDialog->GetOptions(&dword);
	hr = fileOpenDialog->SetOptions(dword | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT);
	//显示对话框
	hr = fileOpenDialog->Show(MainWnd::GetMainWnd().GetWndHwnd());
	//获取用户选择结果
	IShellItem* selectedItem = nullptr;
	hr = fileOpenDialog->GetResult(&selectedItem);
	if (!SUCCEEDED(hr))
	{
		fileOpenDialog->Release();
		CoUninitialize();
		return;
	}
	LPWSTR filePath = nullptr;
	//获取文件夹路径
	selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
	//设置编辑框文本
	SetWindowText(GetDlgItem(hwnd_WndProc, (int)UnpackFuncWndChildID::editFileNameID), filePath);
	//销毁资源
	selectedItem->Release();
	CoTaskMemFree(filePath);
	fileOpenDialog->Release();
	CoUninitialize();
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
	/*if (zipFile->zipFilePath == filePath)
	{
		selectedItem->Release();
		CoTaskMemFree(filePath);
		fileOpenDialog->Release();
		CoUninitialize();
		return false;
	}*/
	//初始化文件列表数组
	if (internalFileArr) {
		delete internalFileArr;
		internalFileArr = new vector<vector<InternalFileInfo>>;
		internalFileArr->resize(1);
		InternalFileInfo::folderAmount = 0;
	}
	//初始化编码符号表
	if (codeSymbol && !codeSymbol->empty()) {
		codeSymbol->clear();
	}
	//初始化文件夹深度
	folderIndex = 0;
	willUnpackAllFiles = true;
	openIntelligentUnpack = true;
	targetPath.clear();
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
		internalFile.filePath = *internalFile.fileName.begin();
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
			sort(beginIt, internalFileArr->at(i).end(), [](const InternalFileInfo& a, const InternalFileInfo& b) {return a.isFolder > b.isFolder; });
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
			InternalFileInfo& temp = internalFileArr->at(folderID).emplace_back(true, *internalFile.fileName.begin(), internalFile.filePath,internalFile.oldFileSize,internalFile.WPL_Size,0 );
			internalFileArr->emplace_back();//创建新的一维容器
			internalFileArr->at(temp.folderID).emplace_back(true, _T("..(返回上一级文件夹)"), _T(""), 0, pair<uintmax_t, BYTE>(0, 0), 0, folderID, true);
			folderID = temp.folderID;
		}
		else {//若存在则增加信息计数
			 it->oldFileSize += internalFile.oldFileSize;
			 it->WPL_Size.first += internalFile.WPL_Size.first;
			 folderID = it->folderID;
		}
		internalFile.fileName = relative(internalFile.fileName, *internalFile.fileName.begin());
		internalFile.filePath /= *internalFile.fileName.begin();
		StoreFileInfo(internalFile, folderID);
	}
	else {//准备存入文件
		internalFileArr->at(folderID).emplace_back(false, internalFile.fileName, internalFile.filePath,internalFile.oldFileSize, internalFile.WPL_Size, internalFile.fileOffset, folderID);
	}
}

