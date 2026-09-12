#ifndef POWERMONITOR_H
#define POWERMONITOR_H

#include "Globals.h"
#include <QString>

// Проверка состояния питания (вызывается из QTimer).
void CheckPowerStatus();

// Показать статус питания в трее.
void ShowPowerStatusNotification();

#endif // POWERMONITOR_H