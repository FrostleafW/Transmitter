#pragma once
#include "Encryption.h"
#include "FileTransfer.h"
#include "AudioTransfer.h"

class Network
{
	SOCKET sock = INVALID_SOCKET;
	int mode = 0; // 0: No connection; 1: Text; 2: Send file; 3: Receive file; 4: Call request; 5: Call accept; 6: Call reject; 7: Call hang up
	bool occupy = false;
	unsigned int count = 0;

	Encryption key;
	FileTransfer file;
	AudioTransfer audio;

	HWND hwnd = NULL;
	HWND hwnd_msg = NULL;
	HWND hwnd_clnt = NULL;
	HWND hwnd_call = NULL;

	void connection();
	bool connect_encrypt();
	void recv_cmd(BYTE* cmd);
	void recv_text(BYTE* text);
	void recv_file(BYTE* data);
	void recv_audio(BYTE* data);
	bool recv_fileinfo();
	bool recv_call();

public:
	void passHandle(HWND hwnd);
	bool is_connected();
	bool connect_to(WCHAR* ip, unsigned short port);
	void send_data(BYTE* data, int len);
	void send_text(WCHAR* text, int len);
	void send_file();
	void send_audio();
	void hangup_audio();
	void disconnect();

};

