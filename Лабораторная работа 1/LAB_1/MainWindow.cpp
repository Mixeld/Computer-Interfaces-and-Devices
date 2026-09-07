#include "MainWindow.h"
#include "SettingsWindow.h"
#include "Notifications.h"
#include "PowerMonitor.h"
#include "Reports.h"
#include <strsafe.h>

//========== ГЛАВНОЕ ОКНО ==========

//Оконная процедура
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        //Создание окна - добавляем иконку в трей
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

            //Запускаем таймер на 2 секунды (2000 мс)
            SetTimer(hwnd, 1, 2000, NULL);

            break;
        }

        //Обработка кликов по иконке в трее
        case WM_TRAY_NOTIFY: {
            if (lParam == WM_LBUTTONDBLCLK) {
                //Двойной клик - показываем статус
                ShowPowerStatusNotification();
            } else if (lParam == WM_RBUTTONUP) {
                //Правый клик - контекстное меню
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();

                //Добавляем пункты меню
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

        //Обработчик таймера
        case WM_TIMER: {
            if (wParam == 1) {
                CheckPowerStatus(hwnd);
            }
            break;
        }

        //Обработка команд из меню
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    //Выход из программы
                    g_isMonitoring = false;  //Останавливаем поток
                    Shell_NotifyIconW(NIM_DELETE, &nid);
                    PostQuitMessage(0);
                    break;

                case ID_TRAY_STATUS:
                    //Показать статус питания
                    ShowPowerStatusNotification();
                    break;

                case ID_TRAY_REPORT:
                    //Сохранить отчет сейчас
                    SaveReport();
                    break;

                case ID_TRAY_SETTINGS:
                    //Открыть окно настроек
                    OpenSettingsWindow(GetModuleHandle(NULL));
                    break;
            }
            break;
        }

        //Закрываем окно через Alt+F4 или крестик
        case WM_DESTROY: {
            g_isMonitoring = false;  //Останавливаем поток
            Shell_NotifyIconW(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}