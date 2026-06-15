//
// Created by BenzoicAcid on 2026/5/14.
//

#include "presentation/pet/RadialMenu.h"
#include "presentation/common/Theme.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

static constexpr double DEG2RAD = M_PI / 180.0;
static constexpr double RAD2DEG = 180.0 / M_PI;

RadialMenu::RadialMenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(WidgetSize, WidgetSize);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void RadialMenu::popup(const QPoint &globalPos, const QVector<Item> &items) {
    m_items   = items;
    m_hovered = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].visible && m_items[i].enabled) {
            m_hovered = i;
            break;
        }
    }

    move(globalPos - QPoint(WidgetSize / 2, WidgetSize / 2));
    show();
    setFocus(Qt::PopupFocusReason);
}

// 返回第 index 个扇形的起始角（弧度，从 -90° 开始顺时针）
double RadialMenu::sectorStartAngle(int index) const {
    const int n       = m_items.size();
    const double step = 360.0 / n;
    return (-90.0 + index * step + GapDeg / 2.0) * DEG2RAD;
}

int RadialMenu::itemAt(QPoint pos) const {
    if (m_items.isEmpty()) return -1;

    const double cx = WidgetSize / 2.0;
    const double cy = WidgetSize / 2.0;
    const double dx = pos.x() - cx;
    const double dy = pos.y() - cy;
    const double r  = std::sqrt(dx * dx + dy * dy);

    if (r < InnerR || r > OuterR) return -1;

    // atan2 返回 [-pi, pi]，转换为从 -90° 起的顺时针角度
    double angleDeg = std::atan2(dy, dx) * RAD2DEG;
    angleDeg += 90.0;
    if (angleDeg < 0) angleDeg += 360.0;

    const int n        = m_items.size();
    const double step  = 360.0 / n;
    const double sweep = step - GapDeg;

    for (int i = 0; i < n; ++i) {
        double start = i * step;
        double end   = start + sweep;
        if (angleDeg >= start && angleDeg < end) {
            if (!m_items[i].visible || !m_items[i].enabled) return -1;
            return i;
        }
    }
    return -1;
}

bool RadialMenu::usesPartialLayout() const {
    for (const auto &item : m_items) {
        if (!item.visible) return true;
    }
    return false;
}

void RadialMenu::drawSector(QPainter &p, int index) const {
    if (!m_items[index].visible) return;

    const int    n      = m_items.size();
    const double step   = 360.0 / n;
    const double sweep  = step - GapDeg;
    const double cx     = WidgetSize / 2.0;
    const double cy     = WidgetSize / 2.0;
    const bool   hov    = (index == m_hovered);

    // Qt 的 arcTo 角度正方向为逆时针，0° 在 3 点钟。
    const double qtStart = -((-90.0 + index * step + GapDeg / 2.0));
    const double qtSweep = -sweep;

    QPainterPath path;
    const QRectF innerRect(cx - InnerR, cy - InnerR, InnerR * 2, InnerR * 2);
    const QRectF outerRect(cx - OuterR, cy - OuterR, OuterR * 2, OuterR * 2);

    const double startRad = sectorStartAngle(index);
    path.moveTo(cx + InnerR * std::cos(startRad), cy + InnerR * std::sin(startRad));
    path.arcTo(outerRect, qtStart, qtSweep);
    path.arcTo(innerRect, qtStart + qtSweep, -qtSweep);
    path.closeSubpath();

    if (hov) {
        p.setBrush(QColor(Theme::Primary));
        p.setPen(Qt::NoPen);
    } else {
        QColor sectorBg(Theme::BgPrimary);
        sectorBg.setAlpha(220);
        p.setBrush(sectorBg);
        p.setPen(Qt::NoPen);
    }
    p.drawPath(path);

    if (usesPartialLayout()) {
        QColor border(Theme::Border);
        border.setAlpha(210);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(border, 1.0));
        p.drawPath(path);
    }

    const double midAngleRad = startRad + (sweep / 2.0) * DEG2RAD;
    const double midR        = (InnerR + OuterR) / 2.0;
    const double lx          = cx + midR * std::cos(midAngleRad);
    const double ly          = cy + midR * std::sin(midAngleRad);
    const QColor textColor   = hov ? QColor(Theme::BgPrimary) : QColor(Theme::TextSecondary);

    if (!m_items[index].icon.isEmpty()) {
        p.setPen(textColor);
        QFont iconFont("Segoe UI Symbol", 15);
        p.setFont(iconFont);
        const QRectF iconRect(lx - 18, ly - 22, 36, 22);
        p.drawText(iconRect, Qt::AlignCenter, m_items[index].icon);

        p.setPen(textColor);
        QFont labelFont("Segoe UI", 8);
        p.setFont(labelFont);
        const QRectF labelRect(lx - 22, ly + 8, 44, 16);
        p.drawText(labelRect, Qt::AlignCenter, m_items[index].label);
    } else {
        p.setPen(textColor);
        QFont labelFont("Segoe UI", 9);
        p.setFont(labelFont);
        const QRectF labelRect(lx - 24, ly - 10, 48, 20);
        p.drawText(labelRect, Qt::AlignCenter, m_items[index].label);
    }
}

void RadialMenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    for (int i = 0; i < m_items.size(); ++i)
        drawSector(p, i);

    const double cx = WidgetSize / 2.0;
    const double cy = WidgetSize / 2.0;

    if (!m_items.isEmpty() && !usesPartialLayout()) {
        const double step = 360.0 / m_items.size();
        QColor separator(0, 0, 0, 125);
        QPen separatorPen(separator, 1.2);
        separatorPen.setCapStyle(Qt::RoundCap);
        p.setPen(separatorPen);
        for (int i = 0; i < m_items.size(); ++i) {
            const double a = (-90.0 + i * step) * DEG2RAD;
            const QPointF inner(cx + (InnerR + 14.0) * std::cos(a), cy + (InnerR + 14.0) * std::sin(a));
            const QPointF outer(cx + (OuterR - 14.0) * std::cos(a), cy + (OuterR - 14.0) * std::sin(a));
            p.drawLine(inner, outer);
        }
    }

    if (!usesPartialLayout()) {
        QColor outerBorder(Theme::Border);
        outerBorder.setAlpha(210);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(outerBorder, 1.0));
        p.drawEllipse(QPointF(cx, cy), InnerR, InnerR);
        p.drawEllipse(QPointF(cx, cy), OuterR, OuterR);
    }
}

void RadialMenu::mouseMoveEvent(QMouseEvent *event) {
    const int prev = m_hovered;
    m_hovered = itemAt(event->position().toPoint());
    if (m_hovered != prev) update();
}

void RadialMenu::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space)
    {
        const int idx = m_hovered;
        hide();
        if (idx >= 0)
            emit triggered(idx);
        return;
    }

    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Right || event->key() == Qt::Key_Down)
    {
        if (m_items.isEmpty())
            return;

        const int direction = (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up) ? -1 : 1;
        int index = m_hovered >= 0 ? m_hovered : 0;
        for (int step = 0; step < m_items.size(); ++step) {
            index = (index + direction + m_items.size()) % m_items.size();
            if (m_items[index].visible && m_items[index].enabled) {
                m_hovered = index;
                update();
                return;
            }
        }
    }

    QWidget::keyPressEvent(event);
}

void RadialMenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        hide();
        return;
    }

    const int idx = itemAt(event->position().toPoint());
    hide();
    if (idx >= 0) emit triggered(idx);
}

void RadialMenu::leaveEvent(QEvent *) {
    m_hovered = -1;
    update();
}
