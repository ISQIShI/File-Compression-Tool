#include"MyWnds.h"

HINSTANCE MyWnds::hInstance;
int MyWnds::maxScreenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
int MyWnds::maxScreenHeight = GetSystemMetrics(SM_CYMAXIMIZED);

void MyWnds::ErrorMessageBox(const HWND& hwnd,const TCHAR* msg,bool showErrorCode){
	//�����ַ������ڽ��մ������
	TCHAR errorCode[20] = _T("");
	if (showErrorCode) _stprintf_s(errorCode, _T("\n�������:%lu"), GetLastError());
	//��msg������������ƴ������
	size_t length = _tcslen(msg) + _tcslen(errorCode) + 1;
	TCHAR* finalMsg = new TCHAR[length];
	_stprintf_s(finalMsg, length ,_T("%s%s"), msg, errorCode);
	//�������󵯴�
	MessageBox(hwnd, finalMsg, _T("������Ϣ"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
	//����ʹ��new���ٵĿռ�
	delete[]finalMsg;
	//�˳�����
	exit(GetLastError());
}

void MyWnds::TestMessageBox(const HWND& hwnd, const TCHAR* text, const TCHAR* title, UINT type){
	MessageBox(hwnd, text, title, type);
}

WPARAM MyWnds::MessageLoop(const HWND& hwnd_IsDialogMessage){
	//���ø�����
	if(isModalDialog)EnableWindow(isModalDialog, FALSE);
	MSG Msg = { 0 };
	BOOL bRet = 1;
	while ((bRet = GetMessage(&Msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) ErrorMessageBox(NULL, _T("��Ϣ����ʧ��"));
		else
		{
			if (!IsDialogMessage(hwnd_IsDialogMessage, &Msg)) //�ú������������Ϣ(��TAB�л��ؼ�����)
			{
				//ת�����ַ���Ϣ
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}
	//���ò��ָ��۽���������
	if (isModalDialog) {
		EnableWindow(isModalDialog, TRUE);
		SetFocus(isModalDialog);
	}
	return Msg.wParam;
}

LRESULT CALLBACK MyWnds::StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	//��������(MyWnds)ָ�벢�ÿգ�����֮��ָ������(�̳���MyWnds����)����
	MyWnds* myWnds = nullptr;
	if (uMsg == WM_NCCREATE){
		//�յ� WM_NCCREATE ��Ϣʱ��lParam ��ָ�� CREATESTRUCT �ṹ��ָ�룬�ýṹ�����й��������Ĵ��ڵ���Ϣ��
		CREATESTRUCT* temp = (CREATESTRUCT*)lParam;
		/*
		CREATESTRUCT �ṹ�е� lpCreateParams ��Ա�����洢���ǵ��� CreateWindowEx ��������ʱ����� lpParam ����ֵ
		�Ҵ����������(�̳���MyWnds����)���������ͨ��Ա���� CreateWnd ʱ������ this ָ��
		�����ʵ����ָ�򴴽����ڵ�����(�̳���MyWnds����)�����ָ�룬���Ա�ǿתΪ����(MyWnds)ָ�벢������(MyWnds)ָ�����
		*/
		myWnds = (MyWnds*)temp->lpCreateParams;
		//�����������ָ���봰�ھ��֮��İ󶨹�ϵ
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)myWnds);
	}
	else{
		//ͨ�����ھ�����ҵ������ô��ڵ�����(�̳���MyWnds����)�����ָ�룬���ø���(MyWnds)ָ��ָ������(�̳���MyWnds����)����
		myWnds = (MyWnds*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}

	if (myWnds){
		return myWnds->WndProc(hwnd, uMsg, wParam, lParam);
	}
	else{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK MyWnds::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	hwnd_WndProc = hwnd;
	uMsg_WndProc = uMsg;
	wParam_WndProc = wParam;
	lParam_WndProc = lParam;
	switch (uMsg) {
	case WM_COMMAND:
		return WM_COMMAND_WndProc();
	case WM_NOTIFY:
		return WM_NOTIFY_WndProc();
	case WM_CTLCOLORSTATIC://����static��edit(ES_READONLY)�ؼ�
		return WM_CTLCOLORSTATIC_WndProc();
	case WM_PAINT://���ƴ��ڸ�������
		return WM_PAINT_WndProc();
	case WM_LBUTTONDOWN://����������
		return WM_LBUTTONDOWN_WndProc();
	case WM_LBUTTONUP://�ɿ�������
		return WM_LBUTTONUP_WndProc();
	case WM_WINDOWPOSCHANGING://���ڸ��Ĵ��ڴ�С
		return WM_WINDOWPOSCHANGING_WndProc();
	case WM_WINDOWPOSCHANGED://���ڴ�С��ɸ���
		return WM_WINDOWPOSCHANGED_WndProc();
	case WM_SIZE:
		return WM_SIZE_WndProc();
	case WM_CREATE:
		return WM_CREATE_WndProc();
	case WM_CLOSE://�رմ���
		return WM_CLOSE_WndProc();
	case WM_DESTROY://���ٴ���
	{
		//������ʱ����
		for (HGDIOBJ & x : tempObject) { 
			if (x != NULL){
				DeleteObject(x);
				x = NULL;
			}
		}
		//���ٳ��ڶ���
		for (HGDIOBJ & x : lTSObject)
		{
			if (x != NULL) {
				DeleteObject(x);
				x = NULL;
			}
		}
		return WM_DESTROY_WndProc();
	}
	default://δ�Զ����������Ϣ
		return DefWindowProc(hwnd, uMsg, wParam, lParam);//����Ĭ�ϴ��ڹ��̺���
	}
}

//��������Ϣ�ĺ���
LRESULT MyWnds::WM_COMMAND_WndProc()
{
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_NOTIFY_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CTLCOLORSTATIC_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_PAINT_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_LBUTTONDOWN_WndProc()
{
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_LBUTTONUP_WndProc()
{
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_WINDOWPOSCHANGING_WndProc() {
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_WINDOWPOSCHANGED_WndProc() {
	//�趨��ʱָ����ո�����Ϣ
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	wndWidth = temp->cx;//���´��ڿ��
	wndHeight = temp->cy;//���´��ڸ߶�
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_SIZE_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CREATE_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CLOSE_WndProc() {
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_DESTROY_WndProc() {
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

BOOL MyWnds::StaticEnumChildProc(HWND hwndChild, LPARAM lParam)
{
	MyWnds* myWnds = (MyWnds*)lParam;
	if (myWnds) return myWnds->EnumChildProc(hwndChild, lParam);
	else return TRUE;
}

BOOL CALLBACK MyWnds::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	return TRUE;
}


WPARAM MyWnds::Wnd(bool needMessageLoop){
	//����ֱ�Ӵ������ڣ���ֹ����Ϊ֮ǰ���ٴ������´����Ӷ������ظ�ע�ᴰ���౨��
	HWND hwnd = CreateWnd();
	if (!hwnd) {
		//���ڴ���ʧ���Ҵ������Ϊ1407(�����಻����)ʱע�ᴰ�����ٴδ�������
		if (GetLastError() == 1407) {
			//ע�ᴰ����
			if (!RegisterWndClass()) ErrorMessageBox(NULL, _T("ע�ᴰ����ʧ��"));
			//��������
			hwnd = CreateWnd();
			if (!hwnd)ErrorMessageBox(NULL, _T("��������ʧ��"));
		}
		else ErrorMessageBox(NULL, _T("��������ʧ��"));
	}
	//������Ϣѭ��
	if (needMessageLoop)return MessageLoop(hwnd);
	else return 0;
}
