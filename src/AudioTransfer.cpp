#include <winsock2.h>

#include <stdio.h>

#include "misc.h"
#include "Network.h"
#include "AudioTransfer.h"

void AudioTransfer::waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR conn, DWORD_PTR waveInHeader, DWORD_PTR dwParam2)
{
	switch (message)
	{
	case WIM_DATA:
	{
		// Send audio
		((Network*)conn)->send_data((BYTE*)((WAVEHDR*)waveInHeader)->lpData, ((WAVEHDR*)waveInHeader)->dwBufferLength);

		waveInPrepareHeader(waveIn, (WAVEHDR*)waveInHeader, sizeof(WAVEHDR));
		waveInAddBuffer(waveIn, (WAVEHDR*)waveInHeader, sizeof(WAVEHDR));
		break;
	}
	}
}

void AudioTransfer::waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2)
{
	switch (message) {
	case WOM_DONE:
	{
		waveOutPrepareHeader(waveOut, (WAVEHDR*)waveOutHeader, sizeof(WAVEHDR));
		break;
	}
	}
}

bool AudioTransfer::start(HWND hwnd, void* conn)
{
	this->hwnd = hwnd;
	hwnd_msg = GetDlgItem(hwnd, MSGBOX_ID);

	// Audio format
	WAVEFORMATEX pcmWaveFormat{};
	pcmWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	pcmWaveFormat.nChannels = 1;
	pcmWaveFormat.nSamplesPerSec = 8000;
	pcmWaveFormat.nAvgBytesPerSec = 8000;
	pcmWaveFormat.nBlockAlign = 1;
	pcmWaveFormat.wBitsPerSample = 8;
	pcmWaveFormat.cbSize = 0;

	// Set up microphone
	if (waveInOpen(&waveIn, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)&AudioTransfer::waveInProc, (DWORD_PTR)conn, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		appendTextW(hwnd_msg, L"\r\n!!!Cannot detect microphone...");
		waveInClose(waveIn);
		waveIn = NULL;
		return false;
	}

	// Set up speaker
	if (waveOutOpen(&waveOut, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)&AudioTransfer::waveOutProc, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		appendTextW(hwnd_msg, L"\r\n!!!Cannot detect speaker...");
		waveInClose(waveIn);
		waveOutClose(waveOut);
		waveIn = NULL;
		waveOut = NULL;
		return false;
	}

	return true;
}

void AudioTransfer::setupBuffer() {
	// Set up audio buffer 
	for (int i = 0; i < numBuffer; i++) {
		waveInBuffer[i] = new BYTE[MAX_TEXT_W * 2 - 1];
		waveInHeader[i].lpData = (char*)waveInBuffer[i];
		waveInHeader[i].dwBufferLength = MAX_TEXT_W * 2 - 1;
		waveInHeader[i].dwBytesRecorded = 0;
		waveInHeader[i].dwFlags = 0;
		waveInPrepareHeader(waveIn, &waveInHeader[i], sizeof(WAVEHDR));
		waveInAddBuffer(waveIn, &waveInHeader[i], sizeof(WAVEHDR));
	}
	for (int i = 0; i < numBuffer / 2; i++) {
		waveOutBuffer[i] = new BYTE[MAX_TEXT_W * 2 - 1];
		waveOutHeader[i].lpData = (char*)waveOutBuffer[i];
		waveOutHeader[i].dwBufferLength = MAX_TEXT_W * 2 - 1;
		waveOutHeader[i].dwFlags = 0;
		waveOutPrepareHeader(waveOut, &waveOutHeader[i], sizeof(WAVEHDR));
	}
}

void AudioTransfer::audioInStart(int& mode)
{
	waveInStart(waveIn);
	while (mode == 5) {
		Sleep(500);
	}
		
	waveInStop(waveIn);
	appendTextW(hwnd_msg, L"\r\n!!!Voice call disconnected!!!");
}

void AudioTransfer::audioOut(BYTE* data, unsigned int& count)
{
	memcpy(waveOutBuffer[count], data, waveOutHeader[count].dwBufferLength);

	// Play audio
	waveOutWrite(waveOut, &waveOutHeader[count], sizeof(WAVEHDR));
	count++;
	if (count == numBuffer / 2)
		count = 0;
}

void AudioTransfer::cleanup()
{
	for (int i = 0; i < numBuffer; i++) {
		delete[] waveInBuffer[i];
		waveInBuffer[i] = nullptr;
	}
	for (int i = 0; i < numBuffer / 2; i++) {
		delete[] waveOutBuffer[i];
		waveOutBuffer[i] = nullptr;
	}
	waveInClose(waveIn);
	waveOutClose(waveOut);
	waveIn = NULL;
	waveOut = NULL;
}

AudioTransfer::~AudioTransfer() {
	cleanup();
}
