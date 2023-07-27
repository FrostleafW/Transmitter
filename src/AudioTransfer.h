#pragma once

class AudioTransfer
{
	short numBuffer = 12;

	HWAVEIN waveIn = NULL;
	HWAVEOUT waveOut = NULL;

	BYTE* waveOutBuffer;

	HWND hwnd = NULL;
	HWND hwnd_msg = NULL;

	static void CALLBACK waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR sock, DWORD_PTR waveInHeader, DWORD_PTR dwParam2);
	static void CALLBACK waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2);

public:
	bool setup(HWND hwnd, SOCKET sock);

	void cleanup();
};

