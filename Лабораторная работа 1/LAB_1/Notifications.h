#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include "Globals.h"
#include <string>

//Функция показа уведомлений
void ShowNotification(const std::wstring& title, const std::wstring& message, int iconType = NIIF_INFO);

#endif // NOTIFICATIONS_H