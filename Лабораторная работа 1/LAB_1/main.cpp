#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0600         //Указываем версию винды

#include <windows.h>                //WinAPI
#include <strsafe.h>                //Функции работы со строками
#include <string>               
#include <fstream>                  //Для работы с файлами
#include <ctime>                    //Для работы со временем
#include <thread>                   //Для отдельного потока
#include <chrono>                   //Для задержек
#include <powrprof.h>               //Для получения схемы питания

#define NTDDI_VERSION 0x06000000
#define _WIN32_WINNT 0x0600

#include <powrprof.h>

#pragma comment(lib, "powrprof.lib")  //Линкуем 

#define WM_TRAY_NOTIFY (WM_USER + 1)    //Если пользователь жмёт на иконку => сообщение приходит в WndProc
#define ID_TRAY_EXIT 1001
#define ID_TRAY_STATUS 1002             // ID для пункта "Показать статус"
#define ID_TRAY_SETTINGS 1003           // ID для пункта "Настройки"
#define ID_TRAY_REPORT 1004             // ID для пункта "Сохранить отчет"

using namespace std;

// Глобальные переменные
NOTIFYICONDATA nid = {};        //Инфа по иконке в трее
HWND hwndMain = nullptr;        //Дескриптор главного окна
HWND hwndSettings = nullptr;    //Дескриптор окна настроек

//Настройки приложения
int g_criticalThreshold = 15;   //Порог критического заряда
int g_reportInterval = 5;       //Интервал сохранения отчетов (минуты)
bool g_isMonitoring = true;     //Флаг для остановки потока

//Структура для состояния питания
struct PowerState {
    SYSTEM_POWER_STATUS status;
    bool ACOnline;
    bool Critical_Charge;
    bool Full_Charge;
    
    PowerState() : ACOnline(false), Critical_Charge(false), Full_Charge(false) {} //Выставляем значения по умолчанию
};

PowerState currentState;

//========== ФУНКЦИИ РАБОТЫ С НАСТРОЙКАМИ ==========

//Функция получения пути к файлу настроек
wstring GetConfigPath() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring configPath = path;
    configPath += L"\\PowerReports\\config.ini";
    return configPath;
}

//Функция загрузки настроек из файла
void LoadSettings() {
    wstring configPath = GetConfigPath();
    
    //Создаем папку если её нет
    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);
    
    HANDLE hFile = CreateFileW(configPath.c_str(), GENERIC_READ, FILE_SHARE_READ, 
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        //Файла нет - используем значения по умолчанию
        g_criticalThreshold = 15;
        g_reportInterval = 5;
        return;
    }
    
    //Читаем файл
    char buffer[256];
    DWORD bytesRead;
    if (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';
        string content(buffer);
        
        //Парсим значения
        size_t pos1 = content.find("CriticalThreshold=");
        if (pos1 != string::npos) {
            size_t end1 = content.find("\n", pos1);
            string value = content.substr(pos1 + 19, end1 - pos1 - 19);
            g_criticalThreshold = atoi(value.c_str());
        }
        
        size_t pos2 = content.find("ReportInterval=");
        if (pos2 != string::npos) {
            size_t end2 = content.find("\n", pos2);
            string value = content.substr(pos2 + 16, end2 - pos2 - 16);
            g_reportInterval = atoi(value.c_str());
        }
    }
    
    CloseHandle(hFile);
}

//Функция сохранения настроек в файл
void SaveSettings() {
    wstring configPath = GetConfigPath();
    
    //Создаем папку если её нет
    wstring folderPath = configPath.substr(0, configPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);
    
    //Формируем содержимое
    string content = "[PowerMonitor]\n";
    content += "CriticalThreshold=" + to_string(g_criticalThreshold) + "\n";
    content += "ReportInterval=" + to_string(g_reportInterval) + "\n";
    
    //Сохраняем в файл
    HANDLE hFile = CreateFileW(configPath.c_str(), GENERIC_WRITE, 0, 
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

//========== ФУНКЦИИ РАБОТЫ С УВЕДОМЛЕНИЯМИ ==========

//Функция показа уведомлений
void ShowNotification(const wstring& title, const wstring& message, int iconType = NIIF_INFO) {
    NOTIFYICONDATA nidNotify = nid;
    nidNotify.uFlags = NIF_INFO;            // Показываем уведомление 
    nidNotify.dwInfoFlags = iconType;       // Тип уведомления (INFO/WARNING/ERROR)
    nidNotify.uTimeout = 5000;              // 5 сек
    
    //Копируем строки
    StringCchCopyW(nidNotify.szInfoTitle, 64, title.c_str());
    StringCchCopyW(nidNotify.szInfo, 256, message.c_str());
    
    //Обновляем иконку
    Shell_NotifyIcon(NIM_MODIFY, &nidNotify);
}

//========== ФУНКЦИИ РАБОТЫ С ОТЧЕТАМИ ==========

//Функция получения текущего времени в строку
string GetCurrentTimeString() {
    time_t now = time(nullptr);
    struct tm tstruct;
    char buf[80];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    return string(buf);
}

//Функция получения активной схемы питания
string GetActivePowerScheme() {
    GUID* activeGuid = nullptr;  //Указатель на GUID, который заполнит функция
    
    if (PowerGetActiveScheme(NULL, &activeGuid) == ERROR_SUCCESS) {
        char str[40];
        snprintf(str, sizeof(str), 
                 "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 activeGuid->Data1, activeGuid->Data2, activeGuid->Data3,
                 activeGuid->Data4[0], activeGuid->Data4[1], 
                 activeGuid->Data4[2], activeGuid->Data4[3],
                 activeGuid->Data4[4], activeGuid->Data4[5], 
                 activeGuid->Data4[6], activeGuid->Data4[7]);
        
        //Освобождаем память, выделенную функцией
        LocalFree(activeGuid);
        return string(str);
    }
    return "Неизвестно";
}

//Функция получения температуры батареи (упрощенная)
bool GetBatteryTemperature(int& temperature) {
    //В реальном приложении здесь должен быть код через WMI
    //Для демонстрации возвращаем примерное значение
    temperature = 25;
    return true;
}

//Функция сохранения события в лог
void SaveEventToLog(const wstring& event) {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring logPath = path;
    logPath += L"\\PowerReports\\events.log";
    
    //Создаем папку если её нет
    wstring folderPath = logPath.substr(0, logPath.find_last_of(L'\\'));
    CreateDirectoryW(folderPath.c_str(), NULL);
    
    HANDLE hFile = CreateFileW(logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 
                               NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        
        string timeStr = GetCurrentTimeString();
        string content = timeStr + " - ";
        
        //Конвертируем wstring в string
        char buffer[512];
        wcstombs_s(NULL, buffer, event.c_str(), 512);
        content += buffer;
        content += "\n";
        
        DWORD bytesWritten;
        WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL);
        CloseHandle(hFile);
    }
}

//Функция сохранения отчета
void SaveReport() {
    //Получаем путь к папке отчетов
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
    wstring reportsPath = path;
    reportsPath += L"\\PowerReports\\";
    
    //Создаем папку если её нет
    CreateDirectoryW(reportsPath.c_str(), NULL);
    
    //Формируем имя файла с датой
    time_t now = time(nullptr);
    struct tm tstruct;
    char timeBuf[80];
    localtime_s(&tstruct, &now);
    strftime(timeBuf, sizeof(timeBuf), "%Y%m%d_%H%M%S", &tstruct);
    
    wstring filename = reportsPath;
    filename += L"power_report_";
    
    //Конвертируем char* в wchar_t*
    wchar_t wtimeBuf[80];
    mbstowcs_s(NULL, wtimeBuf, timeBuf, 80);
    filename += wtimeBuf;
    filename += L".txt";
    
    //Получаем статус питания
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) {
        return;
    }
    
    //Открываем файл для записи
    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, 0, 
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    
    //Формируем содержимое отчета
    string content = "=== ОТЧЕТ О СОСТОЯНИИ ПИТАНИЯ ===\n";
    content += "Время: " + GetCurrentTimeString() + "\n";
    content += "----------------------------------------\n";
    
    //Уровень заряда
    if (status.BatteryLifePercent != 255) {
        content += "Уровень заряда: " + to_string((int)status.BatteryLifePercent) + "%\n";
    } else {
        content += "Уровень заряда: Неизвестно\n";
    }
    
    //Источник питания
    switch(status.ACLineStatus) {
        case 0: content += "Источник питания: Батарея\n"; break;
        case 1: content += "Источник питания: Сеть\n"; break;
        default: content += "Источник питания: Неизвестно\n";
    }
    
    //Состояние зарядки
    if (status.BatteryFlag & 8) {
        content += "Состояние: Заряжается\n";
    } else {
        content += "Состояние: Не заряжается\n";
    }
    
    //Оставшееся время
    if (status.BatteryLifeTime != -1 && status.BatteryLifeTime != 0) {
        int hours = status.BatteryLifeTime / 3600;
        int minutes = (status.BatteryLifeTime % 3600) / 60;
        content += "Оставшееся время: " + to_string(hours) + "ч " + to_string(minutes) + "мин\n";
    }
    
    //Активная схема питания
    content += "Активная схема питания: " + GetActivePowerScheme() + "\n";
    
    //Температура батареи
    int temperature = 0;
    if (GetBatteryTemperature(temperature)) {
        content += "Температура батареи: " + to_string(temperature) + "°C\n";
    }
    
    content += "----------------------------------------\n";
    content += "Порог критического заряда: " + to_string(g_criticalThreshold) + "%\n";
    content += "Интервал отчетов: " + to_string(g_reportInterval) + " мин\n";
    
    //Записываем в файл
    DWORD bytesWritten;
    WriteFile(hFile, content.c_str(), content.length(), &bytesWritten, NULL);
    CloseHandle(hFile);
    
    //Показываем уведомление
    wchar_t msg[256];
    wsprintfW(msg, L"Отчет сохранен в папке PowerReports");
    ShowNotification(L"Отчет сохранен", msg, NIIF_INFO);
}

//Функция потока для автоматического сохранения отчетов
void ReportThread() {
    int counter = 0;
    
    while (g_isMonitoring) {
        //Спим 1 минуту
        this_thread::sleep_for(chrono::minutes(1));
        counter++;
        
        //Если прошло g_reportInterval минут - сохраняем отчет
        if (counter >= g_reportInterval) {
            SaveReport();
            counter = 0;
        }
    }
}

//========== ФУНКЦИИ МОНИТОРИНГА ПИТАНИЯ ==========

//Функция проверки питания
void CheckPowerStatus (HWND hwnd) {
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status)) {
        return; //Если статус не получили то выходим
    }

    bool Now_ACOnline = (status.ACLineStatus == 1);
    int batteryPercent = status.BatteryLifePercent;
    bool Charging = (status.BatteryFlag & 8) != 0;

    //1. Проверяем подключение ЗУ
    if (Now_ACOnline != currentState.ACOnline){
        if (Now_ACOnline){
            ShowNotification(L"Питание", L"Зарядка подключена");
            SaveEventToLog(L"Зарядка подключена");
        } else {
            ShowNotification(L"Питание", L"Зарядка отключена");
            SaveEventToLog(L"Зарядка отключена");
        }
        currentState.ACOnline = Now_ACOnline;
    }

    //2. Проверяем критический заряд (используем настройку g_criticalThreshold)
    bool Now_Critical_Charge = (batteryPercent <= g_criticalThreshold && batteryPercent != 255);

    if (Now_Critical_Charge && !currentState.Critical_Charge){
        wchar_t msg [256];
        wsprintfW(msg, L"Критический заряд!! %d%%", batteryPercent);
        ShowNotification(L"Критический заряд!", msg, NIIF_WARNING);
        SaveEventToLog(wstring(L"Критический заряд: ") + to_wstring(batteryPercent) + L"%");
        currentState.Critical_Charge = true;
    } else if (!Now_Critical_Charge){
        currentState.Critical_Charge = false;   //Если заряд стал выше порога
    }

    //3. Проверяем полную зарядку (100%)
    bool Now_Full_Charge = (batteryPercent >= 100 && batteryPercent != 255);
    
    if (Now_Full_Charge && !currentState.Full_Charge && Charging) {
        ShowNotification(L"Зарядились йоу!!", L"Батарея полностью заряжена", NIIF_INFO);
        SaveEventToLog(L"Батарея полностью заряжена");
        currentState.Full_Charge = true;
    } else if (!Now_Full_Charge) {
        currentState.Full_Charge = false;
    }

    currentState.status = status;
}

//Функция показа статуса питания
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
        wsprintfW(msg, L"Уровень заряда: %d%%\nСеть: %s\nСостояние: %s\nПорог: %d%%", 
                  percent, acStatus.c_str(), chargeStatus.c_str(), g_criticalThreshold);
    } else {
        wsprintfW(msg, L"Уровень заряда: неизвестен\nСеть: %s\nСостояние: %s\nПорог: %d%%", 
                  acStatus.c_str(), chargeStatus.c_str(), g_criticalThreshold);
    }
    
    ShowNotification(L"Статус питания", msg, NIIF_INFO);
}

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
                    SaveSettings();
                    
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

//========== ГЛАВНОЕ ОКНО ==========

//Оконная процедура
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        //Создание окна - добавляем иконку в трей
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
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_STATUS, L"Показать статус");
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_REPORT, L"Сохранить отчет");
                AppendMenuW(hMenu, MF_STRING, ID_TRAY_SETTINGS, L"Настройки");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
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

        //Обработка команд из меню
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case ID_TRAY_EXIT:
                    //Выход из программы
                    g_isMonitoring = false;  //Останавливаем поток
                    Shell_NotifyIcon(NIM_DELETE, &nid);
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
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

//========== ТОЧКА ВХОДА ==========

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //Регистрируем класс главного окна
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"PowerMonitorClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExW(&wc);

    //Регистрируем класс окна настроек
    WNDCLASSEXW settingsClass = {};
    settingsClass.cbSize = sizeof(WNDCLASSEXW);
    settingsClass.lpfnWndProc = SettingsWndProc;
    settingsClass.hInstance = hInstance;
    settingsClass.lpszClassName = L"SettingsClass";
    settingsClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    settingsClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassExW(&settingsClass);

    //Загружаем настройки из файла
    LoadSettings();

    //Создаем главное окно
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

    //Скрываем окно (работаем из трея)
    ShowWindow(hwndMain, SW_HIDE);
    
    //Запускаем поток для автоматического сохранения отчетов
    thread reportThread(ReportThread);
    reportThread.detach();  //Отделяем поток, чтобы он работал независимо
    
    //Цикл обработки сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}