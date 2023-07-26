#pragma once

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void createWidgets(HWND hwnd);

void callbackWidgets(HWND hwnd, int id);

void callbackSize(HWND hwnd);

WNDPROC TextboxProc;
LRESULT CALLBACK callbackTextbox(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
