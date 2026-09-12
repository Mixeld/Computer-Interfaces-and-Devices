#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QTimer;
class QLabel;
class BatteryIndicator;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

private slots:
    void onOkClicked();
    void onUpdateBattery();

private:
    QLabel*           m_titleLabel;
    QLabel*           m_statusLabel;
    BatteryIndicator* m_battery;
    QSpinBox*         m_thresholdSpin;
    QSpinBox*         m_intervalSpin;
    QTimer*           m_batteryTimer;
};

#endif // SETTINGSDIALOG_H