#pragma once

#define DEFAULT_PORT 20710

void Server(HWND hwnd_msg) {
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WSACleanup();
		return;
	}

	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addrServ;
	addrServ.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(DEFAULT_PORT);

	// Setup the TCP listening socket
	bind(ServerSocket, (sockaddr*)&addrServ, sizeof(SOCKADDR));
	listen(ServerSocket, 3);

	// Unblock server
	u_long argp = 1;
	ioctlsocket(ServerSocket, FIONBIO, &argp);

	// Keep trying accepting
	SOCKET ClientSocket = accept(ServerSocket, NULL, NULL);
	while (ClientSocket == INVALID_SOCKET) {
		Sleep(1000);
		appendText(hwnd_msg, ".");
		ClientSocket = accept(ServerSocket, NULL, NULL);
	}
	closesocket(ServerSocket);

	// Establish connection
	appendText(hwnd_msg, "Succeeded!\n");
	char buffer[] = "hello world\n";
	while (true) {
		send(ClientSocket, buffer, 13, 0);
		Sleep(2000);
	}
}

SOCKET Client() {
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WSACleanup();
		return INVALID_SOCKET;
	}

	SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN addrServ;
	inet_pton(AF_INET, (PCSTR)"127.0.0.1", &addrServ.sin_addr);
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(DEFAULT_PORT);

	// Send request to server
	connect(ClientSocket, (SOCKADDR*)&addrServ, sizeof(SOCKADDR));
	return ClientSocket;
}

void Connection(SOCKET sock, HWND hwnd_msg) {
	while (sock != INVALID_SOCKET) {

	}
}