#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QString>

class QSystemTrayIcon;

// Глобальный указатель на трей — устанавливается один раз в TrayApp.
// Функции уведомлений используют его для показа сообщений.
void SetTrayIcon(QSystemTrayIcon* tray);

// Показать уведомление через трей.
// iconType: QSystemTrayIcon::Information / Warning / Critical
void ShowNotification(const QString& title,
                      const QString& message,
                      int iconType = 1 /* Information */);

#endif // NOTIFICATIONS_H