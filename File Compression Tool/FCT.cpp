#include"MainWnd.h"

//程序入口函数
int WINAPI _tWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPreInstance, _In_ LPTSTR lpCmdLine, _In_ INT nShowCmd) {
	//接收应用程序的当前实例的句柄
	MyWnds::SethInstance(_hInstance);
	MyWnds& a = MainWnd::GetMainWnd();
	a.Wnd(true);
}