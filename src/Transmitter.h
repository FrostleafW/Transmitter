#pragma once
#include <thread>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "winmm.lib")
#include "misc.h"
#include "encryption.h"
#include "receive.h"
#include "socket.h"
#include "transfer.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void createWidgets(HWND hwnd);

void callbackWidgets(HWND hwnd, int id);

void callbackSize(HWND hwnd);

WNDPROC TextboxProc;
LRESULT CALLBACK callbackTextbox(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
