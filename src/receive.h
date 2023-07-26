#pragma once

struct FileInfo {
	char cmd[2] = { 0, 1 };
	WCHAR filename[MAX_PATH]{};
	DWORD filesize = 0;
};

bool Recv_file(HWND& hwnd, FileInfo* file_info) {
	if (MessageBoxW(hwnd, L"Receive?", L"Transmitter", MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return false;

	// Initialize savepath
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = file_info->filename;
	ofn.nMaxFile = sizeof(file_info->filename);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	if (!GetSaveFileName(&ofn))
		return false;

	HANDLE file = CreateFileW(file_info->filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);

	// Start receiving file
	SOCKET sock = CONNECTION;
	CONNECTION = INVALID_SOCKET;
	{
		BYTE cmd[2] = { 0, 2 };
		BYTE cipher[AES_PADDING]{};
		int len = AES_encrypt(G_hwnd_key, cmd, sizeof(cmd), cipher, sizeof(cipher));
		send(sock, (char*)cipher, len, 0);
	}
	appendTextW(hwnd_msg, L"\r\nStart receiving...(# -> 1MB)\r\n");

	BYTE cipher[MAX_TEXT_W * 2];
	BYTE data[MAX_TEXT_W * 2 - 1];
	int len;
	int count = 0;
	for (DWORD i = 0; i < file_info->filesize; i += MAX_TEXT_W * 2 - 1) {
		memset(cipher, 0, sizeof(cipher));
		memset(data, 0, sizeof(data));

		len = recv(sock, (char*)cipher, sizeof(cipher), 0);
		while (len != sizeof(cipher)) {
			int tmp_len = recv(sock, (char*)(cipher + len), sizeof(cipher) - len, 0);
			if (tmp_len <= 0)
				break;
			len += tmp_len;
		}
		if (AES_decrypt(G_hwnd_key, cipher, sizeof(cipher), data, sizeof(data)) == 0) {
			appendTextW(hwnd_msg, L"\r\n!!!Transfer Error!!!");
			CloseHandle(file);
			CONNECTION = sock;
			return true;
		}

		// Manage AES padding
		int lastbyte = sizeof(data);
		if (i > file_info->filesize - (MAX_TEXT_W * 2 - 1))
			lastbyte = file_info->filesize - i;
		WriteFile(file, data, lastbyte, NULL, NULL);

		count++;
		if (count % (MAX_TEXT_W * 2 + 1) == 0)
			appendTextW(hwnd_msg, L"#");
	}
	appendTextW(hwnd_msg, L"\r\n!!!Done transfer!!!");

	// Close handle
	CloseHandle(file);
	CONNECTION = sock;
	return true;
}

void Recv_text(HWND& hwnd_msg, BYTE* text) {
	appendTextW(hwnd_msg, L"\r\n> ");
	appendTextW(hwnd_msg, (WCHAR*)text);
}

void audioTransfer(HWND hwnd, bool init, SOCKET sock);
void waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2);
bool Recv_call(HWND& hwnd, bool init) {
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

	// Set up speaker
	HWAVEOUT waveOut;
	waveOutOpen(&waveOut, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)waveOutProc, NULL, CALLBACK_FUNCTION);

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
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	// File info
	if (cmd[1] == 1) {
		appendTextW(hwnd_msg, L"\r\n!!!A file is ready to be sent to you!!! Receive it?");
		appendTextW(hwnd_msg, L"\r\n\tFilename: ");
		appendTextW(hwnd_msg, ((FileInfo*)cmd)->filename);
		appendTextW(hwnd_msg, L"\r\n\tSize: ");
		appendFilesize(hwnd_msg, ((FileInfo*)cmd)->filesize);

		if (!Recv_file(hwnd, (FileInfo*)cmd)) {
			BYTE cmd[2] = { 0, 3 };
			BYTE cipher[AES_PADDING]{};
			int len = AES_encrypt(G_hwnd_key, cmd, sizeof(cmd), cipher, sizeof(cipher));
			send(CONNECTION, (char*)cipher, len, 0);
		}
	}
	// File ready to send
	else if (cmd[1] == 2) {
		CONNECTION = INVALID_SOCKET;
	}
	// File fail to send
	else if (cmd[1] == 3) {
		SEND_MODE = 1;
	}
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