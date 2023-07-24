#pragma once

struct FileInfo {
	char cmd[2] = { 0, 1 };
	WCHAR filename[MAX_PATH] = { 0 };
	DWORD fileblocks = 0;
};

void Recv_text(HWND& hwnd_msg, BYTE* text) {
	appendTextW(hwnd_msg, L"\r\n> ");
	appendTextW(hwnd_msg, (WCHAR*)text);
}

void Recv_cmd(HWND& hwnd, BYTE* cmd) {
	HWND hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	HWND hwnd_file = GetDlgItem(hwnd, BTN_FILE_ID);
	// File info
	if (cmd[1] == 1) {
		FileInfo* file_info = (FileInfo*)cmd;
		appendTextW(hwnd_msg, L"\r\n!!!A file is ready to be sent to you!!! Receive it?");
		appendTextW(hwnd_msg, L"\r\nFilename: ");
		appendTextW(hwnd_msg, file_info->filename);
		appendTextW(hwnd_msg, L"\r\nSize: ");
		appendFilesize(hwnd_msg, file_info->fileblocks);
		SetWindowTextW(hwnd_file, L"Recv File");
		if (MessageBoxW(hwnd, L"Receive?", L"Transmitter", MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return;

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

		CreateFileW(file_info->filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


		//SOCKET sock = CONNECTION;
		//CONNECTION = INVALID_SOCKET;




		//char cmd[2] = { 0, 2 };
		//send(sock, cmd, sizeof(cmd), 0);
	}
	// File ready to send
	else if (cmd[1] == 2) {
		CONNECTION = INVALID_SOCKET;
	}
}
