#pragma once
#include<Windows.h>
#include<CommCtrl.h>
#include<tchar.h>

//������ʹ��Win XP����ʽ�ؼ����
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//ѹ������׺��
#define SUFFIX ".ya"

//������MyWnds��Ϊ�����������ͳһ����
class MyWnds {
protected:
	//����Ӧ�ó���ĵ�ǰʵ���ľ��
	static HINSTANCE hInstance;
	//��Ļ�����
	static int maxScreenWidth;
	//��Ļ���߶�
	static int maxScreenHeight;
	//��ʱ������Դ
	HGDIOBJ tempObject[10] = {};
	//���ڶ�����Դ
	HGDIOBJ lTSObject[10] = {};
	//���ڿ��
	UINT wndWidth = NULL;
	//���ڸ߶�
	UINT wndHeight = NULL;
	//ģ̬�Ի������
	HWND isModalDialog = NULL;
	//�Ӵ��ڹ��̻�ȡ�Ĳ���
	HWND hwnd_WndProc = NULL;
	UINT uMsg_WndProc = NULL;
	WPARAM wParam_WndProc = NULL;
	LPARAM lParam_WndProc = NULL;
	
	//������Ϣ����
	void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);
	//������Ϣ����
	void TestMessageBox(const HWND& hwnd = NULL, const TCHAR* text = _T("����"), const TCHAR* title = _T("����"),UINT type = MB_OK);
	//ע�ᴰ����
	virtual ATOM RegisterWndClass() = 0;
	//��������
	virtual HWND CreateWnd() = 0;
	//������Ϣѭ��
	virtual WPARAM MessageLoop(const HWND& hwnd_IsDialogMessage);

	//��̬���ڹ���
	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//���ڹ���
	virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//--------------------��������Ϣ�ĺ���---------------------
	virtual LRESULT WM_COMMAND_WndProc();
	virtual LRESULT WM_NOTIFY_WndProc();
	//����static��edit(ES_READONLY)�ؼ�
	virtual LRESULT WM_CTLCOLORSTATIC_WndProc();
	//���ƴ��ڸ�������
	virtual LRESULT WM_PAINT_WndProc();
	//����������
	virtual LRESULT WM_LBUTTONDOWN_WndProc();
	//�ɿ�������
	virtual LRESULT	WM_LBUTTONUP_WndProc();
	//�����ڴ�С/λ�÷����ı���յ�����Ϣ
	virtual LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//�����趨�ô��ڴ�С/λ�ú�ķ�����Ϣ
	virtual LRESULT WM_WINDOWPOSCHANGED_WndProc();
	//��ȡ�����µĿ�Ⱥ͸߶�
	virtual LRESULT WM_SIZE_WndProc();
	//��������ʱ���͵���Ϣ
	virtual LRESULT WM_CREATE_WndProc();
	//����رհ�ť
	virtual LRESULT WM_CLOSE_WndProc();
	//���ٴ���
	virtual LRESULT WM_DESTROY_WndProc();
	//----------------------------------------------------------
	
	//ö�ٱ����Ӵ���ʱ���õĻص�����
	static BOOL CALLBACK StaticEnumChildProc(HWND hwndChild, LPARAM lParam);
	virtual BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	//��ȡӦ�ó���ĵ�ǰʵ���ľ��
	static HINSTANCE GethInstance(){ return hInstance; }
	//�趨Ӧ�ó���ĵ�ǰʵ���ľ��
	static void SethInstance(HINSTANCE hIn) { hInstance = hIn; }
	//��ȡ���ڵľ��
	HWND GetWndHwnd() const { return hwnd_WndProc; }
	UINT GetWndWidth()const { return wndWidth; }
	UINT GetWndHeight()const { return wndHeight; }
	//��������
	virtual WPARAM Wnd(bool needMessageLoop = false);
};
