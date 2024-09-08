#include"pch.h"
#include"WINDOW.h"
#include<memory>
#include<crtdbg.h>
#pragma comment(lib, "winmm.lib" )
#pragma comment(lib, "imm32.lib" )

WINDOW& WINDOW::instance()
{
	static std::unique_ptr<WINDOW> instance(new WINDOW);
    return *instance;
}

void WINDOW::create(LPCWSTR windowTitle, int clientWidth, int clientHeight, bool fullscreen, int clientPosX, int clientPosY)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	timeBeginPeriod(1);

	HINSTANCE hInstance = GetModuleHandle(NULL);

	//ウィンドウクラス登録
	WNDCLASSEX windowClass = {};
	windowClass.lpszClassName = L"GAME_WINDOW";
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_BYTEALIGNCLIENT;
	windowClass.lpfnWndProc = WINDOW::WndProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hIcon = LoadIcon(hInstance, L"MYICON");   //アイコンハンドルを指定
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClassEx(&windowClass);

	//引数から表示位置、ウィンドウの大きさ調整
	WindowTitle = windowTitle;
	ClientWidth = clientWidth;
	ClientHeight = clientHeight;
	Aspect = static_cast<float>(ClientWidth) / ClientHeight;
	RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	if (fullscreen) {
		windowStyle = WS_POPUP;//Alt + F4で閉じる
	}
	AdjustWindowRect(&windowRect, windowStyle, FALSE);
	if (clientPosX == -1) {
		clientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;//横中央表示
	}
	if (clientPosY == -1) {
		clientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;//中央表示
	}
	int windowPosX = clientPosX + windowRect.left;
	int windowPosY = clientPosY + windowRect.top;
	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	//ウィンドウをつくる
	HWnd = CreateWindowEx(
		NULL,
		L"GAME_WINDOW",
		WindowTitle,
		windowStyle,
		windowPosX,
		windowPosY,
		windowWidth,
		windowHeight,
		NULL,		//親ウィンドウなし
		NULL,		//メニューなし
		hInstance,
		NULL);		//複数ウィンドウなし

	ShowWindow(HWnd, SW_SHOW);
}

void WINDOW::destroy()
{
	timeEndPeriod(1);
}

bool WINDOW::quit()
{
	while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
		if (Msg.message == WM_QUIT) {
			return true;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return false;
}

LRESULT WINDOW::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
}
