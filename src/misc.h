#pragma once

#define Win_HEIGHT 600
#define Win_WIDTH 800

#define MSGBOX_ID 10
#define BTN_CLNT_ID 12
#define TEXTBOX_ID 13
#define BTN_SEND_ID 14

#define MAX_TEXT 512

void appendTextW(HWND hwnd_msg, const wchar_t* buffer) {
	int index = GetWindowTextLength(hwnd_msg);
	SendMessageW(hwnd_msg, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	SendMessageW(hwnd_msg, EM_REPLACESEL, 0, (LPARAM)buffer);
}

void appendTextA(HWND hwnd_msg, const char* buffer) {
	wchar_t bufferW[MAX_TEXT];
	mbstowcs_s(NULL, bufferW, buffer, sizeof(bufferW));
	appendTextW(hwnd_msg, bufferW);
}

void appendTextByte(HWND hwnd_msg, BYTE* buffer, int len) {
	char str[MAX_TEXT];
	BYTE* pin = buffer;
	const char* hex = "0123456789ABCDEF";
	char* pout = str;
	for (int i = 0; i < len - 1; ++i) {
		*pout++ = hex[(*pin >> 4) & 0xF];
		*pout++ = hex[(*pin++) & 0xF];
		*pout++ = ':';
	}
	*pout++ = hex[(*pin >> 4) & 0xF];
	*pout++ = hex[(*pin) & 0xF];
	*pout = 0;
	appendTextA(hwnd_msg, str);
}

int getText(HWND hwnd, wchar_t* text) {
	HWND hwnd_text = GetDlgItem(hwnd, TEXTBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, MAX_TEXT);
	SetWindowTextW(hwnd_text, NULL);
	return len;
}