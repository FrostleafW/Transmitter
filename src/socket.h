#pragma once

SOCKET Client(HWND hwnd) {
	// Get ip address
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	WCHAR ip[32]{};
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
	sockaddr_in addrServ{};
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
	CONNECTION = sock;
	BYTE cipher[MAX_TEXT_W * 2];
	BYTE data[MAX_TEXT_W * 2];
	int _s;
	while (true) {
		memset(cipher, 0, sizeof(cipher));
		_s = recv(sock, (char*)cipher, sizeof(cipher), 0);
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
			SEND_MODE = 1;
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

void startConnect(SOCKET sock, HWND hwnd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	// Wait until notified
	appendTextW(hwnd_msg, L"\r\nWaiting to start communication...");
	int _s;
	char id[2]{};
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
			BYTE public_key[MAX_TEXT_W]{};
			int len = RSA_exportPublic(G_hwnd_key, public_key, sizeof(public_key));
			if (send(sock, (char*)public_key, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent RSA key...");

			// Wait for AES key
			BYTE aes_key_enc[MAX_TEXT_W]{};
			_s = recv(sock, (char*)aes_key_enc, sizeof(aes_key_enc), 0);
			if (_s <= 0) {
				appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
				return;
			}

			// Decrypt AES key
			BYTE AES_key[16]{};
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
		BYTE public_key[MAX_TEXT_W]{};
		_s = recv(sock, (char*)public_key, sizeof(public_key), 0);
		if (_s <= 0) {
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			return;
		}
		appendTextW(hwnd_msg, L"\r\nReceived RSA key...");

		// Create AES key
		BYTE AES_key[16]{};
		AES_generateKey(AES_key);
		appendTextW(hwnd_msg, L"\r\nGenerated AES key...");

		// Encrypt AES key
		if (RSA_importPublic(G_hwnd_alg, G_hwnd_key, public_key, _s)) {
			BYTE aes_key_enc[MAX_TEXT_W]{};
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
	SEND_MODE = 1;
	std::thread conn(Connection, sock, hwnd);
	conn.detach();
}

void fileTransfer(HWND hwnd) {
	// Select file
	OPENFILENAMEW ofn;
	WCHAR filepath[MAX_PATH]{};
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
	LARGE_INTEGER filesize;

	GetFileSizeEx(file, &filesize);
	file_info.filesize = (DWORD)filesize.QuadPart;

	// Send file info
	SOCKET sock = CONNECTION;
	{
		BYTE cipher[sizeof(FileInfo) + AES_PADDING]{};
		int len = AES_encrypt(G_hwnd_key, (BYTE*)&file_info, sizeof(FileInfo), cipher, sizeof(cipher));
		send(sock, (char*)cipher, len, 0);
	}

	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	appendTextW(hwnd_msg, L"\r\n!!!You tried to send a file!!! Waiting for response...");
	appendTextW(hwnd_msg, L"\r\n\tFilename: ");
	appendTextW(hwnd_msg, file_info.filename);
	appendTextW(hwnd_msg, L"\r\n\tSize: ");
	appendFilesize(hwnd_msg, file_info.filesize);

	// Wait for response for 3 min
	int timeout = 0;
	while (true) {
		Sleep(1000);
		if (CONNECTION == INVALID_SOCKET)
			break;
		if (timeout > 180 || SEND_MODE == 1) {
			SEND_MODE = 1;
			CloseHandle(file);
			return;
		}
		timeout++;
	}
	SEND_MODE = 1;
	appendTextW(hwnd_msg, L"\r\nStart transferring...(# -> 1MB)\r\n");

	// Start sending file
	BYTE data[MAX_TEXT_W * 2 - 1];
	BYTE cipher[MAX_TEXT_W * 2];
	DWORD byteread = 0;
	int count = 0;
	do {
		memset(data, 0, sizeof(data));
		memset(cipher, 0, sizeof(cipher));

		if (!(ReadFile(file, data, sizeof(data), &byteread, NULL) && AES_encrypt(G_hwnd_key, data, sizeof(data), cipher, sizeof(cipher)) > 0 && send(sock, (char*)cipher, sizeof(cipher), 0) > 0)) {
			appendTextW(hwnd_msg, L"\r\n!!!Transfer Error!!!");
			CloseHandle(file);
			CONNECTION = sock;
			return;
		}

		count++;
		if (count % (MAX_TEXT_W * 2 + 1) == 0)
			appendTextW(hwnd_msg, L"#");

	} while (byteread == sizeof(data));

	appendTextW(hwnd_msg, L"\r\n!!!Done transfer!!!");

	// Close handle
	CloseHandle(file);
	CONNECTION = sock;
}

void CALLBACK waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR sock, DWORD_PTR waveInHeader, DWORD_PTR dwParam2);
void audioTransfer(HWND hwnd, bool init, SOCKET sock) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);

	// Audio format
	WAVEFORMATEX pcmWaveFormat{};
	pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	pcmWaveFormat.nChannels = 1;
	pcmWaveFormat.nSamplesPerSec = 8000;
	pcmWaveFormat.nAvgBytesPerSec = 8000;
	pcmWaveFormat.nBlockAlign = 1;
	pcmWaveFormat.wBitsPerSample = 8;
	pcmWaveFormat.cbSize = 0;

	// Set up microphone
	HWAVEIN waveIn;
	waveInOpen(&waveIn, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)waveInProc, (DWORD_PTR)&sock, CALLBACK_FUNCTION);

	// Call request
	if (init) {
		BYTE cmd[2] = { 0, 4 };
		BYTE cipher[AES_PADDING]{};
		int len = AES_encrypt(G_hwnd_key, cmd, sizeof(cmd), cipher, sizeof(cipher));
		send(CONNECTION, (char*)cipher, len, 0);
		appendTextW(hwnd_msg, L"\r\n!!!Voice call request sent!!! Waiting for response...");

		// Wait for response for 3 min
		int timeout = 0;
		while (true) {
			Sleep(1000);
			if (CONNECTION == INVALID_SOCKET)
				break;
			if (timeout > 180 || SEND_MODE == 1) {
				SEND_MODE = 1;
				return;
			}
			timeout++;
		}
	}

	// Set up audio buffer
	char* waveInBuffer[AUDIO_BUFFER]{};
	WAVEHDR waveInHeader[AUDIO_BUFFER]{};
	for (int i = 0; i < AUDIO_BUFFER; i++) {
		waveInBuffer[i] = new char[MAX_TEXT_W * 2 - 1];
		waveInHeader[i].lpData = waveInBuffer[i];
		waveInHeader[i].dwBufferLength = MAX_TEXT_W * 2 - 1;
		waveInHeader[i].dwBytesRecorded = 0;
		waveInHeader[i].dwFlags = 0;
		waveInPrepareHeader(waveIn, &waveInHeader[i], sizeof(WAVEHDR));
		waveInAddBuffer(waveIn, &waveInHeader[i], sizeof(WAVEHDR));
	}

	// Start sending audio
	appendTextW(hwnd_msg, L"\r\n!!!Voice call connected!!!");
	waveInStart(waveIn);
	while (true) {
		Sleep(1000);
		if (SEND_MODE == 1)
			break;
	}
	waveInStop(waveIn);
	appendTextW(hwnd_msg, L"\r\n!!!Voice call disconnected!!!");

	// Clean up
	for (int i = 0; i < AUDIO_BUFFER; i++) {
		waveInUnprepareHeader(waveIn, &waveInHeader[i], sizeof(WAVEHDR));
		delete[] waveInBuffer[i];
	}

	waveInClose(waveIn);
}

void CALLBACK waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR sock, DWORD_PTR waveInHeader, DWORD_PTR dwParam2) {
	switch (message)
	{
	case WIM_DATA:
	{
		BYTE cipher[MAX_TEXT_W * 2];
		AES_encrypt(G_hwnd_key, (BYTE*)((WAVEHDR*)waveInHeader)->lpData, MAX_TEXT_W * 2 - 1, cipher, sizeof(cipher));
		if (send(*(SOCKET*)sock, (char*)cipher, sizeof(cipher), 0) <= 0) {
			SEND_MODE = 1;
			return;
		}

		waveInPrepareHeader(waveIn, (WAVEHDR*)waveInHeader, sizeof(WAVEHDR));
		waveInAddBuffer(waveIn, (WAVEHDR*)waveInHeader, sizeof(WAVEHDR));
		break;
	}
	}
}