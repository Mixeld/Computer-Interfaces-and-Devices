#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <string>

// Настройки приложения
extern int  g_criticalThreshold;
extern int  g_reportInterval;
extern volatile bool g_isMonitoring;

// Структура состояния питания
struct PowerState {
    SYSTEM_POWER_STATUS status;
    bool ACOnline;
    bool Critical_Charge;
    bool Full_Charge;

    PowerState() : status{}, ACOnline(false), Critical_Charge(false), Full_Charge(false) {}
};

extern PowerState currentState;

#endif // GLOBALS_H