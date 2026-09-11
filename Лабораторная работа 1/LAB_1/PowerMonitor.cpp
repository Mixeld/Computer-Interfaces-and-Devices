#include "PowerMonitor.h"
#include "Notifications.h"
#include "Reports.h"
#include <string>

using namespace std;

//========== ФУНКЦИИ МОНИТОРИНГА ПИТАНИЯ ==========

void CheckPowerStatus(HWND hwnd) {
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status)) {
        return;
    }

    bool Now_ACOnline  = (status.ACLineStatus == 1);
    int  batteryPercent = status.BatteryLifePercent;
    bool Charging      = (status.BatteryFlag & 8) != 0;

    // 1. Проверяем подключение ЗУ
    if (Now_ACOnline != currentState.ACOnline) {
        if (Now_ACOnline) {
            ShowNotification(L"Питание", L"Зарядка подключена");
            SaveEventToLog(L"Зарядка подключена");
        } else {
            ShowNotification(L"Питание", L"Зарядка отключена");
            SaveEventToLog(L"Зарядка отключена");
        }
        currentState.ACOnline = Now_ACOnline;
    }

    // 2. Критический заряд
    bool Now_Critical_Charge = (batteryPercent != 255 && batteryPercent <= g_criticalThreshold);

    if (Now_Critical_Charge && !currentState.Critical_Charge) {
        wchar_t msg[256];
        wsprintfW(msg, L"Критический заряд!! %d%%", batteryPercent);
        ShowNotification(L"Критический заряд!", msg, NIIF_WARNING);
        SaveEventToLog(wstring(L"Критический заряд: ") + to_wstring(batteryPercent) + L"%");
        currentState.Critical_Charge = true;
    } else if (!Now_Critical_Charge) {
        currentState.Critical_Charge = false;
    }

    // 3. Полная зарядка
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

void ShowPowerStatusNotification() {
    SYSTEM_POWER_STATUS status;

    if (!GetSystemPowerStatus(&status)) {
        ShowNotification(L"Ошибка", L"Не удалось получить статус питания", NIIF_ERROR);
        return;
    }

    wchar_t msg[512];
    int percent = status.BatteryLifePercent;

    wstring acStatus;
    switch (status.ACLineStatus) {
        case 0:  acStatus = L"Отключено (батарея)"; break;
        case 1:  acStatus = L"Подключено (сеть)";   break;
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