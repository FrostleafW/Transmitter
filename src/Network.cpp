#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>

#include "misc.h"
#include "Network.h"

void Network::connection()
{
	if (!connect_encrypt()) {
		appendTextW(hwnd_msg, L"\r\nConnection failed...");
		return;
	}

	unsigned short offset = 0;
	BYTE cipher[MAX_TEXT_W * 2];
	BYTE data[MAX_TEXT_W * 2];
	int _s;
	while (true) {
		memset(cipher, 0, sizeof(cipher));
		memset(data, 0, sizeof(data));
		_s = recv(sock, (char*)cipher, sizeof(cipher), 0);
		if (_s > 0)
		{
			//AES_decrypt(G_hwnd_key, cipher, _s, data, sizeof(data));
			//// Receive command
			//if (data[0] == 0)
			//	Recv_cmd(hwnd, data);
			//// Receive normal text
			//else
			//	Recv_text(hwnd_msg, data);
		}
		else
		{
			appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
			break;
		}
	}
	key.Cleanup();

}

bool Network::connect_encrypt()
{
	// Wait until notified
	appendTextW(hwnd_msg, L"\r\nWaiting to start communication...");
	int _s;
	char id[2]{};
	_s = recv(sock, id, 2, 0);
	if (_s <= 0)
		return false;

	// Client #1 sets up RSA
	if (id[0] == 0x01) {
		// Create RSA key
		if (key.RSA_construct()) {
			// Send RSA public key
			BYTE public_key[MAX_TEXT_W]{};
			int len = key.RSA_exportPublic(public_key, sizeof(public_key));
			if (send(sock, (char*)public_key, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent RSA key...");

			// Wait for AES key
			BYTE aes_key_enc[MAX_TEXT_W]{};
			_s = recv(sock, (char*)aes_key_enc, sizeof(aes_key_enc), 0);
			if (_s <= 0)
				return false;

			// Decrypt AES key
			BYTE AES_key[16]{};
			if (key.RSA_decrypt(aes_key_enc, _s, AES_key, sizeof(AES_key)) > 0)
				appendTextW(hwnd_msg, L"\r\nReceived AES key...");
			else
				return false;

			// Destory RSA handle
			key.Cleanup();

			// Set up AES handle
			if (key.AES_construct(AES_key, sizeof(AES_key)))
				appendTextW(hwnd_msg, L"\r\nSecure communication established!");
			else
				return false;
		}
		else
			return false;
	}
	// Client #2 waits for RSA
	else if (id[0] == 0x02) {
		// Receive RSA public key
		BYTE public_key[MAX_TEXT_W]{};
		_s = recv(sock, (char*)public_key, sizeof(public_key), 0);
		if (_s <= 0)
			return false;
		appendTextW(hwnd_msg, L"\r\nReceived RSA key...");

		// Create AES key
		BYTE AES_key[16]{};
		key.AES_generateKey(AES_key, sizeof(AES_key));

		// Encrypt AES key
		if (key.RSA_importPublic(public_key, _s)) {
			BYTE aes_key_enc[MAX_TEXT_W]{};
			int len = key.RSA_encrypt(AES_key, sizeof(AES_key), aes_key_enc, sizeof(aes_key_enc));

			// Send AES key
			if (send(sock, (char*)aes_key_enc, len, 0) > 0)
				appendTextW(hwnd_msg, L"\r\nSent AES key...");
		}
		else
			return false;

		// Destory RSA handle
		key.Cleanup();

		// Set up AES handle
		if (key.AES_construct(AES_key, sizeof(AES_key)))
			appendTextW(hwnd_msg, L"\r\nSecure communication established!");
		else
			return false;
	}
	appendTextW(hwnd_msg, L"\r\n----------------------------------------");
	return true;
}

void Network::passHandle(HWND hwnd)
{
	this->hwnd = hwnd;
	this->hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	this->hwnd_clnt = GetDlgItem(hwnd, BTN_CLNT_ID);
}

bool Network::connect_to(WCHAR* ip, unsigned short port)
{
	EnableWindow(hwnd_clnt, false);

	appendTextW(hwnd_msg, L"\r\nTry connecting to ");
	appendTextW(hwnd_msg, ip);
	appendTextW(hwnd_msg, L":");
	appendNumber(hwnd_msg, port);
	appendTextW(hwnd_msg, L"...");

	// Set up socket
	SOCKET Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in addrServ{};
	InetPtonW(AF_INET, (PCWSTR)ip, &addrServ.sin_addr);
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);

	// Send request to server
	if (connect(Server, (SOCKADDR*)&addrServ, sizeof(SOCKADDR)) != 0) {
		appendTextW(hwnd_msg, L"Connection failed!");
		EnableWindow(hwnd_clnt, true);
		return false;
	}
	sock = Server;
	appendTextW(hwnd_msg, L"Succeeded!");
	std::thread connection_thread(&Network::connection, this);
	connection_thread.detach();
	return true;
}

