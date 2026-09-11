#include "MainWindow.h"
#include "Notifications.h"
#include "PowerMonitor.h"
#include "Reports.h"
#include "SettingsDialog.h"   // ← Qt-окно настроек
#include <strsafe.h>

//========== ГЛАВНОЕ ОКНО ==========

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = hwnd;
            nid.uID = 1;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_TRAY_NOTIFY;
            nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);

            StringCchCopyW(nid.szTip, 128, L"Лабораторная работа 1");

            Shell_NotifyIconW(NIM_ADD, &nid);
            ShowNotification(L"Лабораторная работа 1", L"Приложение запущено!");

            SetTimer(hwnd, 1, 2000, NULL);
            break;
        }

        case WM_TRAY_NOTIFY: {
            if (lParam == WM_LBUTTONDBLCLK) {
                ShowPowerStatusNotification();
            } else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();

                AppendMenuW(hMenu, MF_STRING, ID_TRAY_STATUS,   L"Показать статус");
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_REPORT,   L"Сохранить отчет");
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_SETTINGS, L"Настройки");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT,     L"Выход");

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                PostMessage(hwnd, WM_NULL, 0, 0);
                DestroyMenu(hMenu);
            }
            break;
        }

        case WM_TIMER: {
            if (wParam == 1) {
                CheckPowerStatus(hwnd);
            }
            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    g_isMonitoring = false;
                    Shell_NotifyIconW(NIM_DELETE, &nid);
                    PostQuitMessage(0);
                    break;

                case ID_TRAY_STATUS:
                    ShowPowerStatusNotification();
                    break;

                case ID_TRAY_REPORT:
                    SaveReport();
                    break;

                case ID_TRAY_SETTINGS:
                    OpenQtSettingsWindow();   // ← Qt вместо WinAPI
                    break;
            }
            break;
        }

        case WM_DESTROY: {
            g_isMonitoring = false;
            Shell_NotifyIconW(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}