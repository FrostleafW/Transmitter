#pragma once
#include "Encryption.h"
#include "FileTransfer.h"

class Network
{
	SOCKET sock = INVALID_SOCKET;
	int mode = 0; // 0: No connection; 1: Text; 2: Send file; 3: Receive file;
	bool occupy = false;
	unsigned int count = 0;

	Encryption key;
	FileTransfer file;

	HWND hwnd = NULL;
	HWND hwnd_msg = NULL;
	HWND hwnd_clnt = NULL;

	void connection();
	bool connect_encrypt();
	void send_data(BYTE* data, int len);
	void recv_cmd(BYTE* cmd);
	void recv_text(BYTE* text);
	void recv_file(BYTE* data);
	bool recv_fileinfo();

public:
	void passHandle(HWND hwnd);
	bool is_connected();
	bool connect_to(WCHAR* ip, unsigned short port);
	void send_text(WCHAR* text, int len);
	void send_file();
	void send_audio(char* audio, int len);
	void disconnect();
};

