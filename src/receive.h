#pragma once

struct FileInfo {
	char cmd[2] = { 0, 1 };
	WCHAR filename[MAX_PATH] = { 0 };
	DWORD filesize = 0;
};

void Recv_text(HWND& hwnd_msg, BYTE* text) {
	appendTextW(hwnd_msg, L"\r\n> ");
	appendTextW(hwnd_msg, (WCHAR*)text);
}

void Recv_file(HWND& hwnd, FileInfo* file_info) {
	// Initialize savepath
	OPENFILENAMEW ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = file_info->filename;
	ofn.nMaxFile = sizeof(file_info->filename);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	if (!GetSaveFileName(&ofn))
		return;

	HANDLE file = CreateFileW(file_info->filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);

	// Start receiving file
	SOCKET sock = CONNECTION;
	CONNECTION = INVALID_SOCKET;
	{
		BYTE cmd[2] = { 0, 2 };
		BYTE cipher[MAX_TEXT_W / 4] = { 0 };
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
		while (len != sizeof(cipher))
			len += recv(sock, (char*)(cipher + len), sizeof(cipher) - len, 0);
		AES_decrypt(G_hwnd_key, cipher, sizeof(cipher), data, sizeof(data));

		// Manage AES padding
		if (i > file_info->filesize - (MAX_TEXT_W * 2 - 1)) {
			int lastbyte = file_info->filesize - i;
			WriteFile(file, data, lastbyte, NULL, NULL);
		}
		else
			WriteFile(file, data, sizeof(data), NULL, NULL);

		count++;
		if (count % 1025 == 0)
			appendTextW(hwnd_msg, L"#");
	}
	appendTextW(hwnd_msg, L"\r\n!!!Done transfer.");

	// Close handle
	CloseHandle(file);
	CONNECTION = sock;
}

void Recv_cmd(HWND& hwnd, BYTE* cmd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	// File info
	if (cmd[1] == 1) {
		FileInfo* file_info = (FileInfo*)cmd;
		appendTextW(hwnd_msg, L"\r\n!!!A file is ready to be sent to you!!! Receive it?");
		appendTextW(hwnd_msg, L"\r\n\tFilename: ");
		appendTextW(hwnd_msg, file_info->filename);
		appendTextW(hwnd_msg, L"\r\n\tSize: ");
		appendFilesize(hwnd_msg, file_info->filesize);
		if (MessageBoxW(hwnd, L"Receive?", L"Transmitter", MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return;

		Recv_file(hwnd, file_info);
	}
	// File ready to send
	else if (cmd[1] == 2) {
		CONNECTION = INVALID_SOCKET;
	}
}
