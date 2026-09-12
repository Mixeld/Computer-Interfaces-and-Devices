#include "BatteryIndicator.h"

#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>

BatteryIndicator::BatteryIndicator(QWidget* parent)
    : QWidget(parent)
    , m_level(-1)
    , m_criticalThreshold(15)
    , m_colorOk      (QColor( 76, 175,  80))
    , m_colorLow     (QColor(255, 193,   7))
    , m_colorCritical(QColor(229,  57,  53))
    , m_colorUnknown (QColor(158, 158, 158))
{
    setMinimumSize(160, 80);
}

void BatteryIndicator::setLevel(int percent) {
    m_level = (percent < 0 || percent > 100) ? -1 : percent;
    update();
}

void BatteryIndicator::setCriticalThreshold(int threshold) {
    m_criticalThreshold = threshold;
    update();
}

void BatteryIndicator::setColors(const QColor& ok,
                                 const QColor& low,
                                 const QColor& critical,
                                 const QColor& unknown) {
    m_colorOk       = ok;
    m_colorLow      = low;
    m_colorCritical = critical;
    m_colorUnknown  = unknown;
    update();
}

QSize BatteryIndicator::sizeHint() const {
    return QSize(220, 90);
}

void BatteryIndicator::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int margin     = 8;
    const int tailWidth  = 12;
    const int tailHeight = 28;

    QRect bodyRect(
        margin,
        margin,
        width()  - margin * 2 - tailWidth - 6,
        height() - margin * 2
    );
    if (bodyRect.width() <= 0 || bodyRect.height() <= 0) return;

    QRect tailRect(
        bodyRect.right() + 6,
        bodyRect.center().y() - tailHeight / 2,
        tailWidth,
        tailHeight
    );

    // ── Цвет заливки ──
    QColor fillColor;
    if (m_level < 0) {
        fillColor = m_colorUnknown;
    } else if (m_level <= m_criticalThreshold) {
        fillColor = m_colorCritical;
    } else if (m_level <= 40) {
        fillColor = m_colorLow;
    } else {
        fillColor = m_colorOk;
    }

    // ── Тень под корпусом ──
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawRoundedRect(bodyRect.translated(0, 2), 10, 10);

    // ── Корпус ──
    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.setBrush(QColor(245, 245, 245));
    p.drawRoundedRect(bodyRect, 10, 10);

    // ── Хвостик ──
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(80, 80, 80));
    p.drawRoundedRect(tailRect, 4, 4);

    // ── Заливка уровня ──
    if (m_level >= 0) {
        QRect fillRect = bodyRect.adjusted(4, 4, -4, -4);
        int fillWidth = fillRect.width() * m_level / 100;
        fillRect.setWidth(fillWidth);

        if (fillWidth > 0) {
            QLinearGradient grad(fillRect.topLeft(), fillRect.bottomLeft());
            grad.setColorAt(0.0, fillColor.lighter(130));
            grad.setColorAt(0.5, fillColor);
            grad.setColorAt(1.0, fillColor.darker(115));

            p.setBrush(grad);
            p.setPen(Qt::NoPen);

            int radius = qMin(6, fillRect.height() / 2);
            p.drawRoundedRect(fillRect, radius, radius);
        }
    }
}