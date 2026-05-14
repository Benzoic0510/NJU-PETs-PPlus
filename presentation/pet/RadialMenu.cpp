//
// Created by BenzoicAcid on 2026/5/14.
//

#include "presentation/pet/RadialMenu.h"
#include "presentation/common/Theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

RadialMenu::RadialMenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(WidgetSize, WidgetSize);
    setMouseTracking(true);
}

void RadialMenu::popup(const QPoint &globalPos, const QVector<Item> &items) {
    m_items   = items;
    m_hovered = -1;
    // 以点击位置为中心
    move(globalPos - QPoint(WidgetSize / 2, WidgetSize / 2));
    show();
}

QPointF RadialMenu::itemCenter(int index) const {
    const double step  = (m_items.size() > 1) ? 2.0 * M_PI / m_items.size() : 0.0;
    const double start = -M_PI / 2.0;  // 从 12 点钟方向开始
    const double angle = start + index * step;
    const double cx    = WidgetSize / 2.0;
    const double cy    = WidgetSize / 2.0;
    return {cx + Radius * std::cos(angle), cy + Radius * std::sin(angle)};
}

int RadialMenu::itemAt(QPoint pos) const {
    for (int i = 0; i < m_items.size(); ++i) {
        const QPointF c = itemCenter(i);
        const double dx = pos.x() - c.x();
        const double dy = pos.y() - c.y();
        if (dx * dx + dy * dy <= static_cast<double>(ItemR * ItemR))
            return i;
    }
    return -1;
}

void RadialMenu::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 各 item
    QFont font("Segoe UI", 9);
    font.setBold(false);
    p.setFont(font);

    for (int i = 0; i < m_items.size(); ++i) {
        const QPointF c = itemCenter(i);
        const bool hov  = (i == m_hovered);

        // 圆形背景
        p.setPen(QPen(QColor(Theme::Border), 1));
        p.setBrush(hov ? QColor(Theme::Primary) : QColor(255, 255, 255, 230));
        p.drawEllipse(c, static_cast<double>(ItemR), static_cast<double>(ItemR));

        // 标签
        p.setPen(hov ? Qt::white : QColor(Theme::TextSecondary));
        const QRectF rect(c.x() - ItemR, c.y() - ItemR, 2 * ItemR, 2 * ItemR);
        p.drawText(rect, Qt::AlignCenter, m_items[i].label);
    }
}

void RadialMenu::mouseMoveEvent(QMouseEvent *event) {
    const int prev = m_hovered;
    m_hovered = itemAt(event->pos());
    if (m_hovered != prev) update();
}

void RadialMenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) { hide(); return; }
    const int idx = itemAt(event->pos());
    hide();
    if (idx >= 0) emit triggered(idx);
}
