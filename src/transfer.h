#pragma once


void Btn_call(HWND hwnd) {

	// Check mic availability
	// Add ending key

	SEND_MODE = 3;
	std::thread file_tran(audioTransfer, hwnd, true, CONNECTION);
	file_tran.detach();
}
