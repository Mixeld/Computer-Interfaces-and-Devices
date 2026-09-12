#ifndef TRAYAPP_H
#define TRAYAPP_H

#include <QObject>
#include <QSystemTrayIcon>

class QMenu;
class QTimer;

// Главный объект приложения: трей, меню, таймеры.
class TrayApp : public QObject {
    Q_OBJECT

public:
    explicit TrayApp(QObject* parent = nullptr);
    ~TrayApp() override;

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onCheckPower();
    void onSaveReport();
    void onOpenSettings();
    void onShowStatus();
    void onExit();

private:
    QSystemTrayIcon* m_tray;
    QMenu*           m_menu;
    QTimer*          m_powerTimer;      // каждые 2 секунды
    QTimer*          m_reportTimer;     // каждые N минут
};

#endif // TRAYAPP_H