#pragma once
#include "encryption.h"
#include "socket.h"

void Btn_connect(HWND hwnd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	EnableWindow(GetDlgItem(hwnd, BTN_CLNT_ID), false);
	SOCKET ServerSocket = Client(hwnd);
	if (ServerSocket == INVALID_SOCKET) {
		appendTextW(hwnd_msg, L"Connection failed!");
		EnableWindow(GetDlgItem(hwnd, BTN_CLNT_ID), true);
		return;
	}
	appendTextW(hwnd_msg, L"Succeeded!");
	std::thread conn(Connection, ServerSocket, hwnd);
	conn.detach();
}

void Btn_sendText(HWND hwnd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	WCHAR text[MAX_TEXT_W] = { 0 };
	BYTE cipher[MAX_TEXT_W * 2] = { 0 };
	int text_len = getText(hwnd, text);
	if (text_len == 0)
		return;
	int len = AES_encrypt(G_hwnd_key, (BYTE*)text, text_len * 2, cipher, sizeof(cipher));
	send(CONNECTION, (char*)cipher, len, 0);
	appendTextW(hwnd_msg, L"\r\n< ");
	appendTextW(hwnd_msg, text);
}

void Btn_sendFile(HWND hwnd) {
	std::thread file_tran(fileTransfer, hwnd);
	file_tran.detach();
}