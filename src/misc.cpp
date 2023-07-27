#include <winsock2.h>

#include <cmath>

#include "misc.h"

void appendTextW(HWND& hwnd_msg, const WCHAR* buffer) {
	int index = GetWindowTextLength(hwnd_msg);
	SendMessageW(hwnd_msg, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	SendMessageW(hwnd_msg, EM_REPLACESEL, 0, (LPARAM)buffer);
}

void appendTextA(HWND& hwnd_msg, const char* buffer) {
	WCHAR bufferW[MAX_TEXT_W];
	mbstowcs_s(NULL, bufferW, buffer, sizeof(bufferW));
	appendTextW(hwnd_msg, bufferW);
}

void appendNumber(HWND& hwnd_msg, long long num) {
	WCHAR buffer[MAX_TEXT_W]{};
	int digit = (int)log10((double)num);
	for (int i = digit; i >= 0; i--) {
		buffer[i] = num % 10 + L'0';
		num /= 10;
	}
	appendTextW(hwnd_msg, buffer);
}

void appendFilesize(HWND& hwnd_msg, DWORD filesize) {
	if (filesize < 1024) {
		appendNumber(hwnd_msg, filesize);
		appendTextW(hwnd_msg, L" B");
	}
	else {
		filesize = filesize % 1024 > 0 ? filesize / 1024 + 1 : filesize / 1024;
		if (filesize < 1024) {
			appendNumber(hwnd_msg, filesize);
			appendTextW(hwnd_msg, L" KB");
		}
		else {
			filesize = filesize % 1024 > 0 ? filesize / 1024 + 1 : filesize / 1024;
			appendNumber(hwnd_msg, filesize);
			appendTextW(hwnd_msg, L" MB");
		}
	}
}

void appendTextByte(HWND& hwnd_msg, BYTE* buffer, int len) {
	char str[MAX_TEXT_W]{};
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

int getText(HWND hwnd, WCHAR* text) {
	HWND hwnd_text = GetDlgItem(hwnd, TEXTBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, MAX_TEXT_W);
	SetWindowTextW(hwnd_text, NULL);
	return len;
}

USHORT getPort(HWND& hwnd) {
	USHORT port = 0;
	USHORT temp = 0;
	USHORT digit = 0;
	WCHAR text[8]{};
	HWND hwnd_text = GetDlgItem(hwnd, PORTBOX_ID);
	int len = GetWindowTextW(hwnd_text, text, 8);
	for (int i = len - 1; i >= 0; i--) {
		if (text[i] <= L'9' && text[i] >= L'0') {
			temp = text[i] - L'0';
			port += temp * (USHORT)pow(10, digit);
			digit++;
		}
	}
	return port;
}
