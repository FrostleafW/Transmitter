#include <winsock2.h>

#include "misc.h"
#include "AudioTransfer.h"

void AudioTransfer::waveInProc(HWAVEIN waveIn, UINT message, DWORD_PTR sock, DWORD_PTR waveInHeader, DWORD_PTR dwParam2)
{
}

void AudioTransfer::waveOutProc(HWAVEOUT waveOut, UINT message, DWORD_PTR dwInstance, DWORD_PTR waveOutHeader, DWORD_PTR dwParam2)
{
}

bool AudioTransfer::setup(HWND hwnd, SOCKET sock)
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
	HWAVEIN waveIn;
	if (waveInOpen(&waveIn, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)&AudioTransfer::waveInProc, (DWORD_PTR)&sock, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		appendTextW(hwnd_msg, L"\r\n!!!Cannot detect microphone...");
		return false;
	}

	// Set up speaker
	HWAVEOUT waveOut;
	if (waveOutOpen(&waveOut, WAVE_MAPPER, &pcmWaveFormat, (DWORD_PTR)&AudioTransfer::waveOutProc, NULL, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		appendTextW(hwnd_msg, L"\r\n!!!Cannot detect speaker...");
		return false;
	}

	return true;
}

void AudioTransfer::cleanup()
{
	waveInClose(waveIn);
	waveOutClose(waveOut);
}
