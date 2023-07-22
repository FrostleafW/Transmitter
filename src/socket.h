#pragma once

#define DEFAULT_PORT 17010

SOCKET CONNECTION = INVALID_SOCKET;
BCRYPT_ALG_HANDLE G_hwnd_alg = NULL;
BCRYPT_KEY_HANDLE G_hwnd_key = NULL;

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
			BYTE public_key[MAX_TEXT] = { 0 };
			int len = RSA_exportPublic(G_hwnd_key, public_key, sizeof(public_key));
			if (send(sock, (char*)public_key, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent RSA key...");

			// Wait for AES key
			BYTE aes_key_enc[MAX_TEXT] = { 0 };
			_s = recv(sock, (char*)aes_key_enc, sizeof(aes_key_enc), 0);
			if (_s <= 0) {
				appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
				return;
			}

			// Decrypt AES key
			BYTE AES_key[16] = { 0 };
			if (RSA_decrypt(G_hwnd_key, aes_key_enc, _s, AES_key, sizeof(AES_key)) > 0) {
				appendTextW(hwnd_msg, L"\r\nReceived AES key: ");
				appendTextByte(hwnd_msg, AES_key, sizeof(AES_key));
			}

			// Destory RSA handle
			BCryptDestroyKey(G_hwnd_key);
			BCryptCloseAlgorithmProvider(G_hwnd_alg, NULL);
			G_hwnd_alg = NULL;
			G_hwnd_key = NULL;

			// Set up AES handle
			if (AES_start(G_hwnd_alg, G_hwnd_key, AES_key, sizeof(AES_key)))
				appendTextW(hwnd_msg, L"\r\nSecure communication established!");
		}
	}
	// Client #2 waits for RSA
	else if (id[0] == 0x02) {
		// Receive RSA public key
		BYTE public_key[MAX_TEXT] = { 0 };
		_s = recv(sock, (char*)public_key, sizeof(public_key), 0);
		if (_s <= 0) {
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			return;
		}
		appendTextW(hwnd_msg, L"\r\nReceived RSA key...");

		// Create AES key
		BYTE AES_key[16];
		AES_generateKey(AES_key);
		appendTextW(hwnd_msg, L"\r\nGenerated AES key: ");
		appendTextByte(hwnd_msg, AES_key, sizeof(AES_key));

		// Encrypt AES key
		if (RSA_importPublic(G_hwnd_alg, G_hwnd_key, public_key, _s)) {
			BYTE aes_key_enc[MAX_TEXT] = { 0 };
			int len = RSA_encrypt(G_hwnd_key, AES_key, sizeof(AES_key), aes_key_enc, sizeof(aes_key_enc));

			// Send AES key
			if (send(sock, (char*)aes_key_enc, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent AES key...");
		}

		// Destory RSA handle
		BCryptDestroyKey(G_hwnd_key);
		BCryptCloseAlgorithmProvider(G_hwnd_alg, NULL);
		G_hwnd_alg = NULL;
		G_hwnd_key = NULL;

		// Set up AES handle
		if (AES_start(G_hwnd_alg, G_hwnd_key, AES_key, sizeof(AES_key)))
			appendTextW(hwnd_msg, L"\r\nSecure communication established!");
	}

	appendTextW(hwnd_msg, L"\r\n----------------------------------------");

	// Main recerve loop
	CONNECTION = sock;
	while (true) {
		BYTE cipher[MAX_TEXT * 2] = { 0 };
		_s = recv(sock, (char*)cipher, MAX_TEXT * 2, 0);
		if (_s > 0)
		{
			wchar_t text[MAX_TEXT] = { 0 };
			AES_decrypt(G_hwnd_key, cipher, _s, (BYTE*)text, sizeof(text));
			appendTextW(hwnd_msg, L"\r\n> ");
			appendTextW(hwnd_msg, text);
		}
		else
		{
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			break;
		}
		Sleep(500);
	}
	closesocket(sock);
	CONNECTION = INVALID_SOCKET;
}

SOCKET Client(HWND hwnd) {
	// Get ip address
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	wchar_t ipW[MAX_TEXT] = { 0 };
	char ip[MAX_TEXT] = { 0 };
	getText(hwnd, ipW);
	if (ipW[0] == 0)
		memcpy(ipW, L"127.0.0.1", 20);
	appendTextW(hwnd_msg, ipW);
	appendTextW(hwnd_msg, L"...");
	wcstombs_s(NULL, ip, ipW, sizeof(ip) - 1);

	// Set up socket
	SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addrServ;
	inet_pton(AF_INET, (PCSTR)ip, &addrServ.sin_addr);
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(DEFAULT_PORT);

	// Send request to server
	if (connect(ClientSocket, (SOCKADDR*)&addrServ, sizeof(SOCKADDR)) == 0)
		return ClientSocket;
	closesocket(ClientSocket);
	return INVALID_SOCKET;
}