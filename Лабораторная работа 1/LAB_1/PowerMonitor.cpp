#include "PowerMonitor.h"
#include "Notifications.h"
#include "Reports.h"
#include <QSystemTrayIcon>
#include <string>

using namespace std;

void CheckPowerStatus() {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) return;

    bool nowAC       = (status.ACLineStatus == 1);
    int  percent     = status.BatteryLifePercent;
    bool charging    = (status.BatteryFlag & 8) != 0;

    // 1. Подключение/отключение ЗУ
    if (nowAC != currentState.ACOnline) {
        if (nowAC) {
            ShowNotification(QStringLiteral("Питание"),
                             QStringLiteral("Зарядка подключена"),
                             QSystemTrayIcon::Information);
            SaveEventToLog(L"Зарядка подключена");
        } else {
            ShowNotification(QStringLiteral("Питание"),
                             QStringLiteral("Зарядка отключена"),
                             QSystemTrayIcon::Information);
            SaveEventToLog(L"Зарядка отключена");
        }
        currentState.ACOnline = nowAC;
    }

    // 2. Критический заряд
    bool nowCritical = (percent != 255 && percent <= g_criticalThreshold);

    if (nowCritical && !currentState.Critical_Charge) {
        QString msg = QStringLiteral("Критический заряд!! %1%").arg(percent);
        ShowNotification(QStringLiteral("Критический заряд!"), msg,
                         QSystemTrayIcon::Warning);
        SaveEventToLog(wstring(L"Критический заряд: ") + to_wstring(percent) + L"%");
        currentState.Critical_Charge = true;
    } else if (!nowCritical) {
        currentState.Critical_Charge = false;
    }

    // 3. Полная зарядка
    bool nowFull = (percent >= 100 && percent != 255);

    if (nowFull && !currentState.Full_Charge && charging) {
        ShowNotification(QStringLiteral("Батарея заряжена"),
                         QStringLiteral("Батарея полностью заряжена"),
                         QSystemTrayIcon::Information);
        SaveEventToLog(L"Батарея полностью заряжена");
        currentState.Full_Charge = true;
    } else if (!nowFull) {
        currentState.Full_Charge = false;
    }

    currentState.status = status;
}

void ShowPowerStatusNotification() {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) {
        ShowNotification(QStringLiteral("Ошибка"),
                         QStringLiteral("Не удалось получить статус питания"),
                         QSystemTrayIcon::Critical);
        return;
    }

    int percent = status.BatteryLifePercent;

    QString acStatus;
    switch (status.ACLineStatus) {
        case 0:  acStatus = QStringLiteral("Отключено (батарея)"); break;
        case 1:  acStatus = QStringLiteral("Подключено (сеть)");   break;
        default: acStatus = QStringLiteral("Неизвестно");
    }

    QString chargeStatus = (status.BatteryFlag & 8)
                           ? QStringLiteral("Заряжается")
                           : QStringLiteral("Не заряжается");

    QString percentStr = (percent != 255)
                         ? QStringLiteral("%1%").arg(percent)
                         : QStringLiteral("неизвестен");

    QString msg = QStringLiteral("Уровень заряда: %1\nСеть: %2\nСостояние: %3\nПорог: %4%")
                      .arg(percentStr, acStatus, chargeStatus)
                      .arg(g_criticalThreshold);

    ShowNotification(QStringLiteral("Статус питания"), msg,
                     QSystemTrayIcon::Information);
}