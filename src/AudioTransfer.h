#pragma once

class AudioTransfer
{
	static const short numBuffer = 16;

	HWAVEIN waveIn = NULL;
	HWAVEOUT waveOut = NULL;

	WAVEHDR waveInHeader[numBuffer]{};
	BYTE* waveInBuffer[numBuffer]{};
	WAVEHDR waveOutHeader[numBuffer / 2]{};
	BYTE* waveOutBuffer[numBuffer / 2]{};

	HWND hwnd = NULL;
	HWND hwnd_msg = NULL;

	static void CALLBACK waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR sock, DWORD_PTR waveInHeader, DWORD_PTR dwParam2);
	static void CALLBACK waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2);

public:
	bool start(HWND hwnd, void* conn);
	void setupBuffer();
	void audioInStart(int& mode);
	void audioOut(BYTE* data, unsigned int& count);
	void cleanup();
	~AudioTransfer();
};

