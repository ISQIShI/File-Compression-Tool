#include"MainWnd.h"

//������ں���
int WINAPI _tWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPreInstance, _In_ LPTSTR lpCmdLine, _In_ INT nShowCmd) {
	//����Ӧ�ó���ĵ�ǰʵ���ľ��
	MyWnds::SethInstance(_hInstance);
	MyWnds& a = MainWnd::GetMainWnd();
	a.Wnd(true);
}