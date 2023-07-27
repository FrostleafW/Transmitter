#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <thread>

#include "misc.h"
#include "Network.h"

void Network::connection()
{
	if (!connect_encrypt()) {
		disconnect();
		return;
	}

	mode = 1;
	BYTE cipher[MAX_TEXT_W * 2];
	BYTE data[MAX_TEXT_W * 2];
	int len;
	while (true) {
		memset(cipher, 0, sizeof(cipher));
		memset(data, 0, sizeof(data));
		len = recv(sock, (char*)cipher, sizeof(cipher), 0);
		if (len > 0)
		{
			if (occupy) {
				while (len != MAX_TEXT_W * 2) {
					int tmp_len = recv(sock, (char*)(cipher + len), sizeof(cipher) - len, 0);
					if (tmp_len <= 0) {
						sock = INVALID_SOCKET;
						break;
					}
					len += tmp_len;
				}
			}
			len = key.AES_decrypt(cipher, len, data, sizeof(data));
			if (len == 0) {
				appendTextW(hwnd_msg, L"\r\n!!!AES decryption failed...");
				continue;
			}
			if (mode == 1 || (len == 2 && data[0] == 0)) {
				// Receive command
				if (data[0] == 0)
					recv_cmd(data);
				// Receive normal text
				else
					recv_text(data);
			}
			// File transfer
			else if (mode == 2)
				recv_file(data);
		}
		else
		{
			disconnect();
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

void Network::send_data(BYTE* data, int len)
{
	if (len >= MAX_TEXT_W * 2) {
		appendTextW(hwnd_msg, L"\r\n!!!Sending data too long...");
		return;
	}
	BYTE* cipher = new BYTE[len + AES_PADDING];
	len = key.AES_encrypt(data, len, cipher, len + AES_PADDING);
	send(sock, (char*)cipher, len, 0);
	delete[] cipher;
}

void Network::recv_cmd(BYTE* cmd)
{
	// File info
	if (cmd[1] == 1) {
		file = new FileTransfer;
		file->load_fileinfo((FileInfo*)cmd);
		if (!recv_fileinfo()) {
			BYTE cmd[2] = { 0, 3 };
			send_data(cmd, 2);
			delete file;
			file = nullptr;
		}
	}
	// File ready to send
	else if (cmd[1] == 2) {
		occupy = true;
	}
	// File fail to send
	else if (cmd[1] == 3) {
		mode = 1;
	}
}

void Network::recv_text(BYTE* text)
{
	appendTextW(hwnd_msg, L"\r\n> ");
	appendTextW(hwnd_msg, (WCHAR*)text);
}

void Network::recv_file(BYTE* data)
{
	// Handle AES padding
	if (count < 2) {
		file->write_file(data, file->fileinfo.filesize);
		appendTextW(hwnd_msg, L"\r\n!!!Done transfer!!!");
		delete file;
		file = nullptr;
		occupy = false;
		mode = 1;
		return;
	}
	count--;

	file->fileinfo.filesize -= MAX_TEXT_W * 2 - 1;
	file->write_file(data, MAX_TEXT_W * 2 - 1);

	if (count % (1024 * 1024 / (MAX_TEXT_W * 2 - 1)) == 0)
		appendTextW(hwnd_msg, L"#");
}

bool Network::recv_fileinfo()
{
	appendTextW(hwnd_msg, L"\r\n!!!A file is ready to be sent to you!!! Receive it?");
	appendTextW(hwnd_msg, L"\r\n\tFilename: ");
	appendTextW(hwnd_msg, file->fileinfo.filename);
	appendTextW(hwnd_msg, L"\r\n\tSize: ");
	appendFilesize(hwnd_msg, file->fileinfo.filesize);

	if (MessageBoxW(hwnd, L"Receive it?", L"Transmitter", MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return false;

	// Initialize savepath
	if (!file->save_file(hwnd))
		return false;
	BYTE cmd[2] = { 0, 2 };
	send_data(cmd, 2);

	occupy = true;
	mode = 2;
	count = file->fileinfo.filesize % (MAX_TEXT_W * 2 - 1) == 0 ? file->fileinfo.filesize / (MAX_TEXT_W * 2 - 1) : file->fileinfo.filesize / (MAX_TEXT_W * 2 - 1) + 1;

	appendTextW(hwnd_msg, L"\r\nStart receiving...(# -> 1MB)\r\n");
	return true;
}

void Network::passHandle(HWND hwnd)
{
	this->hwnd = hwnd;
	this->hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);
	this->hwnd_clnt = GetDlgItem(hwnd, BTN_CLNT_ID);
}

bool Network::is_connected()
{
	return sock != INVALID_SOCKET && mode == 1;
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
		appendTextW(hwnd_msg, L"Failed!");
		EnableWindow(hwnd_clnt, true);
		return false;
	}
	sock = Server;
	appendTextW(hwnd_msg, L"Succeeded!");
	std::thread connection_thread([&] {connection(); });
	connection_thread.detach();
	return true;
}

void Network::send_text(WCHAR* text, int len)
{
	appendTextW(hwnd_msg, L"\r\n< ");
	appendTextW(hwnd_msg, text);
	send_data((BYTE*)text, len * 2);
}

void Network::send_file()
{
	file = new FileTransfer;
	if (!file->open_file(hwnd))
		return;

	// Send file info
	send_data((BYTE*)&file->fileinfo, sizeof(FileInfo));
	appendTextW(hwnd_msg, L"\r\n!!!You tried to send a file!!! Waiting for response...");
	appendTextW(hwnd_msg, L"\r\n\tFilename: ");
	appendTextW(hwnd_msg, file->fileinfo.filename);
	appendTextW(hwnd_msg, L"\r\n\tSize: ");
	appendFilesize(hwnd_msg, file->fileinfo.filesize);
	mode = 2;

	// Wait for response for 50 sec
	int timeout = 0;
	while (true) {
		Sleep(500);
		if (occupy == true)
			break;
		if (mode != 2) {
			appendTextW(hwnd_msg, L"\r\nFile rejected...");
			return;
		}
		if (timeout > 100) {
			appendTextW(hwnd_msg, L"\r\nFile time out...");
			mode = 1;
			return;
		}
		timeout++;
	}

	appendTextW(hwnd_msg, L"\r\nStart transferring...(# -> 1MB)\r\n");

	// Start sending file
	BYTE data[MAX_TEXT_W * 2 - 1];
	DWORD byteread = 0;
	int count = 0;
	do {
		memset(data, 0, sizeof(data));

		byteread = file->read_file(data, sizeof(data));
		send_data(data, sizeof(data));

		if (mode != 2) {
			appendTextW(hwnd_msg, L"\r\n!!!Transfer failed!!!");
			delete file;
			file = nullptr;
			return;
		}

		count++;
		if (count % (1024 * 1024 / (MAX_TEXT_W * 2 - 1)) == 0)
			appendTextW(hwnd_msg, L"#");

	} while (byteread == sizeof(data));

	appendTextW(hwnd_msg, L"\r\n!!!Done transfer!!!");
	delete file;
	file = nullptr;
	occupy = false;
	mode = 1;
}

void Network::send_audio(char* audio, int len)
{
}

void Network::disconnect()
{
	mode = 0;
	occupy = false;
	delete file;
	file = nullptr;
	key.Cleanup();
	closesocket(sock);
	sock = INVALID_SOCKET;
	appendTextW(hwnd_msg, L"\r\n!!!Connection stopped...");
	EnableWindow(hwnd_clnt, true);
}

Network::~Network() {
	delete file;
	file = nullptr;
}


