#pragma once
#include "receive.h"

SOCKET Client(HWND hwnd) {
	// Get ip address
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	WCHAR ip[MAX_TEXT_W] = { 0 };
	USHORT port;
	int len;

	appendTextW(hwnd_msg, L"\r\nTry connecting to ");
	len = getIP(hwnd, ip);
	if (len < 7)
		memcpy(ip, L"127.0.0.1", 20);
	port = getPort(hwnd);
	if (port == 0)
		port = DEFAULT_PORT;
	appendTextW(hwnd_msg, ip);
	appendTextW(hwnd_msg, L":");
	appendNumber(hwnd_msg, port);
	appendTextW(hwnd_msg, L"...");

	// Set up socket
	SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addrServ;
	InetPtonW(AF_INET, (PCWSTR)ip, &addrServ.sin_addr);
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);

	// Send request to server
	if (connect(ClientSocket, (SOCKADDR*)&addrServ, sizeof(SOCKADDR)) == 0)
		return ClientSocket;
	closesocket(ClientSocket);
	return INVALID_SOCKET;
}

void Connection(SOCKET sock, HWND hwnd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	// Wait until notified
	appendTextW(hwnd_msg, L"\r\nWaiting to start communication...");
	int _s;
	char id[2] = { 0 };
	_s = recv(sock, id, 2, 0);
	if (_s <= 0) {
		appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
		return;
	}

	// Client #1 sets up RSA
	if (id[0] == 0x01) {
		// Create RSA key
		if (RSA_start(G_hwnd_alg, G_hwnd_key)) {
			// Send RSA public key
			BYTE public_key[MAX_TEXT_W] = { 0 };
			int len = RSA_exportPublic(G_hwnd_key, public_key, sizeof(public_key));
			if (send(sock, (char*)public_key, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent RSA key...");

			// Wait for AES key
			BYTE aes_key_enc[MAX_TEXT_W] = { 0 };
			_s = recv(sock, (char*)aes_key_enc, sizeof(aes_key_enc), 0);
			if (_s <= 0) {
				appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
				return;
			}

			// Decrypt AES key
			BYTE AES_key[16] = { 0 };
			if (RSA_decrypt(G_hwnd_key, aes_key_enc, _s, AES_key, sizeof(AES_key)) > 0) {
				appendTextW(hwnd_msg, L"\r\nReceived AES key...");
			}

			// Destory RSA handle
			Key_cleanup();

			// Set up AES handle
			if (AES_start(G_hwnd_alg, G_hwnd_key, AES_key, sizeof(AES_key)))
				appendTextW(hwnd_msg, L"\r\nSecure communication established!");
		}
	}
	// Client #2 waits for RSA
	else if (id[0] == 0x02) {
		// Receive RSA public key
		BYTE public_key[MAX_TEXT_W] = { 0 };
		_s = recv(sock, (char*)public_key, sizeof(public_key), 0);
		if (_s <= 0) {
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			return;
		}
		appendTextW(hwnd_msg, L"\r\nReceived RSA key...");

		// Create AES key
		BYTE AES_key[16];
		AES_generateKey(AES_key);
		appendTextW(hwnd_msg, L"\r\nGenerated AES key...");

		// Encrypt AES key
		if (RSA_importPublic(G_hwnd_alg, G_hwnd_key, public_key, _s)) {
			BYTE aes_key_enc[MAX_TEXT_W] = { 0 };
			int len = RSA_encrypt(G_hwnd_key, AES_key, sizeof(AES_key), aes_key_enc, sizeof(aes_key_enc));

			// Send AES key
			if (send(sock, (char*)aes_key_enc, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent AES key...");
		}

		// Destory RSA handle
		Key_cleanup();

		// Set up AES handle
		if (AES_start(G_hwnd_alg, G_hwnd_key, AES_key, sizeof(AES_key)))
			appendTextW(hwnd_msg, L"\r\nSecure communication established!");
	}

	appendTextW(hwnd_msg, L"\r\n----------------------------------------");

	// Main recerve loop
	CONNECTION = sock;
	BYTE cipher[MAX_TEXT_W * 2];
	BYTE data[MAX_TEXT_W * 2];
	while (true) {
		memset(cipher, 0, sizeof(cipher));
		_s = recv(sock, (char*)cipher, MAX_TEXT_W * 2, 0);
		if (_s > 0)
		{
			memset(data, 0, sizeof(data));
			AES_decrypt(G_hwnd_key, cipher, _s, data, sizeof(data));
			// Receive command
			if (data[0] == 0)
				Recv_cmd(hwnd, data);
			// Receive normal text
			else
				Recv_text(hwnd_msg, data);
		}
		else
		{
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			break;
		}
		Sleep(500);
	}
	Key_cleanup();
	closesocket(sock);
	CONNECTION = INVALID_SOCKET;

	EnableWindow(GetDlgItem(hwnd, BTN_CLNT_ID), true);
}



void fileTransfer(HWND hwnd) {
	// Select file
	OPENFILENAMEW ofn;
	WCHAR filepath[MAX_PATH] = { 0 };
	FileInfo file_info;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"All Files\0*.*\0\0";
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.lpstrFileTitle = file_info.filename;
	ofn.nMaxFileTitle = sizeof(file_info.filename);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (!GetOpenFileName(&ofn))
		return;

	// Open file and get size
	HANDLE file = CreateFileW(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	BYTE buffer[MAX_TEXT_W * 2];
	LARGE_INTEGER filesize;

	GetFileSizeEx(file, &filesize);
	file_info.fileblocks = (DWORD)(filesize.QuadPart / sizeof(buffer)) + 1;
	
	// Send file info
	{
		BYTE cipher[MAX_TEXT_W * 2] = { 0 };
		int len = AES_encrypt(G_hwnd_key, (BYTE*)&file_info, sizeof(file_info), cipher, sizeof(cipher));
		send(CONNECTION, (char*)&cipher, len, 0);
	}

	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	appendTextW(hwnd_msg, L"\r\n!!!You tried to send a file!!! Waiting for response...");
	appendTextW(hwnd_msg, L"\r\nFilename: ");
	appendTextW(hwnd_msg, file_info.filename);
	appendTextW(hwnd_msg, L"\r\nSize: ");
	appendFilesize(hwnd_msg, file_info.fileblocks);
	EnableWindow(GetDlgItem(hwnd, BTN_SEND_ID), false);

	// Wait for response
	int timeout = 0;
	while (true) {
		Sleep(1000);
		if (CONNECTION == INVALID_SOCKET)
			break;
		if (timeout > 180)
			return;
		timeout++;
	}

	//DWORD len_read = 0;
	//for (DWORD i = 0; i < fileblocks; i++) {
	//	memset(buffer, 0, sizeof(buffer));
	//	if (!ReadFile(file, buffer, sizeof(buffer), &len_read, NULL))
	//		break;
	//	if (len_read == 0)
	//		break;
	//}


	// Close file
	CloseHandle(file);
}