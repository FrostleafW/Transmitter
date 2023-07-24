#pragma once
#include <thread>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")
#include "misc.h"
#include "transfer.h"

WNDPROC TextboxProc;
LRESULT CALLBACK callbackTextbox(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void createWidgets(HWND hwnd) {
	// Render widgets
	HWND MSGBOX = CreateWindowW(TEXT("EDIT"), TEXT("Welcome to Transmitter!\r\nInput IP address to connect..."), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 5, 5, Win_WIDTH - 150, Win_HEIGHT - 150, hwnd, (HMENU)MSGBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_CLNT = CreateWindowW(TEXT("BUTTON"), TEXT("Connect"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, 15, 100, 40, hwnd, (HMENU)BTN_CLNT_ID, GetModuleHandle(NULL), NULL);
	HWND TEXTBOX = CreateWindowW(TEXT("EDIT"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, 5, Win_HEIGHT - 135, Win_WIDTH - 150, 90, hwnd, (HMENU)TEXTBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_SEND = CreateWindowW(TEXT("BUTTON"), TEXT("Send"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, Win_HEIGHT - 110, 100, 40, hwnd, (HMENU)BTN_SEND_ID, GetModuleHandle(NULL), NULL);
	HWND IPBOX = CreateWindowW(TEXT("EDIT"), TEXT("IP"), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL, Win_WIDTH - 140, 60, 120, 24, hwnd, (HMENU)IPBOX_ID, GetModuleHandle(NULL), NULL);
	HWND PORTBOX = CreateWindowW(TEXT("EDIT"), TEXT("PORT"), WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER, Win_WIDTH - 140, 90, 120, 24, hwnd, (HMENU)PORTBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_FILE = CreateWindowW(TEXT("BUTTON"), TEXT("Send File"), WS_CHILD | WS_VISIBLE | ES_CENTER, Win_WIDTH - 130, Win_HEIGHT - 190, 100, 40, hwnd, (HMENU)BTN_FILE_ID, GetModuleHandle(NULL), NULL);


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

	SendMessageW(MSGBOX, EM_SETREADONLY, TRUE, NULL);
	SendMessageW(TEXTBOX, EM_SETLIMITTEXT, (WPARAM)MAX_TEXT_W, NULL);
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
		Btn_connect(hwnd);
		break;
	}
	case BTN_SEND_ID:
	{
		// Send Btn
		if (CONNECTION != INVALID_SOCKET) {
			Btn_sendText(hwnd);
		}
		break;
	}
	case BTN_FILE_ID:
	{
		// Send File Btn
		if (CONNECTION != INVALID_SOCKET)
			Btn_sendFile(hwnd);
		break;
	}
	}
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
