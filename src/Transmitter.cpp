#include "Transmitter.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	//AllocConsole();
	//freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	// Register window class
	const WCHAR Class_Name[] = L"Transmitter";
	const WCHAR App_Name[] = L"Transmitter";

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Class_Name;

	if (!RegisterClass(&wc)) {
		MessageBox(NULL, L"This program fails to run on this computer.", Class_Name, MB_ICONERROR);
		return 0;
	}

	// Create window
	HWND hwnd = CreateWindowEx(0, Class_Name, App_Name, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, (GetSystemMetrics(0) - Win_WIDTH) / 2, (GetSystemMetrics(1) - Win_HEIGHT) / 2, Win_WIDTH, Win_HEIGHT, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nShowCmd);

	// Run message loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;

	switch (message) {
	case WM_CREATE:
		createWidgets(hwnd);
		break;

	case WM_COMMAND:
		callbackWidgets(hwnd, LOWORD(wParam));
		break;

	case WM_PAINT:
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		closesocket(CONNECTION);
		WSACleanup();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

