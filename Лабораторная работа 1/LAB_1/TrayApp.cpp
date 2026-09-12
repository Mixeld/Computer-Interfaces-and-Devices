#include "TrayApp.h"
#include "Globals.h"
#include "Notifications.h"
#include "PowerMonitor.h"
#include "Reports.h"
#include "SettingsDialog.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QApplication>
#include <QIcon>
#include <QStyle>          // ← добавлено: полное определение QStyle

TrayApp::TrayApp(QObject* parent)
    : QObject(parent)
    , m_tray(nullptr)
    , m_menu(nullptr)
    , m_powerTimer(nullptr)
    , m_reportTimer(nullptr)
{
    // ── Иконка и меню ──
    m_tray = new QSystemTrayIcon(this);
    m_tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    m_tray->setToolTip(QStringLiteral("Лабораторная работа 1"));

    m_menu = new QMenu();

    QAction* actStatus   = m_menu->addAction(QStringLiteral("Показать статус"));
    QAction* actReport   = m_menu->addAction(QStringLiteral("Сохранить отчет"));
    QAction* actSettings = m_menu->addAction(QStringLiteral("Настройки"));
    m_menu->addSeparator();
    QAction* actExit     = m_menu->addAction(QStringLiteral("Выход"));

    connect(actStatus,   &QAction::triggered, this, &TrayApp::onShowStatus);
    connect(actReport,   &QAction::triggered, this, &TrayApp::onSaveReport);
    connect(actSettings, &QAction::triggered, this, &TrayApp::onOpenSettings);
    connect(actExit,     &QAction::triggered, this, &TrayApp::onExit);

    m_tray->setContextMenu(m_menu);

    connect(m_tray, &QSystemTrayIcon::activated,
            this,   &TrayApp::onTrayActivated);

    SetTrayIcon(m_tray);

    m_tray->show();

    // ── Таймер проверки питания: каждые 2 сек ──
    m_powerTimer = new QTimer(this);
    m_powerTimer->setInterval(2000);
    connect(m_powerTimer, &QTimer::timeout, this, &TrayApp::onCheckPower);
    m_powerTimer->start();

    // ── Таймер отчётов ──
    m_reportTimer = new QTimer(this);
    m_reportTimer->setInterval(g_reportInterval * 60 * 1000);
    connect(m_reportTimer, &QTimer::timeout, this, &TrayApp::onSaveReport);
    m_reportTimer->start();

    ShowNotification(QStringLiteral("Лабораторная работа 1"),
                     QStringLiteral("Приложение запущено!"),
                     QSystemTrayIcon::Information);
}

TrayApp::~TrayApp() {
    delete m_menu;
}

void TrayApp::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowStatus();
    }
}

void TrayApp::onCheckPower() {
    CheckPowerStatus();
}

void TrayApp::onSaveReport() {
    SaveReport();
}

void TrayApp::onShowStatus() {
    ShowPowerStatusNotification();
}

void TrayApp::onOpenSettings() {
    SettingsDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        m_reportTimer->setInterval(g_reportInterval * 60 * 1000);
        m_reportTimer->start();
    }
}

void TrayApp::onExit() {
    g_isMonitoring = false;
    QApplication::quit();
}