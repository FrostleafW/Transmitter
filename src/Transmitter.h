#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "socket.h"

#define TEXTBOX_ID 10
#define BTN_ID 11

void createWidgets(HWND hwnd);
void callbackWidgets(HWND hwnd, int wmID);
void appendText(HWND hwnd);

void createWidgets(HWND hwnd) {
	// Render widgets
	HWND Textbox = CreateWindowW(TEXT("EDIT"), TEXT("Welcome to Transmitter!\n"), WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 5, 5, 450, 500, hwnd, (HMENU)TEXTBOX_ID, GetModuleHandle(NULL), NULL);
	HWND Btn = CreateWindowW(TEXT("BUTTON"), TEXT("Hi"), WS_CHILD | WS_VISIBLE | ES_CENTER, 500, 100, 100, 40, hwnd, (HMENU)BTN_ID, GetModuleHandle(NULL), NULL);

	// Set fonts
	HFONT TextFont = CreateFontW(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	SendMessageW(Textbox, WM_SETFONT, WPARAM(TextFont), TRUE);

}

void callbackWidgets(HWND hwnd, int wmID) {
	switch (wmID)
	{
	case BTN_ID:
		appendText(hwnd);
		break;
	}
}

void appendText(HWND hwnd) {
	char buffer[] = "aaa\n";
	HWND hwnda = GetDlgItem(hwnd, TEXTBOX_ID);
	int index = GetWindowTextLength(hwnda);
	SendMessageA(hwnda, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	SendMessageA(hwnda, EM_REPLACESEL, 0, (LPARAM)buffer);
}