#include "SettingsDialog.h"
#include "BatteryIndicator.h"
#include "Settings.h"
#include "Globals.h"

#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTimer>
#include <QFrame>

// ─────────────────────────────────────────────────────────
//  QSS-тема окна (без изменений)
// ─────────────────────────────────────────────────────────
static const char* kStyleSheet = R"(
QDialog {
    background-color: #1e1f22;
    color: #e6e6e6;
}

QLabel#titleLabel {
    color: #ffffff;
    font-size: 16px;
    font-weight: bold;
    padding: 2px 0;
}

QLabel#statusLabel {
    color: #9aa0a6;
    font-size: 12px;
}

QFrame#card {
    background-color: #2b2d31;
    border: 1px solid #3a3d42;
    border-radius: 12px;
}

QFrame#separator {
    background-color: #3a3d42;
    max-height: 1px;
    border: none;
}

QLabel {
    color: #e6e6e6;
    font-size: 13px;
}

QSpinBox {
    background-color: #2b2d31;
    color: #ffffff;
    border: 1px solid #3a3d42;
    border-radius: 6px;
    padding: 3px 8px;
    min-height: 22px;
    min-width: 80px;
    selection-background-color: #4c8bf5;
    selection-color: #ffffff;
}

QSpinBox:focus {
    border: 1px solid #4c8bf5;
}

QSpinBox::up-button,
QSpinBox::down-button {
    width: 0;
    height: 0;
    border: none;
    background: none;
}

QPushButton {
    background-color: #3a3d42;
    color: #ffffff;
    border: 1px solid #4a4d52;
    border-radius: 6px;
    padding: 5px 14px;
    min-width: 70px;
    min-height: 24px;
}

QPushButton:hover {
    background-color: #4a4d52;
}

QPushButton:pressed {
    background-color: #2b2d31;
}

QPushButton#okButton {
    background-color: #4c8bf5;
    border: 1px solid #4c8bf5;
    color: #ffffff;
    font-weight: bold;
}

QPushButton#okButton:hover {
    background-color: #5c9bff;
}

QPushButton#okButton:pressed {
    background-color: #3a7be0;
}
)";

// ─────────────────────────────────────────────────────────
//  Конструктор
// ─────────────────────────────────────────────────────────
SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , m_titleLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_battery(nullptr)
    , m_thresholdSpin(nullptr)
    , m_intervalSpin(nullptr)
    , m_batteryTimer(nullptr)
{
    setWindowTitle(QStringLiteral("Настройки"));
    setFixedSize(340, 340);

    setStyleSheet(QString::fromUtf8(kStyleSheet));

    // ── Заголовок ──
    m_titleLabel = new QLabel(QStringLiteral("Настройки монитора питания"), this);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));

    m_statusLabel = new QLabel(QStringLiteral("Обновление статуса каждую секунду…"), this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));

    // ── Карточка с индикатором ──
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("card"));
    card->setFrameShape(QFrame::NoFrame);

    m_battery = new BatteryIndicator(card);
    m_battery->setCriticalThreshold(g_criticalThreshold);

    m_battery->setColors(
        QColor( 76, 175,  80),
        QColor(255, 193,   7),
        QColor(229,  57,  53),
        QColor(120, 120, 120)
    );

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(5, 16, 16, 16);
    cardLayout->addWidget(m_battery); 

    // ── Поля ввода ──
    m_thresholdSpin = new QSpinBox(this);
    m_thresholdSpin->setRange(5, 50);
    m_thresholdSpin->setSuffix(QStringLiteral(" %"));
    m_thresholdSpin->setValue(g_criticalThreshold);
    m_thresholdSpin->setFixedWidth(80);

    m_intervalSpin = new QSpinBox(this);
    m_intervalSpin->setRange(1, 60);
    m_intervalSpin->setSuffix(QStringLiteral(" мин"));
    m_intervalSpin->setValue(g_reportInterval);
    m_intervalSpin->setFixedWidth(80);

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);
    form->setContentsMargins(0, 0, 0, 0);   // ← без внутренних отступов
    form->addRow(QStringLiteral("Порог заряда:"),   m_thresholdSpin);
    form->addRow(QStringLiteral("Интервал отчётов:"), m_intervalSpin);

    // ── Разделитель ──
    auto* separator = new QFrame(this);
    separator->setObjectName(QStringLiteral("separator"));
    separator->setFrameShape(QFrame::HLine);

    // ── Кнопки (без иконок) ──
    auto* okButton     = new QPushButton(QStringLiteral("OK"),     this);
    auto* cancelButton = new QPushButton(QStringLiteral("Отмена"), this);
    okButton->setObjectName(QStringLiteral("okButton"));

    auto* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(okButton);
    buttons->addWidget(cancelButton);

    // ── Общая раскладка ──
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 16, 16, 16);   // ← слева 10
    mainLayout->setSpacing(10);
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(card);
    mainLayout->addLayout(form);
    mainLayout->addStretch();
    mainLayout->addWidget(separator);
    mainLayout->addLayout(buttons);

    // ── Сигналы ──
    connect(okButton,     &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_thresholdSpin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            m_battery,
            &BatteryIndicator::setCriticalThreshold);

    // ── Таймер обновления ──
    m_batteryTimer = new QTimer(this);
    m_batteryTimer->setInterval(1000);
    connect(m_batteryTimer, &QTimer::timeout, this, &SettingsDialog::onUpdateBattery);
    m_batteryTimer->start();

    onUpdateBattery();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::onUpdateBattery() {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) {
        m_battery->setLevel(-1);
        m_statusLabel->setText(QStringLiteral("Статус питания недоступен"));
        return;
    }

    int percent = status.BatteryLifePercent;

    if (percent == 255) {
        m_battery->setLevel(-1);
        m_statusLabel->setText(QStringLiteral("Устройство без батареи"));
        return;
    }

    m_battery->setLevel(percent);

    QString state;
    if (status.BatteryFlag & 8) {
        state = QStringLiteral("заряжается");
    } else if (status.ACLineStatus == 1) {
        state = QStringLiteral("подключено к сети");
    } else {
        state = QStringLiteral("от батареи");
    }

    m_statusLabel->setText(
        QStringLiteral("Текущий заряд: %1%  •  %2").arg(percent).arg(state));
}

void SettingsDialog::onOkClicked() {
    g_criticalThreshold = m_thresholdSpin->value();
    g_reportInterval    = m_intervalSpin->value();

    SaveSettings();
    accept();
}