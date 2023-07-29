#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <thread>

#include "misc.h"
#include "Network.h"
#include "Transmitter.h"

Network conn;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	//AllocConsole();
	//freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	// Register window class
	const WCHAR Class_Name[] = L"Transmitter";
	const WCHAR App_Name[] = L"Transmitter";

	WNDCLASS wc{};
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

	int Win_WIDTH = 800;
	int Win_HEIGHT = 600;

	// Create window
	HWND hwnd = CreateWindowExW(0, Class_Name, App_Name, WS_OVERLAPPEDWINDOW, (GetSystemMetrics(0) - Win_WIDTH) / 2, (GetSystemMetrics(1) - Win_HEIGHT) / 2, Win_WIDTH, Win_HEIGHT, NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nShowCmd);

	// Run message loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;

	switch (message) {
	case WM_CREATE:
	{
		createWidgets(hwnd);
		conn.passHandle(hwnd);
		break;
	}
	case WM_COMMAND:
	{
		callbackWidgets(hwnd, LOWORD(wParam));
		break;
	}
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
			return 0;
		callbackSize(hwnd);
		break;
	}
	case WM_PAINT:
	{
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
	{
		conn.disconnect();
		WSACleanup();
		PostQuitMessage(0);
		break;
	}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void createWidgets(HWND hwnd) {
	int Win_WIDTH = 800;
	int Win_HEIGHT = 600;

	// Render widgets
	HWND MSGBOX = CreateWindowW(TEXT("EDIT"), TEXT("Welcome to Transmitter!\r\nInput IP address to connect..."), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 5, 5, Win_WIDTH - 150, Win_HEIGHT - 150, hwnd, (HMENU)MSGBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_CLNT = CreateWindowW(TEXT("BUTTON"), TEXT("Connect"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, 15, 100, 40, hwnd, (HMENU)BTN_CLNT_ID, GetModuleHandle(NULL), NULL);
	HWND TEXTBOX = CreateWindowW(TEXT("EDIT"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, 5, Win_HEIGHT - 135, Win_WIDTH - 150, 90, hwnd, (HMENU)TEXTBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_SEND = CreateWindowW(TEXT("BUTTON"), TEXT("Send"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, Win_HEIGHT - 110, 100, 40, hwnd, (HMENU)BTN_SEND_ID, GetModuleHandle(NULL), NULL);
	HWND IPBOX = CreateWindowW(TEXT("EDIT"), TEXT("IP"), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, Win_WIDTH - 140, 60, 120, 24, hwnd, (HMENU)IPBOX_ID, GetModuleHandle(NULL), NULL);
	HWND PORTBOX = CreateWindowW(TEXT("EDIT"), TEXT("PORT"), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, Win_WIDTH - 140, 90, 120, 24, hwnd, (HMENU)PORTBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_FILE = CreateWindowW(TEXT("BUTTON"), TEXT("Send File"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, Win_HEIGHT - 190, 100, 40, hwnd, (HMENU)BTN_FILE_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_CALL = CreateWindowW(TEXT("BUTTON"), TEXT("Call"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, Win_HEIGHT - 250, 100, 40, hwnd, (HMENU)BTN_CALL_ID, GetModuleHandle(NULL), NULL);

	// Set fonts
	HFONT TextFont = CreateFontW(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	HFONT TextFontSmall = CreateFontW(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SendMessageW(MSGBOX, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(TEXTBOX, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(BTN_CLNT, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(BTN_SEND, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(IPBOX, WM_SETFONT, (WPARAM)TextFontSmall, TRUE);
	SendMessageW(PORTBOX, WM_SETFONT, (WPARAM)TextFontSmall, TRUE);
	SendMessageW(BTN_FILE, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(BTN_CALL, WM_SETFONT, (WPARAM)TextFont, TRUE);

	// Set properties
	SendMessageW(MSGBOX, EM_SETREADONLY, TRUE, NULL);
	SendMessageW(TEXTBOX, EM_SETLIMITTEXT, (WPARAM)MAX_TEXT_W - 1, NULL);
	SendMessageW(IPBOX, EM_SETLIMITTEXT, (WPARAM)32, NULL);
	SendMessageW(PORTBOX, EM_SETLIMITTEXT, (WPARAM)8, NULL);

	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WSACleanup();
		EnableWindow(GetDlgItem(hwnd, BTN_CLNT_ID), false);
	}

	// Set Textbox callback
	TextboxProc = (WNDPROC)SetWindowLongPtr(TEXTBOX, GWLP_WNDPROC, (LONG_PTR)callbackTextbox);
}

void callbackWidgets(HWND hwnd, int id) {
	switch (id)
	{
	case BTN_CLNT_ID:
	{
		// Connect Btn
		WCHAR ip[32]{};
		int len = GetWindowTextW(GetDlgItem(hwnd, IPBOX_ID), ip, 32);
		if (len < 7)
			memcpy(ip, L"127.0.0.1", 20);

		USHORT port = getPort(hwnd);
		if (port == 0)
			port = DEFAULT_PORT;

		conn.connect_to(ip, port);
		break;
	}
	case BTN_SEND_ID:
	{
		// Send Btn
		if (conn.is_connected()) {
			WCHAR text[MAX_TEXT_W]{};
			int len = getText(hwnd, text);
			if (len > 0)
				conn.send_text(text, len);
		}
		break;
	}
	case BTN_FILE_ID:
	{
		// Send File Btn
		if (conn.is_connected()) {
			std::thread file_thread([&] {conn.send_file(); });
			file_thread.detach();
		}
		break;
	}
	case BTN_CALL_ID:
	{
		// Call Btn
		if (conn.is_connected()) {
			std::thread audio_thread([&] {conn.send_audio(); });
			audio_thread.detach();
		}
		else
			conn.hangup_audio();
		break;
	}
	}
}

void callbackSize(HWND hwnd) {
	int Win_WIDTH = 800;
	int Win_HEIGHT = 600;
	// Get window size
	RECT rect;
	if (GetWindowRect(hwnd, &rect))
	{
		Win_WIDTH = rect.right - rect.left;
		Win_HEIGHT = rect.bottom - rect.top;
	}

	SetWindowPos(GetDlgItem(hwnd, MSGBOX_ID), NULL, NULL, NULL, Win_WIDTH - 150, Win_HEIGHT - 150, SWP_NOMOVE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, BTN_CLNT_ID), NULL, Win_WIDTH - 130, 15, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, TEXTBOX_ID), NULL, 5, Win_HEIGHT - 135, Win_WIDTH - 150, 90, SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, BTN_SEND_ID), NULL, Win_WIDTH - 130, Win_HEIGHT - 110, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, IPBOX_ID), NULL, Win_WIDTH - 140, 60, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, PORTBOX_ID), NULL, Win_WIDTH - 140, 90, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, BTN_FILE_ID), NULL, Win_WIDTH - 130, Win_HEIGHT - 190, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
	SetWindowPos(GetDlgItem(hwnd, BTN_CALL_ID), NULL, Win_WIDTH - 130, Win_HEIGHT - 250, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);
}

LRESULT CALLBACK callbackTextbox(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CHAR:
	{
		switch (wParam)
		{
		case VK_RETURN:
		{
			// Hijack return key
			HWND mainWin = GetAncestor(hwnd, GA_PARENT);
			SendMessageW(mainWin, WM_COMMAND, (DWORD)BTN_SEND_ID, NULL);
			return NULL;
		}
		}
		break;
	}
	}
	return CallWindowProc((WNDPROC)TextboxProc, hwnd, message, wParam, lParam);
}