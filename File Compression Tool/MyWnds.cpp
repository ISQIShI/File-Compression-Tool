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

WPARAM MyWnds::MessageLoop(const HWND& hwnd_IsDialogMessage, const HWND& hwnd_GetMessage){
	MSG Msg = { 0 };
	BOOL bRet = 1;
	while ((bRet = GetMessage(&Msg, hwnd_GetMessage, 0, 0)) != 0) {
		if (bRet == -1) ErrorMessageBox(NULL, _T("��Ϣ����ʧ��"));
		else
		{
			if (!IsDialogMessage(hwnd_IsDialogMessage, &Msg)) //�ú������������Ϣ(��TAB�л��ؼ�����)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
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
	case WM_WINDOWPOSCHANGING://���ڸ��Ĵ��ڴ�С
		return WM_WINDOWPOSCHANGING_WndProc();
	case WM_WINDOWPOSCHANGED://���ڴ�С��ɸ���
		return WM_WINDOWPOSCHANGED_WndProc();
	case WM_CREATE:
		return WM_CREATE_WndProc();
	case WM_CLOSE://�رմ���
		return WM_CLOSE_WndProc();
	case WM_DESTROY://���ٴ���
		return WM_DESTROY_WndProc();
	default://δ�Զ����������Ϣ
		return DefWindowProc(hwnd, uMsg, wParam, lParam);//����Ĭ�ϴ��ڹ��̺���
	}
}

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

//��������Ϣ�ĺ���
LRESULT MyWnds::WM_PAINT_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_WINDOWPOSCHANGING_WndProc() {
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

LRESULT MyWnds::WM_WINDOWPOSCHANGED_WndProc() {
	//�趨��ʱָ����ո�����Ϣ
	WINDOWPOS* temp = (WINDOWPOS*)lParam_WndProc;
	//����lParam������ö���Ӵ���ʱ���ص���������
	RECT rect = { wndWidth,wndHeight,temp->cx ,temp->cy };//ʹ��RECT���ͱ����洢���ڱ仯ǰ��Ŀ�͸�
	LPARAM lParam = (LPARAM)&rect;//ʹlParam�洢rect��ֵַ���Ӷ�֮��ǿתΪRECTָ���ȡ����
	EnumChildWindows(hwnd_WndProc, EnumChildProc, lParam);
	wndWidth = temp->cx;//���´��ڿ��
	wndHeight = temp->cy;//���´��ڸ߶�
	return 0;
}

LRESULT MyWnds::WM_CREATE_WndProc(){
	return DefWindowProc(hwnd_WndProc, uMsg_WndProc, wParam_WndProc, lParam_WndProc);
}

LRESULT MyWnds::WM_CLOSE_WndProc() {
	DestroyWindow(hwnd_WndProc);//���ٴ��ڲ�����WM_DESTROY��Ϣ
	return 0;
}

LRESULT MyWnds::WM_DESTROY_WndProc() {
	PostQuitMessage(0);//����WM_QUIT��Ϣ
	return 0;
}

BOOL CALLBACK MyWnds::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	//�趨��ʱָ����ո�����Ϣ
	RECT* temp = (RECT*)lParam;
	//��ȡ�Ӵ�������Ļ�ϵ�����
	RECT rect;
	GetWindowRect(hwndChild, &rect);
	//���Ӵ�������ת��Ϊ����ڸ����ڵ�����
	POINT point = { rect.left,rect.top };
	ScreenToClient(GetParent(hwndChild), &point);
	//�����Ӵ����ڸ����������λ��
	MoveWindow(hwndChild, double(point.x * temp->right) / temp->left , double(point.y * temp->bottom) / temp->top , rect.right - rect.left, rect.bottom - rect.top ,TRUE);
	return TRUE;
}

WPARAM MyWnds::Wnd(bool needMessageLoop){
	//����ֱ�Ӵ������ڣ���ֹ����Ϊ֮ǰ���ٴ������´����Ӷ������ظ�ע�ᴰ���౨��
	HWND hwnd = CreateWnd();
	if (hwnd) return 0;//���ڴ����ɹ�ֱ�ӷ���
	//���ڴ���ʧ���Ҵ������Ϊ1407(�����಻����)ʱע�ᴰ�����ٴδ�������
	if (GetLastError()==1407) {
		//ע�ᴰ����
		if (!RegisterWndClass()) ErrorMessageBox(NULL, _T("ע�ᴰ����ʧ��"));
		//��������
		hwnd = CreateWnd();
		if (!hwnd)ErrorMessageBox(NULL, _T("��������ʧ��"));
		//������Ϣѭ��
		if (needMessageLoop)return MessageLoop(hwnd);
		else return 0;
	}
	else ErrorMessageBox(NULL, _T("��������ʧ��"));
}

//------------------------------------MainWnd----------------------------------------------
ATOM MainWnd::RegisterWndClass()
{
    //ʵ�������������---������
    WNDCLASSEX mainWndClass = { 0 };
    mainWndClass.cbSize = sizeof(WNDCLASSEX);
    mainWndClass.style = CS_HREDRAW | CS_VREDRAW;//����ʽ
    mainWndClass.lpfnWndProc = StaticWndProc;//���ڹ���
    mainWndClass.hInstance = hInstance;//����ʵ��
    mainWndClass.hbrBackground = defBrush;//�౳����ˢ
    mainWndClass.lpszClassName = _T("mainWndClassName");//��������
    mainWndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);//����ͼ��
    return RegisterClassEx(&mainWndClass);
}


HWND MainWnd::CreateWnd()
{
	//��������---������
	HWND mainHwnd = CreateWindowEx(
		WS_EX_CONTROLPARENT, _T("mainWndClassName"), _T("Ѽһѹ"), WS_TILEDWINDOW,
		int(0.15 * wndWidth), int(0.15 * wndHeight), wndWidth, wndHeight,
		NULL, NULL, hInstance, this
	);
	//��ʾ����
	if(mainHwnd) ShowWindow(mainHwnd, SW_SHOW);
	return mainHwnd;
}

LRESULT MainWnd::WM_CREATE_WndProc()
{
	return LRESULT();
}

