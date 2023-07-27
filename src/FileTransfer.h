#pragma once

struct FileInfo {
	char cmd[2] = { 0, 1 };
	DWORD filesize = 0;
	WCHAR filename[MAX_PATH / 2]{};
};

class FileTransfer
{
	HANDLE file = NULL;
	WCHAR filepath[MAX_PATH]{};

public:
	FileInfo fileinfo;

	bool open_file(HWND hwnd);
	bool save_file(HWND hwnd);
	void load_fileinfo(FileInfo* fileinfo);
	DWORD read_file(BYTE* data, int len);
	bool write_file(BYTE* data, int len);
	~FileTransfer();
};

