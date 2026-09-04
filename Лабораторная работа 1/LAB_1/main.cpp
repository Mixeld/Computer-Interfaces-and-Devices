#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0600         //Указываем версию винды

#include <windows.h>                //WinAPI
#include <strsafe.h>                //Функции работы со строками
#include <string>               

#define WM_TRAY_NOTIFY (WM_USER + 1)    //Если пользователь жмёт на иконку => сообщение приходит в WndProc
#define ID_TRAY_EXIT 1001
#define ID_TRAY_STATUS 1002             // ID для пункта "Показать статус"
#define CRITICAL_THRESHOLD 15           //Критический заряд!!!

using namespace std;

// Глобальные переменные
NOTIFYICONDATA nid = {};        //Инфа по иконке в трее
HWND hwndMain = nullptr;        //Дескриптор главного окна

//Структура для состояния питания
struct PowerState {
    SYSTEM_POWER_STATUS status;
    bool ACOnline;
    bool Critical_Charge;
    bool Full_Charge;
    
    PowerState() : ACOnline(false), Critical_Charge(false), Full_Charge(false) {} //Выставляем значения по умолчанию
};

PowerState currentState;

void ShowNotification(const wstring& title, const wstring& message, int iconType = NIIF_INFO) {
    NOTIFYICONDATA nidNotify = nid;
    nidNotify.uFlags = NIF_INFO;            // Показываем уведомление 
    nidNotify.dwInfoFlags = NIIF_INFO;      // Информационное
    nidNotify.uTimeout = 10000;              // 5 сек
    
    //ЖЁСТКО КОПИРУЕМ СТРОКИ
    StringCchCopyW(nidNotify.szInfoTitle, 64, title.c_str());
    StringCchCopyW(nidNotify.szInfo, 256, message.c_str());
    
    //Обновляем иконку
    Shell_NotifyIcon(NIM_MODIFY, &nidNotify);
}

void CheckPowerStatus (HWND hwnd) {
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status)) {
        return; //Если статус не получили то выходим
    }

    bool Now_ACOnline = (status.ACLineStatus ==1);

    if (Now_ACOnline != currentState.ACOnline){
        if (Now_ACOnline){
            ShowNotification(L"Питание", L"Зарядка подключена");
        } else {
            ShowNotification(L"Питание", L"Зарядка отключена");
        }

        int batteryPercent = status.BatteryLifePercent;
        bool Charging =(status.BatteryFlag & 8) != 0;

        bool Now_Critical_Charge = (batteryPercent < CRITICAL_THRESHOLD && batteryPercent != 255);

        if (Now_Critical_Charge && !currentState.Critical_Charge){
            wchar_t msg [256];
            wsprintfW(msg, L"Критический заряд!! %d%%", batteryPercent);
            ShowNotification(L"Критический заряд!", msg, NIIF_WARNING);
            currentState.Critical_Charge = true;
        } else if (!Now_Critical_Charge){
            currentState.Critical_Charge = false;   //Если заряд стал выше порога
        }

        bool Now_Full_Charge = (batteryPercent == 100 && batteryPercent != 255);
        
        if (Now_Full_Charge && !currentState.Full_Charge && Now_ACOnline) {
            ShowNotification(L"Зарядились йоу!!", L"Батарея полностью заряжена", NIIF_INFO);
            currentState.Full_Charge = true;
        } else if (!Now_Full_Charge) {
            currentState.Full_Charge = false;
        }

        currentState.ACOnline = Now_ACOnline;
    }

    currentState.status = status;
}

void ShowPowerStatusNotification() {
    SYSTEM_POWER_STATUS status;
    
    if (!GetSystemPowerStatus(&status)) {
        ShowNotification(L"Ошибка", L"Не удалось получить статус питания", NIIF_ERROR);
        return;
    }
    
    wchar_t msg[512];
    int percent = status.BatteryLifePercent;
    
    wstring acStatus;
    switch(status.ACLineStatus) {
        case 0: acStatus = L"Отключено (батарея)"; break;
        case 1: acStatus = L"Подключено (сеть)"; break;
        default: acStatus = L"Неизвестно";
    }
    
    wstring chargeStatus = (status.BatteryFlag & 8) ? L"Заряжается" : L"Не заряжается";
    
    if (percent != 255) {
        wsprintfW(msg, L"Уровень заряда: %d%%\nСеть: %s\nСостояние: %s", percent, acStatus.c_str(), chargeStatus.c_str());
    } else {
        wsprintfW(msg, L"Уровень заряда: неизвестен\nСеть: %s\nСостояние: %s", acStatus.c_str(), chargeStatus.c_str());
    }
    
    ShowNotification(L"Статус питания", msg, NIIF_INFO);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        //Всплывающее окно
        case WM_CREATE: {
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uID = 1;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_TRAY_NOTIFY;
            nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
            
            StringCchCopyW(nid.szTip, 128, L"Лабораторная работа 1");
            
            Shell_NotifyIcon(NIM_ADD, &nid);
            ShowNotification(L"Лабораторная работа 1", L"Приложение запущено!");
            
            SetTimer(hwnd, 1, 200, NULL);
            
            break;

        }

        //Двойное нажатие в трее
        case WM_TRAY_NOTIFY: {
            if (lParam == WM_LBUTTONDBLCLK) {
                ShowPowerStatusNotification();
            } else if (lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Показать статус");
                AppendMenuW(hMenu, MFT_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Выход");

                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                PostMessage(hwnd, WM_NULL, 0, 0);
                DestroyMenu(hMenu);
            }
            break;
        }

        //Обработчик таймера
        case WM_TIMER: {
            if(wParam == 1){
                CheckPowerStatus(hwnd);
            }
            break;
        }

        //Закрываем приложение
        case WM_COMMAND: {
               switch (LOWORD(wParam)) {
                    case ID_TRAY_EXIT:
                        Shell_NotifyIcon(NIM_DELETE, &nid);
                        PostQuitMessage(0);
                        break;
            

                    case ID_TRAY_STATUS:
                        ShowPowerStatusNotification();
                        break;
                }
        break;
        }

        //Закрываем окно через Alt+F4
        case WM_DESTROY: {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"PowerMonitorClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExW(&wc);

    hwndMain = CreateWindowExW(
        0,
        L"PowerMonitorClass",
        L"Power Monitor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 300,
        NULL, NULL,
        hInstance, NULL
    );

    if (!hwndMain) {
        MessageBoxW(NULL, L"Ошибка создания окна!", L"Ошибка", MB_OK);
        return 1;
    }

    ShowWindow(hwndMain, SW_HIDE);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}