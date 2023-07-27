#pragma once

// Windows ID
#define MSGBOX_ID 10
#define BTN_CLNT_ID 11
#define TEXTBOX_ID 12
#define BTN_SEND_ID 13
#define IPBOX_ID 14
#define PORTBOX_ID 15
#define BTN_FILE_ID 16
#define BTN_CALL_ID 17

// Text length limit
#define AES_PADDING 16
#define MAX_TEXT_W 1024

#define AUDIO_BUFFER 16

#define DEFAULT_PORT 17010

void appendTextW(HWND& hwnd_msg, const WCHAR* buffer);

void appendTextA(HWND& hwnd_msg, const char* buffer);

void appendNumber(HWND& hwnd_msg, long long num);

void appendFilesize(HWND& hwnd_msg, DWORD filesize);

void appendTextByte(HWND& hwnd_msg, BYTE* buffer, int len);

int getText(HWND hwnd, WCHAR* text);

USHORT getPort(HWND& hwnd);
