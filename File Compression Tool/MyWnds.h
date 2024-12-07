#pragma once
#include<Windows.h>
#include<tchar.h>

//������ʹ��Win XP����ʽ�ؼ����
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//ѹ������׺��
#define SUFFIX .ya

//������MyWnds��Ϊ�����������ͳһ����
class MyWnds {
protected:
	//����Ӧ�ó���ĵ�ǰʵ���ľ��
	static HINSTANCE hInstance;
	//��Ļ�����
	static int maxScreenWidth;
	//��Ļ���߶�
	static int maxScreenHeight;
	//Ĭ�ϻ�ˢ
	HBRUSH defBrush = CreateSolidBrush(RGB(66, 149, 224));
	//���ڿ��
	int wndWidth = 0.53 * maxScreenWidth;
	//���ڸ߶�
	int wndHeight = 0.62 * maxScreenHeight;

	//�Ӵ��ڹ��̻�ȡ�Ĳ���
	HWND hwnd_WndProc;
	UINT uMsg_WndProc;
	WPARAM wParam_WndProc;
	LPARAM lParam_WndProc;

	~MyWnds(){
		DeleteObject(defBrush);//������ʱ��ˢ
	}
	//������Ϣ����
	void ErrorMessageBox(const HWND& hwnd = NULL, const TCHAR* msg = _T(""), bool showErrorCode = true);

	//ע�ᴰ����
	virtual ATOM RegisterWndClass() = 0;
	//��������
	virtual HWND CreateWnd() = 0;
	//������Ϣѭ��
	virtual WPARAM MessageLoop(const HWND& hwnd_IsDialogMessage, const HWND& hwnd_GetMessage =NULL);

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
	//�����ڴ�С/λ�÷����ı���յ�����Ϣ�������������˴��ڵ���С�ߴ�
	virtual LRESULT WM_WINDOWPOSCHANGING_WndProc();
	//�����趨�ô��ڴ�С/λ�ú�ķ�����Ϣ������������ͬ���������ڸô��ڵ������Ӵ���/�ؼ���λ�û��С��ʵ������Ӧ�仯
	virtual LRESULT WM_WINDOWPOSCHANGED_WndProc();
	//��������ʱ���͵���Ϣ
	virtual LRESULT WM_CREATE_WndProc();
	//����رհ�ť
	virtual LRESULT WM_CLOSE_WndProc();
	//���ٴ���
	virtual LRESULT WM_DESTROY_WndProc();
	//----------------------------------------------------------
	
	//ö���Ӵ���ʱ���õĻص�����
	static BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

public:
	//��ȡӦ�ó���ĵ�ǰʵ���ľ��
	static HINSTANCE GethInstance() { return hInstance; }
	//�趨Ӧ�ó���ĵ�ǰʵ���ľ��
	static void SethInstance(HINSTANCE hIn) { hInstance = hIn; }
	//��������
	virtual WPARAM Wnd(bool needMessageLoop = false);
};


//��������
class MainWnd :public MyWnds {
	//���õ�����������������ֻ��ʵ����һ������
	MainWnd(){} //��ֹ�ⲿ����
	~MainWnd() {} //��ֹ�ⲿ����
	MainWnd(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��������
	const MainWnd& operator=(const MainWnd& mainWnd) = delete;//��ֹ�ⲿ��ֵ����

	//----------------------������д�ĺ���--------------------------------
	//ע�ᴰ����
	ATOM RegisterWndClass();
	//��������
	HWND CreateWnd();

	LRESULT WM_CREATE_WndProc();

public:
	static MainWnd& GetMainWnd() { 
		static MainWnd mainWnd;
		return mainWnd; 
	}
};
