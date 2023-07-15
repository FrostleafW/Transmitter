#pragma once

#define MSGBOX_ID 10
#define BTN_SERV_ID 11
#define BTN_CLNT_ID 12

void appendText(HWND hwnd_msg, const char* buffer) {
	int index = GetWindowTextLength(hwnd_msg);
	SendMessageA(hwnd_msg, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	SendMessageA(hwnd_msg, EM_REPLACESEL, 0, (LPARAM)buffer);
}

void enable_ConnectionBtn(HWND hwnd, bool _switch) {
	EnableWindow(GetDlgItem(hwnd, BTN_SERV_ID), _switch);
	EnableWindow(GetDlgItem(hwnd, BTN_CLNT_ID), _switch);
}