#ifndef GLOBALS_H
#define GLOBALS_H

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0600 //Указываем версию винды
#define NTDDI_VERSION 0x06000000

#include <windows.h>  //WinAPI
#include <shellapi.h> // ОБЯЗАТЕЛЬНО ДЛЯ MinGW (NOTIFYICONDATAW и Shell_NotifyIcon)
#include <string>

#define WM_TRAY_NOTIFY (WM_USER + 1) //Если пользователь жмёт на иконку => сообщение приходит в WndProc
#define ID_TRAY_EXIT     1001
#define ID_TRAY_STATUS   1002 // ID для пункта "Показать статус"
#define ID_TRAY_SETTINGS 1003 // ID для пункта "Настройки"
#define ID_TRAY_REPORT   1004 // ID для пункта "Сохранить отчет"

//Структура для состояния питания
struct PowerState {
    SYSTEM_POWER_STATUS status;
    bool ACOnline;
    bool Critical_Charge;
    bool Full_Charge;

    PowerState() : status{}, ACOnline(false), Critical_Charge(false), Full_Charge(false) {} //Выставляем значения по умолчанию
};

// Глобальные переменные (объявления)
extern NOTIFYICONDATAW nid;         //Инфа по иконке в трее
extern HWND hwndMain;               //Дескриптор главного окна
extern HWND hwndSettings;           //Дескриптор окна настроек

//Настройки приложения
extern int  g_criticalThreshold;    //Порог критического заряда
extern int  g_reportInterval;       //Интервал сохранения отчетов (минуты)
extern bool g_isMonitoring;         //Флаг для остановки потока

extern PowerState currentState;

#endif // GLOBALS_H