#pragma once

void audioTransfer(HWND hwnd, bool init, SOCKET sock);

bool Recv_call(HWND& hwnd, bool init) {


	

	// Set up audio buffer 
	BYTE* waveOutBuffer[AUDIO_BUFFER / 2]{};
	WAVEHDR waveOutHeader[AUDIO_BUFFER / 2]{};
	for (int i = 0; i < AUDIO_BUFFER / 2; i++) {
		waveOutBuffer[i] = new BYTE[MAX_TEXT_W * 2 - 1];
		waveOutHeader[i].lpData = (LPSTR)waveOutBuffer[i];
		waveOutHeader[i].dwBufferLength = MAX_TEXT_W * 2 - 1;
		waveOutHeader[i].dwFlags = 0;
		waveOutPrepareHeader(waveOut, &waveOutHeader[i], sizeof(WAVEHDR));
	}

	// Start voice call
	SOCKET sock = CONNECTION;
	CONNECTION = INVALID_SOCKET;
	if (init) {
		BYTE cmd[2] = { 0, 5 };
		BYTE cipher[AES_PADDING]{};
		int len = AES_encrypt(G_hwnd_key, cmd, sizeof(cmd), cipher, sizeof(cipher));
		send(sock, (char*)cipher, len, 0);

		// Start sending audio
		std::thread file_tran(audioTransfer, hwnd, false, sock);
		file_tran.detach();
	}

	// Start receiving audio
	BYTE cipher[MAX_TEXT_W * 2];
	int len;
	int count = 0;
	while (true) {
		memset(cipher, 0, sizeof(cipher));
		memset(waveOutBuffer[count], 0, waveOutHeader[count].dwBufferLength);

		len = recv(sock, (char*)cipher, sizeof(cipher), 0);
		while (len != sizeof(cipher)) {
			int tmp_len = recv(sock, (char*)(cipher + len), sizeof(cipher) - len, 0);
			if (tmp_len <= 0)
				break;
			len += tmp_len;
		}
		if (AES_decrypt(G_hwnd_key, cipher, sizeof(cipher), waveOutBuffer[count], waveOutHeader[count].dwBufferLength) == 0) {
			appendTextW(hwnd_msg, L"\r\n!!!Transfer Error!!!");
			break;
		}
		// Play audio
		waveOutWrite(waveOut, &waveOutHeader[count], sizeof(WAVEHDR));

		count++;
		if (count == AUDIO_BUFFER / 2)
			count = 0;
	}

	for (int i = 0; i < AUDIO_BUFFER / 2; i++) {
		waveOutUnprepareHeader(waveOut, &waveOutHeader[i], sizeof(WAVEHDR));
		delete[] waveOutBuffer[i];
	}

	waveOutClose(waveOut);
	CONNECTION = sock;
	return true;
}

void waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2)
{
	switch (message) {
	case WOM_DONE:
	{
		waveOutPrepareHeader(waveOut, (WAVEHDR*)waveOutHeader, sizeof(WAVEHDR));
		break;
	}
	}
}

void Recv_cmd(HWND& hwnd, BYTE* cmd) {

	
	// Call request
	else if (cmd[1] == 4) {
		appendTextW(hwnd_msg, L"\r\n!!!The other side wants to start a voice call!!!");
		if (!(MessageBoxW(hwnd, L"Start a voice call?", L"Transmitter", MB_YESNO | MB_DEFBUTTON2) == IDYES && Recv_call(hwnd, true))) {
			BYTE cmd[2] = { 0, 6 };
			BYTE cipher[AES_PADDING]{};
			int len = AES_encrypt(G_hwnd_key, cmd, sizeof(cmd), cipher, sizeof(cipher));
			send(CONNECTION, (char*)cipher, len, 0);
		}
	}
	// Call ready
	else if (cmd[1] == 5) {
		CONNECTION = INVALID_SOCKET;
		Recv_call(hwnd, false);
	}
	// Call terminate
	else if (cmd[1] == 6) {
		SEND_MODE = 1;
	}
}