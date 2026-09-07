#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "Globals.h"

//Функция окна настроек
LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Функция открытия окна настроек
void OpenSettingsWindow(HINSTANCE hInstance);

#endif // SETTINGSWINDOW_H