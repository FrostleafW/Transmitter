#pragma once

#define Win_HEIGHT 600
#define Win_WIDTH 800

#define MSGBOX_ID 10
#define BTN_CLNT_ID 12
#define TEXTBOX_ID 13
#define BTN_SEND_ID 14
#define IPBOX_ID 15
#define PORTBOX_ID 16

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
	char str[MAX_TEXT] = { 0 };
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

void appendNumber(HWND hwnd_msg, int num) {
	wchar_t buffer[MAX_TEXT] = { 0 };
	int digit = log10(num);
	for (int i = digit; i >= 0; i--) {
		buffer[i] = num % 10 + L'0';
		num /= 10;
	}
	appendTextW(hwnd_msg, buffer);
}

int getText(HWND hwnd, wchar_t* text) {
	HWND hwnd_text = GetDlgItem(hwnd, TEXTBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, MAX_TEXT);
	SetWindowTextW(hwnd_text, NULL);
	return len;
}

int getIP(HWND hwnd, wchar_t* text) {
	HWND hwnd_text = GetDlgItem(hwnd, IPBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, MAX_TEXT);
	return len;
}

u_long getPort(HWND hwnd) {
	u_long port = 0;
	u_long temp = 0;
	u_long digit = 0;
	wchar_t text[MAX_TEXT] = { 0 };
	HWND hwnd_text = GetDlgItem(hwnd, PORTBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, MAX_TEXT);
	for (int i = len - 1; i >= 0; i--) {
		if (text[i] <= L'9') {
			temp = text[i] - L'0';
			port += temp * pow(10, digit);
			digit++;
		}
	}
	return port;
}