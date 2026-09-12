#ifndef BATTERYINDICATOR_H
#define BATTERYINDICATOR_H

#include <QWidget>
#include <QColor>

class BatteryIndicator : public QWidget {
    Q_OBJECT

public:
    explicit BatteryIndicator(QWidget* parent = nullptr);

    void setLevel(int percent);
    void setCriticalThreshold(int threshold);

    // Цвета можно переопределить извне
    void setColors(const QColor& ok,
                   const QColor& low,
                   const QColor& critical,
                   const QColor& unknown);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_level;
    int m_criticalThreshold;

    QColor m_colorOk;
    QColor m_colorLow;
    QColor m_colorCritical;
    QColor m_colorUnknown;
};

#endif // BATTERYINDICATOR_H