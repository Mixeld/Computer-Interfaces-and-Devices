#include "Notifications.h"
#include <QSystemTrayIcon>

static QSystemTrayIcon* g_tray = nullptr;

void SetTrayIcon(QSystemTrayIcon* tray) {
    g_tray = tray;
}

void ShowNotification(const QString& title, const QString& message, int iconType) {
    if (!g_tray) return;

    QSystemTrayIcon::MessageIcon icon = static_cast<QSystemTrayIcon::MessageIcon>(iconType);

    g_tray->showMessage(title, message, icon, 5000);
}