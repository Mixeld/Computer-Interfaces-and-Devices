#ifndef GLOBALS_H
#define GLOBALS_H

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0600
#define NTDDI_VERSION 0x06000000

#include <windows.h>
#include <shellapi.h>
#include <string>

#define WM_TRAY_NOTIFY (WM_USER + 1)
#define ID_TRAY_EXIT     1001
#define ID_TRAY_STATUS   1002
#define ID_TRAY_SETTINGS 1003
#define ID_TRAY_REPORT   1004

// Структура для состояния питания
struct PowerState {
    SYSTEM_POWER_STATUS status;
    bool ACOnline;
    bool Critical_Charge;
    bool Full_Charge;

    PowerState() : status{}, ACOnline(false), Critical_Charge(false), Full_Charge(false) {}
};

// Глобальные переменные (объявления)
extern NOTIFYICONDATAW nid;
extern HWND hwndMain;
// hwndSettings больше не нужен — окно настроек теперь Qt

// Настройки приложения
extern int  g_criticalThreshold;
extern int  g_reportInterval;
extern bool g_isMonitoring;

extern PowerState currentState;

#endif // GLOBALS_H