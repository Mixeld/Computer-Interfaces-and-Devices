#include "SettingsWindow.h"
#include "Settings.h" // <-- ЭТА СТРОКА ОБЯЗАТЕЛЬНА ДЛЯ SaveSettings()

//========== ОКНО НАСТРОЕК ==========

//Функция окна настроек
LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            //Создаем надписи и поля ввода
            CreateWindowW(L"STATIC", L"Порог критического заряда (%):",
                         WS_CHILD | WS_VISIBLE, 20, 20, 180, 25, hwnd, NULL, NULL, NULL);

            CreateWindowW(L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                         210, 20, 50, 25, hwnd, (HMENU)101, NULL, NULL);

            CreateWindowW(L"STATIC", L"Интервал отчетов (мин):",
                         WS_CHILD | WS_VISIBLE, 20, 60, 180, 25, hwnd, NULL, NULL, NULL);

            CreateWindowW(L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
                         210, 60, 50, 25, hwnd, (HMENU)102, NULL, NULL);

            CreateWindowW(L"BUTTON", L"OK",
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         70, 110, 80, 30, hwnd, (HMENU)IDOK, NULL, NULL);

            CreateWindowW(L"BUTTON", L"Отмена",
                         WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                         170, 110, 80, 30, hwnd, (HMENU)IDCANCEL, NULL, NULL);

            //Загружаем текущие настройки в поля
            wchar_t buffer[32];
            wsprintfW(buffer, L"%d", g_criticalThreshold);
            SetDlgItemTextW(hwnd, 101, buffer);

            wsprintfW(buffer, L"%d", g_reportInterval);
            SetDlgItemTextW(hwnd, 102, buffer);

            break;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDOK: {
                    //Читаем порог
                    wchar_t buffer[32];
                    GetDlgItemTextW(hwnd, 101, buffer, 32);
                    int newThreshold = _wtoi(buffer);

                    if (newThreshold >= 5 && newThreshold <= 50) {
                        g_criticalThreshold = newThreshold;
                    } else {
                        MessageBoxW(hwnd, L"Порог должен быть от 5 до 50%", L"Ошибка", MB_OK);
                        return TRUE;
                    }

                    //Читаем интервал
                    GetDlgItemTextW(hwnd, 102, buffer, 32);
                    int newInterval = _wtoi(buffer);

                    if (newInterval >= 1 && newInterval <= 60) {
                        g_reportInterval = newInterval;
                    } else {
                        MessageBoxW(hwnd, L"Интервал должен быть от 1 до 60 минут", L"Ошибка", MB_OK);
                        return TRUE;
                    }

                    //Сохраняем настройки в файл
                    SaveSettings(); // <-- Использует объявление из Settings.h

                    DestroyWindow(hwnd);
                    hwndSettings = nullptr;
                    break;
                }

                case IDCANCEL:
                    DestroyWindow(hwnd);
                    hwndSettings = nullptr;
                    break;
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            hwndSettings = nullptr;
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//Функция открытия окна настроек
void OpenSettingsWindow(HINSTANCE hInstance) {
    if (hwndSettings != nullptr) {
        SetForegroundWindow(hwndSettings);
        return;
    }

    hwndSettings = CreateWindowW(
        L"SettingsClass",
        L"Настройки",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        320, 200,
        NULL, NULL,
        hInstance, NULL
    );
}