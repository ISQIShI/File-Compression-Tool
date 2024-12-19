#include"ZipFunc.h"

//ע����������
ATOM MainWnd::RegisterWndClass()
{
	//ʵ�������������---������
	WNDCLASSEX mainWndClass = { 0 };
	mainWndClass.cbSize = sizeof(WNDCLASSEX);
	mainWndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;//����ʽ
	mainWndClass.lpfnWndProc = StaticWndProc;//���ڹ���
	mainWndClass.hInstance = hInstance;//����ʵ��
	mainWndClass.hbrBackground = HBRUSH(6);//�౳����ˢ
	mainWndClass.lpszClassName = _T("mainWndClassName");//��������
	mainWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//����ͼ��
	return RegisterClassEx(&mainWndClass);
}

//����������
HWND MainWnd::CreateWnd()
{
	//��������---������
	HWND mainWndHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT | WS_EX_ACCEPTFILES, _T("mainWndClassName"), _T("Ѽһѹ"), WS_TILEDWINDOW,
		0.5 * (maxScreenWidth - wndWidth), 0.5 * (maxScreenHeight - wndHeight), wndWidth, wndHeight,
		NULL, NULL, hInstance, this
	);
	//��ʾ����
	if (mainWndHwnd) ShowWindow(mainWndHwnd, SW_SHOW);
	return mainWndHwnd;
}

LRESULT MainWnd::WM_COMMAND_WndProc()
{
	HWND hwnd = (HWND)lParam_WndProc;
	//�ɹ������ؼ���������Ϣ
	if (hwnd == GetDlgItem(hwnd_WndProc,toolBarID)) {
		//֪ͨ�����ǵ����ť
		if (HIWORD(wParam_WndProc) == BN_CLICKED) {
			//���ݵ���İ�ť��ִͬ�в�ͬ����
			switch (LOWORD(wParam_WndProc)) {
			case buttonOpenID_ToolBar://��
			{
				TestMessageBox();
				break;
			}
			case buttonPreviewID_ToolBar://Ԥ��
			{

				break;
			}
			case buttonZipID_ToolBar://ѹ��
			{
				ZipFunc::GetZipFunc().Wnd(true);
				break;
			}
			case buttonUnpackID_ToolBar://��ѹ
			{

				break;
			}
			case buttonSetID_ToolBar://����
			{

				break;
			}
			}
		}
	}
	else {
		//����δ�������Ϣʹ��Ĭ�ϴ��ڹ��̴���
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	return 0;
}

LRESULT MainWnd::WM_NOTIFY_WndProc()
{
	//��ȡ��ϢЯ������Ϣ
	LPNMHDR lpnmhdr = (LPNMHDR)lParam_WndProc;

	//�ɹ������ؼ���������Ϣ
	if (lpnmhdr->idFrom == toolBarID){
		//�ж�֪ͨ����
		switch (lpnmhdr->code) {
		case NM_CUSTOMDRAW: //֪ͨ�ؼ��ĸ������Զ����ͼ����
		{
			//��ȡ֪ͨЯ���������Ϣ
			LPNMTBCUSTOMDRAW lpnmtbcd = (LPNMTBCUSTOMDRAW)lParam_WndProc;

			//�жϵ�ǰ��ͼ�׶�
			switch (lpnmtbcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT: //�ڻ������ڿ�ʼ֮ǰ
			{
				//������ˢ
				tempObject = CreateSolidBrush(RGB(45, 138, 221));
				//�Զ��幤��������
				FillRect(lpnmtbcd->nmcd.hdc, &lpnmtbcd->nmcd.rc, (HBRUSH)tempObject);
				//���ٻ�ˢ�ͷ���Դ
				DeleteObject(tempObject);
				tempObject = NULL;
				//֪ͨ��������������������Զ������
				return CDRF_NOTIFYITEMDRAW;
			}
			case CDDS_ITEMPREPAINT://�ڻ�����(��ť)֮ǰ
			{
				RECT tempRC = lpnmtbcd->nmcd.rc;
				tempRC.left += 4;
				tempRC.right -= 0;
				tempRC.top += 4;
				tempRC.bottom -= 1;

				//���õ����ťʱ�ı���ɫ
				if (SendMessage(lpnmhdr->hwndFrom, TB_ISBUTTONPRESSED, lpnmtbcd->nmcd.dwItemSpec, 0) && (lpnmtbcd->nmcd.uItemState & CDIS_HOT)) {
					//������ˢ
					tempObject = CreateSolidBrush(RGB(12, 74, 129));
					//��䰴ť��ɫ
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//���ٻ�ˢ�ͷ���Դ
					DeleteObject(tempObject);
					tempObject = NULL;
				}
				else if ((lpnmtbcd->nmcd.uItemState & CDIS_HOT) || (lpnmtbcd->nmcd.uItemState & CDIS_MARKED)) //�����׷��״̬
				{
					//������ˢ
					tempObject = CreateSolidBrush(RGB(17, 99, 172));
					//������׷�ٱ���ɫ
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//���ٻ�ˢ�ͷ���Դ
					DeleteObject(tempObject);
					tempObject = NULL;
					//����������ȡ�����״̬
					//if (lpnmtbcd->nmcd.uItemState & CDIS_MARKED)SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, lpnmtbcd->nmcd.dwItemSpec, FALSE);
				}
				else {
					//����Ĭ�ϱ���ɫ
					//FillRect(lpnmtbcd->nmcd.hdc, &lpnmtbcd->nmcd.rc, tempObject);
				}
				//��ȡ��ǰ��Ҫ���Ƶİ�ť����ʽ��Ϣ
				TBBUTTONINFO tbbInfo;
				tbbInfo.cbSize = sizeof(TBBUTTONINFO);
				tbbInfo.dwMask = TBIF_STYLE;
				SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONINFO, lpnmtbcd->nmcd.dwItemSpec, LPARAM(&tbbInfo));
				// �����ʽ��BTNS_DROPDOWN�����Ʒָ�����������ͷ����
				if (tbbInfo.fsStyle & BTNS_DROPDOWN) {
					// ���Ʒָ�������������
					//��ͷ������Ϊ16
					tempRC.left = lpnmtbcd->nmcd.rc.right - 17;
					tempRC.right = tempRC.left + 1;
					//������ˢ
					tempObject = CreateSolidBrush(RGB(45, 138, 221));
					//�����빤����������ͬ��ɫ������
					FillRect(lpnmtbcd->nmcd.hdc, &tempRC, (HBRUSH)tempObject);
					//���ٻ�ˢ�ͷ���Դ
					DeleteObject(tempObject);
					tempObject = NULL;
					//������ͷ���ڵ�����
					tempRC = { lpnmtbcd->nmcd.rc.right - 16, lpnmtbcd->nmcd.rc.top, lpnmtbcd->nmcd.rc.right, lpnmtbcd->nmcd.rc.bottom };
					//��ͷ���������ĵ�
					int centerX = (tempRC.left + tempRC.right) / 2;
					int centerY = (tempRC.top + tempRC.bottom) / 2;
					//���������ǵĶ�������
					POINT points[] = {
						{centerX - 4, centerY - 2},//���Ͻ�
						{centerX, centerY + 2},//�ײ�
						{centerX + 5, centerY - 3 }//���Ͻ�
					};
					//������ɫ����
					tempObject = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					SelectObject(lpnmtbcd->nmcd.hdc, tempObject);
					//���Ƽ�ͷ(����)
					Polyline(lpnmtbcd->nmcd.hdc, points, 3);
					//���ٻ����ͷ���Դ
					DeleteObject(tempObject);
					tempObject = NULL;
				}

				//��������
				static HFONT newFont = CreateFont(
					35, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
					DEFAULT_PITCH | FF_SWISS, _T("����")); // ����һ�� ���� ���壬35px ��С
				// ���������������ɫ
				defFont = newFont;
				HFONT oldFont = (HFONT)SelectObject(lpnmtbcd->nmcd.hdc, defFont);//ѡ�������Զ�������
				SetTextColor(lpnmtbcd->nmcd.hdc, RGB(255, 255, 255));//������ɫ
				SetBkMode(lpnmtbcd->nmcd.hdc, TRANSPARENT);//����͸��

				// �Զ�������ı�����ť�ı���
				TCHAR buttonText[20];
				SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONTEXT, lpnmtbcd->nmcd.dwItemSpec, (LPARAM)buttonText);
				DrawText(lpnmtbcd->nmcd.hdc, buttonText, -1, &lpnmtbcd->nmcd.rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				//�ָ�����
				SelectObject(lpnmtbcd->nmcd.hdc, oldFont);
				return CDRF_SKIPDEFAULT; //����Ĭ�ϻ����߼�
			}
			}
			break;
		}
		case NM_CLICK://�ڹ������ؼ��������ɿ��Ѿ����µ������͵���Ϣ
		{
			LPNMMOUSE nmouse = (LPNMMOUSE)lParam_WndProc;

			//��ȡ��ǰ�ɿ����λ�õİ�ť����ʽ��Ϣ
			TBBUTTONINFO tbbInfo;
			tbbInfo.cbSize = sizeof(TBBUTTONINFO);
			tbbInfo.dwMask = TBIF_STYLE;
			SendMessage(lpnmhdr->hwndFrom, TB_GETBUTTONINFO, nmouse->dwItemSpec, LPARAM(&tbbInfo));
			//��ȡ����
			int index = SendMessage(lpnmhdr->hwndFrom, TB_COMMANDTOINDEX, nmouse->dwItemSpec, 0);
			//�����ʽ��BTNS_DROPDOWN��֮ǰ�����ʱ�����ڵ��������ͷ
			if ((tbbInfo.fsStyle & BTNS_DROPDOWN) && SendMessage(lpnmhdr->hwndFrom, TB_ISBUTTONPRESSED, nmouse->dwItemSpec, 0)) {
				//������ť������
				RECT rc;
				SendMessage(lpnmhdr->hwndFrom, TB_GETITEMRECT, index, (LPARAM)&rc);
				//�趨�ɿ�����״̬�ͱ��״̬
				SendMessage(lpnmhdr->hwndFrom, TB_PRESSBUTTON, nmouse->dwItemSpec, FALSE);
				SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, nmouse->dwItemSpec, TRUE);
				//�ػ�ð�ť
				InvalidateRect(lpnmhdr->hwndFrom, &rc, TRUE);
				//����ɿ�����λ����������ͷ������
				if (nmouse->pt.x >= rc.right - 16 && nmouse->pt.x <= rc.right) {
					//���������˵�(Ϊstatic������ֻ����һ��)
					static HMENU hMenu = CreatePopupMenu();
					if (!GetMenuItemCount(hMenu)) {
						AppendMenu(hMenu, MF_STRING, firstPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("ѡ�� 1"));
						AppendMenu(hMenu, MF_STRING, secondPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("ѡ�� 2"));
						AppendMenu(hMenu, MF_STRING, thirdPopUpMenuOptionID_ButtonUnpack_ToolBar, _T("ѡ�� 3"));
					}
					// ת��Ϊ��Ļ����
					MapWindowPoints(lpnmhdr->hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);
					// ��ʾ�˵�
					int choice = TrackPopupMenu(
						hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
						rc.left + 4, rc.bottom, 0, hwnd_WndProc, NULL);
					//���ٲ˵�
					//DestroyMenu(hMenu);
					// ����˵�����
					if (choice) {
						switch (choice) {
						case firstPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("ѡ�� 1 �����"), _T("��Ϣ"), MB_OK);
							break;
						case secondPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("ѡ�� 2 �����"), _T("��Ϣ"), MB_OK);
							break;
						case thirdPopUpMenuOptionID_ButtonUnpack_ToolBar:
							MessageBox(hwnd_WndProc, _T("ѡ�� 3 �����"), _T("��Ϣ"), MB_OK);
							break;
						}
					}
				}
				//ȡ�����״̬
				SendMessage(lpnmhdr->hwndFrom, TB_MARKBUTTON, nmouse->dwItemSpec, FALSE);
				//�ػ�ð�ť
				InvalidateRect(lpnmhdr->hwndFrom, &rc, TRUE);
			}
			else {
				return FALSE;//Ĭ�ϴ�����
			}
			//��굥���Ѵ���
			return TRUE;
		}
		
		case TBN_DROPDOWN: //���û�����������ťʱ���ɹ������ؼ�����
		{
			//��ȡ֪ͨЯ���������Ϣ
			LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR)lParam_WndProc;
			//�趨����״̬
			SendMessage(lpnmhdr->hwndFrom, TB_PRESSBUTTON, lpnmtb->iItem, TRUE);//��� TB_ISBUTTONPRESSED
			//�ػ�ð�ť
			InvalidateRect(lpnmhdr->hwndFrom, &lpnmtb->rcButton, TRUE);
			//SendMessage(lpnmhdr->hwndFrom, TB_CHECKBUTTON, lpnmtb->iItem, TRUE);//��� TB_ISBUTTONCHECKED
			//SendMessage(lpnmhdr->hwndFrom, TB_INDETERMINATE, lpnmtb->iItem, TRUE);//��� TB_ISBUTTONINDETERMINATE
			
			//�Ѵ��������б�
			return TBDDRET_DEFAULT;
		}
		}
	}
	else {
		//����δ�������Ϣʹ��Ĭ�ϴ��ڹ��̴���
		return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
	}
	//ǰ�澭�������û�� return ֱ�� break �����Ĳ���ͳһ return 0 ��ֹ���Ĵ���
	return 0;
}

//���ƴ�����С�ߴ�
LRESULT MainWnd::WM_WINDOWPOSCHANGING_WndProc()
{
	//�趨��ʱָ����ո�����Ϣ
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	if (temp->cx < 960) {
		temp->cx = 960;
	}//����µĴ��ڿ��С��960�����أ����趨Ϊ960
	if (temp->cy < 540) {
		temp->cy = 540;
	}//����µĴ��ڸ߶�С��540�����أ����趨Ϊ540
	return 0;
}

//ͳһ����Ӧ�Ӵ���λ��/��С����������ƥ��
LRESULT MainWnd::WM_WINDOWPOSCHANGED_WndProc()
{
	//�趨��ʱָ����ո�����Ϣ
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	//����lParam������ö���Ӵ���ʱ���ص���������
	LPARAM lParam = (LPARAM)this;//ʹlParam�洢thisָ�룬�Ӷ�֮��ǿתΪ MyWnds * ��ȡ����
	EnumChildWindows(hwnd_WndProc, StaticEnumChildProc, lParam);
	wndWidth = temp->cx;//���´��ڿ��
	wndHeight = temp->cy;//���´��ڸ߶�
	return 0;
}

//����������
LRESULT MainWnd::WM_CREATE_WndProc() {
	//����������
	HWND toolBarHwnd = CreateWindowEx(0, TOOLBARCLASSNAME, _T("������"), WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_LIST,
		0, 0, wndWidth, int(0.1 * wndHeight),
		hwnd_WndProc, HMENU(toolBarID), hInstance, this);
	if (!toolBarHwnd)ErrorMessageBox(hwnd_WndProc, _T("����������toolBarʧ��"));
	//�趨��չ��ʽ
	SendMessage(toolBarHwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	//������ť
	TBBUTTON buttons[5] =
	{
		{I_IMAGENONE , buttonOpenID_ToolBar    ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("��")},//��
		{I_IMAGENONE , buttonPreviewID_ToolBar ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("Ԥ��")},//Ԥ��
		{I_IMAGENONE , buttonZipID_ToolBar     ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("ѹ��")},//ѹ��
		{I_IMAGENONE , buttonUnpackID_ToolBar  ,TBSTATE_ENABLED,BTNS_DROPDOWN, {0},0,(INT_PTR)_T("��ѹ")},//��ѹ
		{I_IMAGENONE , buttonSetID_ToolBar     ,TBSTATE_ENABLED,BTNS_BUTTON,   {0},0,(INT_PTR)_T("����")}//����
	};
	//��Ӱ�ť
	SendMessage(toolBarHwnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
	SendMessage(toolBarHwnd, TB_ADDBUTTONS, sizeof(buttons) / sizeof(TBBUTTON), (LPARAM)buttons);
	//�趨��ť��С
	SendMessage(toolBarHwnd, TB_SETBUTTONSIZE, 0, MAKELPARAM(140, 70));
	//�Զ������ߴ�
	SendMessage(toolBarHwnd, TB_AUTOSIZE, 0, 0);
	//��ʾ������
	ShowWindow(toolBarHwnd, SW_SHOWNORMAL);

	//��ȡ�������ߴ�
	RECT rect,rc;
	GetWindowRect(toolBarHwnd, &rect);
	//���Ӵ�������ת��Ϊ����ڸ����ڵ�����
	POINT point = { rect.right,rect.bottom };
	ScreenToClient(hwnd_WndProc, &point);
	//��ȡ�����ڹ������ߴ�
	GetClientRect(hwnd_WndProc, &rc);
	//�����ļ��б�
	HWND fileListViewHwnd = CreateWindowEx(0, WC_LISTVIEW, _T("�ļ��б�"), WS_CHILD| WS_BORDER| LVS_REPORT | LVS_SHOWSELALWAYS | WS_VISIBLE,
		0, point.y, point.x, rc.bottom-rc.top- point.y,
		hwnd_WndProc, HMENU(fileListID), hInstance, this);
	if (!fileListViewHwnd)ErrorMessageBox(hwnd_WndProc, _T("�����ļ��б��fileListViewʧ��"));
	//������չ��ʽ
	ListView_SetExtendedListViewStyle(fileListViewHwnd, LVS_EX_COLUMNSNAPPOINTS | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES| LVS_EX_DOUBLEBUFFER);
	//������
	LVCOLUMN column = { 0 };
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_MINWIDTH;
	column.cx = 0.15 * point.x;
	column.pszText = (LPTSTR)_T("����");
	column.iSubItem = (int)FileListColumnID::columnNameID;
	column.cxMin = 0.15 * point.x;
	ListView_InsertColumn(fileListViewHwnd, FileListColumnID::columnNameID, &column);

	column.fmt = LVCFMT_CENTER;
	column.cx = 0.15 * point.x;
	column.pszText = (LPTSTR)_T("����");
	column.iSubItem = (int)FileListColumnID::columnTypeID;
	column.cxMin = 0.15 * point.x;
	ListView_InsertColumn(fileListViewHwnd, FileListColumnID::columnTypeID, &column);

	return 0;
}

//�رմ���
LRESULT MainWnd::WM_CLOSE_WndProc() {
	DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
	return 0;
}

//���ٴ���
LRESULT MainWnd::WM_DESTROY_WndProc()
{
	PostQuitMessage(0);//����WM_QUIT��Ϣ
	return 0;
}

//ö���Ӵ��ڹ���
BOOL MainWnd::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	//�Զ�����ͬ���Ӵ��ڴ�С
	if(hwndChild == GetDlgItem(hwnd_WndProc, toolBarID)) SendMessage(hwndChild, TB_AUTOSIZE, 0, 0);
	else if (hwndChild == GetDlgItem(hwnd_WndProc, fileListID)) {
		//��ȡ�������ߴ�
		RECT rect, rc;
		GetWindowRect(GetDlgItem(hwnd_WndProc, toolBarID), &rect);
		//���Ӵ�������ת��Ϊ����ڸ����ڵ�����
		POINT point = { rect.right,rect.bottom };
		ScreenToClient(hwnd_WndProc, &point);
		//��ȡ�����ڹ������ߴ�
		GetClientRect(hwnd_WndProc, &rc);
		MoveWindow(hwndChild, 0, point.y, point.x, rc.bottom - rc.top - point.y, TRUE);
	}
	return TRUE;
}