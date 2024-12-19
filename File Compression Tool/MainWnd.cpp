#include"ZipFunc.h"

//注册主窗口类
ATOM MainWnd::RegisterWndClass()
{
	//实例化窗口类对象---主窗口
	WNDCLASSEX mainWndClass = { 0 };
	mainWndClass.cbSize = sizeof(WNDCLASSEX);
	mainWndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;//类样式
	mainWndClass.lpfnWndProc = StaticWndProc;//窗口过程
	mainWndClass.hInstance = hInstance;//程序实例
	mainWndClass.hbrBackground = HBRUSH(6);//类背景画刷
	mainWndClass.lpszClassName = _T("mainWndClassName");//窗口类名
	mainWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//窗口图标
	return RegisterClassEx(&mainWndClass);
}

//创建主窗口
HWND MainWnd::CreateWnd()
{
	//创建窗口---主窗口
	HWND mainWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("mainWndClassName"), _T("鸭一压"), WS_TILEDWINDOW,
		0.5 * (maxScreenWidth - wndWidth), 0.5 * (maxScreenHeight - wndHeight), wndWidth, wndHeight,
		NULL, NULL, hInstance, this
	);
	//显示窗口
	if (mainWndHwnd) ShowWindow(mainWndHwnd, SW_SHOW);
	return mainWndHwnd;
}

LRESULT MainWnd::WM_COMMAND_WndProc()
{
	HWND hwnd = (HWND)lParam_WndProc;
	//由工具栏控件发来的消息
	if (hwnd == GetDlgItem(hwnd_WndProc,toolBarID)) {
		//通知代码是点击按钮
		if (HIWORD(wParam_WndProc) == BN_CLICKED) {
			//根据点击的按钮不同执行不同功能
			switch (LOWORD(wParam_WndProc)) {
			case buttonOpenID_ToolBar://打开
			{
				TestMessageBox();
				break;
			}
			case buttonPreviewID_ToolBar://预览
			{

				break;
			}
			case buttonZipID_ToolBar://压缩
			{
				ZipFunc::GetZipFunc().Wnd(true);
				break;
			}
			case buttonUnpackID_ToolBar://解压
			{

				break;
			}
			case buttonSetID_ToolBar://设置
			{

				break;
			}
			}
		}
	}
	else {
		//其他未处理的消息使用默认窗口过程处理
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT MainWnd::WM_NOTIFY_WndProc()
{
	//获取消息携带的信息
	LPNMHDR lpnmhdr = (LPNMHDR)lParam_WndProc;

	//由工具栏控件发来的消息
	if (lpnmhdr->idFrom == toolBarID){
		//判断通知代码
		switch (lpnmhdr->code) {
		case NM_CUSTOMDRAW: //通知控件的父窗口自定义绘图操作
		{
			//获取通知携带的相关信息
			LPNMTBCUSTOMDRAW lpnmtbcd = (LPNMTBCUSTOMDRAW)lParam_WndProc;

			//判断当前绘图阶段
			switch (lpnmtbcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT: //在绘制周期开始之前
			{
				//创建画刷
				tempObject = CreateSolidBrush(RGB(45, 138, 221));
				//自定义工具栏背景
				FillRect(lpnmtbcd->nmcd.hdc, &lpnmtbcd->nmcd.rc, (HBRUSH)tempObject);
				//销毁画刷释放资源
				DeleteObject(tempObject);
				tempObject = NULL;
				//通知工具栏的所有子项进行自定义绘制
				return CDRF_NOTIFYITEMDRAW;
			}
			case CDDS_ITEMPREPAINT://在绘制项(按钮)之前
			{
				RECT tempRC = lpnmtbcd->nmcd.rc;
				tempRC.left += 4;
				tempRC.right -= 0;
				tempRC.top += 4;
				tempRC.bottom -= 1;

				//设置点击按钮时的背景色
				if (SendMessage(lpnmhdr->hwndFrom, TB_ISBUTTONPRESSED, lpnmtbcd->nmcd.dwItemSpec, 0) && (lpnmtbcd->nmcd.uItemState & CDIS_HOT)) {
					//创建画刷
					tempObject = CreateSolidBrush(RGB(12, 74, 129));
					//填充按钮颜色
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//销毁画刷释放资源
					DeleteObject(tempObject);
					tempObject = NULL;
				}
				else if ((lpnmtbcd->nmcd.uItemState & CDIS_HOT) || (lpnmtbcd->nmcd.uItemState & CDIS_MARKED)) //检查热追踪状态
				{
					//创建画刷
					tempObject = CreateSolidBrush(RGB(17, 99, 172));
					//设置热追踪背景色
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//销毁画刷释放资源
					DeleteObject(tempObject);
					tempObject = NULL;
					//如果被标记则取消标记状态
					//if (lpnmtbcd->nmcd.uItemState & CDIS_MARKED)SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, lpnmtbcd->nmcd.dwItemSpec, FALSE);
				}
				else {
					//设置默认背景色
					//FillRect(lpnmtbcd->nmcd.hdc, &lpnmtbcd->nmcd.rc, tempObject);
				}
				//获取当前将要绘制的按钮的样式信息
				TBBUTTONINFO tbbInfo;
				tbbInfo.cbSize = sizeof(TBBUTTONINFO);
				tbbInfo.dwMask = TBIF_STYLE;
				SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONINFO, lpnmtbcd->nmcd.dwItemSpec, LPARAM(&tbbInfo));
				// 如果样式是BTNS_DROPDOWN，绘制分隔符与下拉箭头部分
				if (tbbInfo.fsStyle & BTNS_DROPDOWN) {
					// 绘制分隔符的那条竖线
					//箭头区域宽度为16
					tempRC.left = lpnmtbcd->nmcd.rc.right - 17;
					tempRC.right = tempRC.left + 1;
					//创建画刷
					tempObject = CreateSolidBrush(RGB(45, 138, 221));
					//绘制与工具栏背景相同颜色的竖线
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//销毁画刷释放资源
					DeleteObject(tempObject);
					tempObject = NULL;
					//下拉箭头所在的区域
					tempRC = { lpnmtbcd->nmcd.rc.right - 16, lpnmtbcd->nmcd.rc.top, lpnmtbcd->nmcd.rc.right, lpnmtbcd->nmcd.rc.bottom };
					//箭头所处的中心点
					int centerX = (tempRC.left + tempRC.right) / 2;
					int centerY = (tempRC.top + tempRC.bottom) / 2;
					//构建倒三角的顶点坐标
					POINT points[] = {
						{centerX - 4, centerY - 2},//左上角
						{centerX, centerY + 2},//底部
						{centerX + 5, centerY - 3 }//右上角
					};
					//创建白色画笔
					tempObject = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					SelectObject(lpnmtbcd->nmcd.hdc, tempObject);
					//绘制箭头(折线)
					Polyline(lpnmtbcd->nmcd.hdc, points, 3);
					//销毁画笔释放资源
					DeleteObject(tempObject);
					tempObject = NULL;
				}

				//创建字体
				static HFONT newFont = CreateFont(
					35, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					DEFAULT_PITCH | FF_SWISS, _T("楷体")); // 创建一个 楷体 字体，35px 大小
				// 设置字体和文字颜色
				defFont = newFont;
				HFONT oldFont = (HFONT)SelectObject(lpnmtbcd->nmcd.hdc, defFont);//选择设置自定义字体
				SetTextColor(lpnmtbcd->nmcd.hdc, RGB(255, 255, 255));//文字颜色
				SetBkMode(lpnmtbcd->nmcd.hdc, TRANSPARENT);//背景透明

				// 自定义绘制文本（按钮文本）
				TCHAR buttonText[20];
				SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONTEXT, lpnmtbcd->nmcd.dwItemSpec, (LPARAM)buttonText);
				DrawText(lpnmtbcd->nmcd.hdc, buttonText, -1, &lpnmtbcd->nmcd.rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				//恢复字体
				SelectObject(lpnmtbcd->nmcd.hdc, oldFont);
				return CDRF_SKIPDEFAULT; //跳过默认绘制逻辑
			}
			}
			break;
		}
		case NM_CLICK://在工具栏控件的区域松开已经按下的鼠标后发送的消息
		{
			LPNMMOUSE nmouse = (LPNMMOUSE)lParam_WndProc;

			//获取当前松开鼠标位置的按钮的样式信息
			TBBUTTONINFO tbbInfo;
			tbbInfo.cbSize = sizeof(TBBUTTONINFO);
			tbbInfo.dwMask = TBIF_STYLE;
			SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONINFO, nmouse->dwItemSpec, LPARAM(&tbbInfo));
			//获取索引
			int index = SendMessage(lpnmhdr->hwndFrom, TB_COMMANDTOINDEX, nmouse->dwItemSpec, 0);
			//如果样式是BTNS_DROPDOWN且之前点击的时候是在点击下拉箭头
			if ((tbbInfo.fsStyle & BTNS_DROPDOWN) && SendMessage(lpnmhdr->hwndFrom, TB_ISBUTTONPRESSED, nmouse->dwItemSpec, 0)) {
				//检索按钮的区域
				RECT rc;
				SendMessage(lpnmhdr->hwndFrom, TB_GETITEMRECT, index, (LPARAM)&rc);
				//设定松开按下状态和标记状态
				SendMessage(lpnmhdr->hwndFrom, TB_PRESSBUTTON, nmouse->dwItemSpec, FALSE);
				SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, nmouse->dwItemSpec, TRUE);
				//重绘该按钮
				InvalidateRect(lpnmhdr->hwndFrom, &rc, TRUE);
				//如果松开鼠标的位置在下拉箭头区域内
				if (nmouse->pt.x >= rc.right - 16 && nmouse->pt.x <= rc.right) {
					//创建下拉菜单(为static变量，只创建一次)
					static HMENU hMenu = CreatePopupMenu();
					if (!GetMenuItemCount(hMenu)) {
						AppendMenu(hMenu, MF_STRING, firstPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("选项 1"));
						AppendMenu(hMenu, MF_STRING, secondPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("选项 2"));
						AppendMenu(hMenu, MF_STRING, thirdPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("选项 3"));
					}
					// 转换为屏幕坐标
					MapWindowPoints(lpnmhdr->hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);
					// 显示菜单
					int choice = TrackPopupMenu(
						hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
						rc.left + 4, rc.bottom, 0, hwnd_WndProc, NULL);
					//销毁菜单
					//DestroyMenu(hMenu);
					// 处理菜单命令
					if (choice) {
						switch (choice) {
						case firstPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("选项 1 被点击"), _T("信息"), MB_OK);
							break;
						case secondPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("选项 2 被点击"), _T("信息"), MB_OK);
							break;
						case thirdPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("选项 3 被点击"), _T("信息"), MB_OK);
							break;
						}
					}
				}
				//取消标记状态
				SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, nmouse->dwItemSpec, FALSE);
				//重绘该按钮
				InvalidateRect(lpnmhdr->hwndFrom, &rc, TRUE);
			}
			else {
				return FALSE;//默认处理单击
			}
			//鼠标单击已处理
			return TRUE;
		}
		
		case TBN_DROPDOWN: //当用户单击下拉按钮时，由工具栏控件发送
		{
			//获取通知携带的相关信息
			LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR)lParam_WndProc;
			//设定按下状态
			SendMessage(lpnmhdr->hwndFrom, TB_PRESSBUTTON, lpnmtb->iItem, TRUE);//检测 TB_ISBUTTONPRESSED
			//重绘该按钮
			InvalidateRect(lpnmhdr->hwndFrom, &lpnmtb->rcButton, TRUE);
			//SendMessage(lpnmhdr->hwndFrom, TB_CHECKBUTTON, lpnmtb->iItem, TRUE);//检测 TB_ISBUTTONCHECKED
			//SendMessage(lpnmhdr->hwndFrom, TB_INDETERMINATE, lpnmtb->iItem, TRUE);//检测 TB_ISBUTTONINDETERMINATE
			
			//已处理下拉列表
			return TBDDRET_DEFAULT;
		}
		}
	}
	else {
		//其他未处理的消息使用默认窗口过程处理
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	//前面经过处理后没有 return 直接 break 出来的采用统一 return 0 防止粗心大意
	return 0;
}

//限制窗口最小尺寸
LRESULT MainWnd::WM_WINDOWPOSCHANGING_WndProc()
{
	//设定临时指针接收附加信息
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	if (temp->cx < 960) {
		temp->cx = 960;
	}//如果新的窗口宽度小于960（像素），设定为960
	if (temp->cy < 540) {
		temp->cy = 540;
	}//如果新的窗口高度小于540（像素），设定为540
	return 0;
}

//统一自适应子窗口位置/大小与主窗口相匹配
LRESULT MainWnd::WM_WINDOWPOSCHANGED_WndProc()
{
	//设定临时指针接收附加信息
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	//创建lParam用于在枚举子窗口时给回调函数传参
	LPARAM lParam = (LPARAM)this;//使lParam存储this指针，从而之后强转为 MyWnds * 获取数据
	EnumChildWindows(hwnd_WndProc, StaticEnumChildProc, lParam);
	wndWidth = temp->cx;//更新窗口宽度
	wndHeight = temp->cy;//更新窗口高度
	return 0;
}

//创建主窗口
LRESULT MainWnd::WM_CREATE_WndProc() {
	//创建工具栏
	HWND toolBarHwnd = CreateWindowEx(0, TOOLBARCLASSNAME, _T("工具栏"), WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_LIST,
		0, 0, wndWidth, int(0.1 * wndHeight),
		hwnd_WndProc, HMENU(toolBarID), hInstance, this);
	if (!toolBarHwnd)ErrorMessageBox(hwnd_WndProc, _T("创建工具栏toolBar失败"));
	//设定扩展样式
	SendMessage(toolBarHwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	//创建按钮
	TBBUTTON buttons[5] =
	{
		{I_IMAGENONE , buttonOpenID_ToolBar    ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("打开")},//打开
		{I_IMAGENONE , buttonPreviewID_ToolBar ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("预览")},//预览
		{I_IMAGENONE , buttonZipID_ToolBar     ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("压缩")},//压缩
		{I_IMAGENONE , buttonUnpackID_ToolBar  ,TBSTATE_ENABLED,BTNS_DROPDOWN, {0},0,(INT_PTR)_T("解压")},//解压
		{I_IMAGENONE , buttonSetID_ToolBar     ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("设置")}//设置
	};
	//添加按钮
	SendMessage(toolBarHwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessage(toolBarHwnd, TB_ADDBUTTONS, sizeof(buttons) / sizeof(TBBUTTON), (LPARAM)buttons);
	//设定按钮大小
	SendMessage(toolBarHwnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(140, 70));
	//自动调整尺寸
	SendMessage(toolBarHwnd, TB_AUTOSIZE, 0, 0);
	//显示工具栏
	ShowWindow(toolBarHwnd, SW_SHOWNORMAL);

	//获取工具栏尺寸
	RECT rect,rc;
	GetWindowRect(toolBarHwnd, &rect);
	//将子窗口坐标转换为相对于父窗口的坐标
	POINT point = { rect.right,rect.bottom };
	ScreenToClient(hwnd_WndProc, &point);
	//获取主窗口工作区尺寸
	GetClientRect(hwnd_WndProc, &rc);
	//创建文件列表
	HWND fileListViewHwnd = CreateWindowEx(0, WC_LISTVIEW, _T("文件列表"), WS_CHILD| WS_BORDER| LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE,
		0, point.y, point.x, rc.bottom-rc.top- point.y,
		hwnd_WndProc, HMENU(fileListID), hInstance, this);
	if (!fileListViewHwnd)ErrorMessageBox(hwnd_WndProc, _T("创建文件列表框fileListView失败"));
	//设置扩展样式
	ListView_SetExtendedListViewStyle(fileListViewHwnd, LVS_EX_COLUMNSNAPPOINTS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES| LVS_EX_DOUBLEBUFFER);
	//插入列
	LVCOLUMN column = { 0 };
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_MINWIDTH;
	column.cx = 0.15 * point.x;
	column.pszText = (LPTSTR)_T("名称");
	column.iSubItem = (int)FileListColumnID::columnNameID;
	column.cxMin = 0.15 * point.x;
	ListView_InsertColumn(fileListViewHwnd, FileListColumnID::columnNameID, &column);

	column.fmt = LVCFMT_CENTER;
	column.cx = 0.15 * point.x;
	column.pszText = (LPTSTR)_T("类型");
	column.iSubItem = (int)FileListColumnID::columnTypeID;
	column.cxMin = 0.15 * point.x;
	ListView_InsertColumn(fileListViewHwnd, FileListColumnID::columnTypeID, &column);

	return 0;
}

//关闭窗口
LRESULT MainWnd::WM_CLOSE_WndProc() {
	DestroyWindow(hwnd_WndProc);//销毁窗口并发送WM_DESTROY消息
	return 0;
}

//销毁窗口
LRESULT MainWnd::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//发布WM_QUIT消息
	return 0;
}

//枚举子窗口过程
BOOL MainWnd::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	//自动调整同步子窗口大小
	if(hwndChild == GetDlgItem(hwnd_WndProc, toolBarID)) SendMessage(hwndChild, TB_AUTOSIZE, 0, 0);
	else if (hwndChild == GetDlgItem(hwnd_WndProc, fileListID)) {
		//获取工具栏尺寸
		RECT rect, rc;
		GetWindowRect(GetDlgItem(hwnd_WndProc, toolBarID), &rect);
		//将子窗口坐标转换为相对于父窗口的坐标
		POINT point = { rect.right,rect.bottom };
		ScreenToClient(hwnd_WndProc, &point);
		//获取主窗口工作区尺寸
		GetClientRect(hwnd_WndProc, &rc);
		MoveWindow(hwndChild, 0, point.y, point.x, rc.bottom - rc.top - point.y, TRUE);
	}
	return TRUE;
}