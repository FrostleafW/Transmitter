#pragma once
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "misc.h"
#include "socket.h"

void createWidgets(HWND hwnd);
void callbackWidgets(HWND hwnd, int wmID);

void createWidgets(HWND hwnd) {
	// Render widgets
	HWND MSGBOX = CreateWindowW(TEXT("EDIT"), TEXT("Welcome to Transmitter!\n"), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 5, 5, 450, 500, hwnd, (HMENU)MSGBOX_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_SERV = CreateWindowW(TEXT("BUTTON"), TEXT("Hi"), WS_CHILD | WS_VISIBLE | ES_CENTER, 500, 100, 100, 40, hwnd, (HMENU)BTN_SERV_ID, GetModuleHandle(NULL), NULL);
	HWND BTN_CLNT = CreateWindowW(TEXT("BUTTON"), TEXT("Ho"), WS_CHILD | WS_VISIBLE | ES_CENTER, 500, 200, 100, 40, hwnd, (HMENU)BTN_CLNT_ID, GetModuleHandle(NULL), NULL);


	// Set fonts
	HFONT TextFont = CreateFontW(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SendMessageW(MSGBOX, WM_SETFONT, (WPARAM)TextFont, TRUE);
	SendMessageW(MSGBOX, EM_SETREADONLY, TRUE, NULL);

}

void callbackWidgets(HWND hwnd, int wmID) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	switch (wmID)
	{
	case BTN_SERV_ID:
	{
		enable_ConnectionBtn(hwnd, false);
		appendText(hwnd_msg, "Start listening...");
		std::thread serv(Server, hwnd_msg);
		serv.detach();
		break;
	}
	case BTN_CLNT_ID:
	{
		enable_ConnectionBtn(hwnd, false);
		appendText(hwnd_msg, "Try connecting...");
		SOCKET ServerSocket = Client();
		if (ServerSocket == INVALID_SOCKET) {
			appendText(hwnd_msg, "Connection failed!\n");
			enable_ConnectionBtn(hwnd, true);
			return;
		}
		appendText(hwnd_msg, "Succeeded!\n");
		enable_ConnectionBtn(hwnd, true);
		Sleep(1000);
		char buffer[100];
		while (true) {
			recv(ServerSocket, buffer, 100, 0);
			appendText(hwnd_msg, buffer);
			Sleep(1000);
		}

		break;
	}
	}
}