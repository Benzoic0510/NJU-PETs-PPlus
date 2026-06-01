//
// Created by BenzoicAcid on 2026/5/14.
//

#include "presentation/pet/RadialMenu.h"
#include "presentation/common/Theme.h"

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
}

void RadialMenu::popup(const QPoint &globalPos, const QVector<Item> &items) {
    m_items   = items;
    m_hovered = -1;
    move(globalPos - QPoint(WidgetSize / 2, WidgetSize / 2));
    show();
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

    // atan2 返回 [-π, π]，转换为从 -90° 起的顺时针角度
    double angleDeg = std::atan2(dy, dx) * RAD2DEG;  // [-180, 180]
    angleDeg += 90.0;                                  // 转为从 12 点钟起
    if (angleDeg < 0) angleDeg += 360.0;              // [0, 360)

    const int n        = m_items.size();
    const double step  = 360.0 / n;
    const double sweep = step - GapDeg;

    for (int i = 0; i < n; ++i) {
        double start = i * step;   // 该扇形有效区起始（度，顺时针从12点）
        double end   = start + sweep;
        // 规范化后比较
        if (angleDeg >= start && angleDeg < end) return i;
    }
    return -1;
}

void RadialMenu::drawSector(QPainter &p, int index) const {
    const int    n      = m_items.size();
    const double step   = 360.0 / n;
    const double sweep  = step - GapDeg;
    const double cx     = WidgetSize / 2.0;
    const double cy     = WidgetSize / 2.0;
    const bool   hov    = (index == m_hovered);

    // Qt 的 arcTo 角度：正方向逆时针，0° 在 3 点钟
    // 我们的起始角是从 12 点钟顺时针，需转换
    const double qtStart = -((-90.0 + index * step + GapDeg / 2.0));  // 逆时针正向
    const double qtSweep = -sweep;                                      // 顺时针为负

    QPainterPath path;
    const QRectF innerRect(cx - InnerR, cy - InnerR, InnerR * 2, InnerR * 2);
    const QRectF outerRect(cx - OuterR, cy - OuterR, OuterR * 2, OuterR * 2);

    // 从内圆弧起点开始
    const double startRad = (-90.0 + index * step + GapDeg / 2.0) * DEG2RAD;
    path.moveTo(cx + InnerR * std::cos(startRad), cy + InnerR * std::sin(startRad));

    // 外圆弧（顺时针）
    path.arcTo(outerRect, qtStart, qtSweep);

    // 内圆弧回来（逆时针，所以 sweep 取正）
    path.arcTo(innerRect, qtStart + qtSweep, -qtSweep);

    path.closeSubpath();

    // 填充
    if (hov) {
        p.setBrush(QColor(Theme::Primary));
        p.setPen(Qt::NoPen);
    } else {
        QColor sectorBg(Theme::BgPrimary);
        sectorBg.setAlpha(220);
        p.setBrush(sectorBg);
        p.setPen(QPen(QColor(Theme::Border), 1.0));
    }
    p.drawPath(path);

    // 标签位置：角平分线中点
    const double midAngleRad = startRad + (sweep / 2.0) * DEG2RAD;
    const double midR        = (InnerR + OuterR) / 2.0;
    const double lx          = cx + midR * std::cos(midAngleRad);
    const double ly          = cy + midR * std::sin(midAngleRad);

    const QColor textColor = hov ? QColor(Theme::BgPrimary) : QColor(Theme::TextSecondary);

    // 图标
    if (!m_items[index].icon.isEmpty()) {
        p.setPen(textColor);
        QFont iconFont("Segoe UI", 13);
        p.setFont(iconFont);
        const QRectF iconRect(lx - 18, ly - 20, 36, 22);
        p.drawText(iconRect, Qt::AlignCenter, m_items[index].icon);

        p.setPen(textColor);
        QFont labelFont("Segoe UI", 8);
        p.setFont(labelFont);
        const QRectF labelRect(lx - 22, ly + 2, 44, 16);
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

    // 中心圆
    const double cx = WidgetSize / 2.0;
    const double cy = WidgetSize / 2.0;
    QColor centerBg(Theme::BgPrimary);
    centerBg.setAlpha(200);
    p.setPen(QPen(QColor(Theme::Border), 1));
    p.setBrush(centerBg);
    p.drawEllipse(QPointF(cx, cy), InnerR - 4.0, InnerR - 4.0);
}

void RadialMenu::mouseMoveEvent(QMouseEvent *event) {
    const int prev = m_hovered;
    m_hovered = itemAt(event->position().toPoint());
    if (m_hovered != prev) update();
}

void RadialMenu::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) { hide(); return; }
    const int idx = itemAt(event->position().toPoint());
    hide();
    if (idx >= 0) emit triggered(idx);
}

void RadialMenu::leaveEvent(QEvent *) {
    m_hovered = -1;
    update();
}
