#include "SettingsDialog.h"
#include "Settings.h"
#include "Globals.h"

#include <QApplication>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QThread>

// ─────────────────────────────────────────────────────────
//  Конструктор / деструктор
// ─────────────────────────────────────────────────────────
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_thresholdSpin(nullptr)
    , m_intervalSpin(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle(QStringLiteral("Настройки"));
    setFixedSize(340, 180);

    m_thresholdSpin = new QSpinBox(this);
    m_thresholdSpin->setRange(5, 50);
    m_thresholdSpin->setSuffix(QStringLiteral(" %"));
    m_thresholdSpin->setValue(g_criticalThreshold);

    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(1, 60);
    m_intervalSpin->setSuffix(QStringLiteral(" мин"));
    m_intervalSpin->setValue(g_reportInterval);

    m_okButton     = new QPushButton(QStringLiteral("OK"),     this);
    m_cancelButton = new QPushButton(QStringLiteral("Отмена"), this);

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Порог критического заряда:"), m_thresholdSpin);
    form->addRow(QStringLiteral("Интервал отчётов:"),          m_intervalSpin);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(m_okButton);
    buttons->addWidget(m_cancelButton);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addStretch();
    mainLayout->addLayout(buttons);

    connect(m_okButton,     &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
}

SettingsDialog::~SettingsDialog() = default;

// ─────────────────────────────────────────────────────────
//  Слоты
// ─────────────────────────────────────────────────────────
void SettingsDialog::onOkClicked() {
    g_criticalThreshold = m_thresholdSpin->value();
    g_reportInterval    = m_intervalSpin->value();

    SaveSettings();
    accept();
}

void SettingsDialog::onCancelClicked() {
    reject();
}

// ─────────────────────────────────────────────────────────
//  Запуск Qt-окна в отдельном потоке
// ─────────────────────────────────────────────────────────
namespace {

void QtSettingsThreadFunc() {
    int argc = 1;
    char arg0[] = "PRICOL";
    char* argv[] = { arg0, nullptr };

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    SettingsDialog dlg;
    dlg.exec();

    app.quit();
}

} // namespace

void OpenQtSettingsWindow() {
    static bool s_running = false;
    if (s_running) {
        return;
    }
    s_running = true;

    QThread* thread = QThread::create([]() {
        QtSettingsThreadFunc();
        s_running = false;
    });
    thread->setObjectName(QStringLiteral("QtSettingsThread"));
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}