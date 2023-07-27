#pragma once

void Btn_sendFile(HWND hwnd) {
	SEND_MODE = 2;
	std::thread file_tran(fileTransfer, hwnd);
	file_tran.detach();
}

void Btn_call(HWND hwnd) {

	// Check mic availability
	// Add ending key

	SEND_MODE = 3;
	std::thread file_tran(audioTransfer, hwnd, true, CONNECTION);
	file_tran.detach();
}
