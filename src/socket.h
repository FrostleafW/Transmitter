#pragma once



void audioTransfer(HWND hwnd, bool init, SOCKET sock) {



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