#pragma once
#include "Encryption.h"

class Network
{
	SOCKET sock = INVALID_SOCKET;

	Encryption key;

	HWND hwnd;
	HWND hwnd_msg;
	HWND hwnd_clnt;

	void connection();
	bool connect_encrypt();
	bool send_data(BYTE* data, int len);

public:
	void passHandle(HWND hwnd);
	bool connect_to(WCHAR* ip, unsigned short port);
	bool send_text(WCHAR* text, int len);
	bool send_file(WCHAR* filename, int len);
	bool send_audio(char* audio, int len);
};

