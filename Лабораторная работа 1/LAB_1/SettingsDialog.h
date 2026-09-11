#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QPushButton;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;

private slots:
    void onOkClicked();
    void onCancelClicked();

private:
    QSpinBox*    m_thresholdSpin;
    QSpinBox*    m_intervalSpin;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
};

// Запускает Qt-окно настроек в отдельном потоке.
void OpenQtSettingsWindow();

#endif // SETTINGSDIALOG_H