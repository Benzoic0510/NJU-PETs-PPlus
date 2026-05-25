//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/calendar/MiniCalendar.h"
#include "presentation/common/Theme.h"

#include <QMouseEvent>
#include <QPainter>

static const QString DOW_LABELS[7] = {"日", "一", "二", "三", "四", "五", "六"};

static QString navBtnStyle() {
    return QString(
        "QPushButton {"
        "  border: 1px solid %1; border-radius: 6px;"
        "  background: transparent; font-size: 16px; color: %2;"
        "}"
        "QPushButton:hover { background: %3; }")
        .arg(Theme::Border)
        .arg(Theme::TextSecondary)
        .arg(Theme::BgSecondary);
}

MiniCalendar::MiniCalendar(QWidget *parent)
    : QWidget(parent)
{
    m_selected = QDate::currentDate();
    m_month    = QDate(m_selected.year(), m_selected.month(), 1);

    m_prevBtn = new QPushButton("‹", this);
    m_prevBtn->setFixedSize(28, 28);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setStyleSheet(navBtnStyle());

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        QString("font-size: 14px; font-weight: 500; color: %1;").arg(Theme::TextPrimary));

    m_nextBtn = new QPushButton("›", this);
    m_nextBtn->setFixedSize(28, 28);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setStyleSheet(navBtnStyle());

    connect(m_prevBtn, &QPushButton::clicked, this, &MiniCalendar::prevMonth);
    connect(m_nextBtn, &QPushButton::clicked, this, &MiniCalendar::nextMonth);

    repositionNav();
}

QSize MiniCalendar::sizeHint() const {
    return {220, NavH + DowH + 6 * CellH};
}

void MiniCalendar::setEventDates(const QSet<QDate> &dates) {
    m_eventDates = dates;
    update();
}

void MiniCalendar::setDdlDates(const QSet<QDate> &dates) {
    m_ddlDates = dates;
    update();
}

void MiniCalendar::selectDate(const QDate &date) {
    if (!date.isValid()) return;
    m_selected = date;
    m_month    = QDate(date.year(), date.month(), 1);
    repositionNav();
    update();
}

void MiniCalendar::prevMonth() {
    m_month = m_month.addMonths(-1);
    repositionNav();
    update();
}

void MiniCalendar::nextMonth() {
    m_month = m_month.addMonths(1);
    repositionNav();
    update();

}

void MiniCalendar::repositionNav() {
    const int w = width() > 0 ? width() : 220;
    m_prevBtn->move(0, (NavH - 28) / 2);
    m_nextBtn->move(w - 28, (NavH - 28) / 2);
    m_titleLabel->setGeometry(32, 0, w - 64, NavH);
    m_titleLabel->setText(
        QString("%1 年 %2 月").arg(QString::number(m_month.year()), QString::number(m_month.month())));
}

void MiniCalendar::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);
    repositionNav();
}

// col: 0=Sun, 1=Mon … 6=Sat；row: 0-5
QRectF MiniCalendar::cellRect(int col, int row) const {
    const double cw = static_cast<double>(width()) / 7.0;
    return {col * cw, static_cast<double>(NavH + DowH + row * CellH), cw, CellH};
}

QDate MiniCalendar::dateFromCell(int col, int row) const {
    // Qt dayOfWeek(): 1=Mon..7=Sun → %7 → 0=Sun..6=Sat
    const int firstDow   = m_month.dayOfWeek() % 7;
    const int dayOffset  = row * 7 + col - firstDow;
    return m_month.addDays(dayOffset);
}

void MiniCalendar::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double cw      = static_cast<double>(width()) / 7.0;
    const QDate  today   = QDate::currentDate();

    // ── 星期行 ───────────────────────────────────────────
    QFont dowFont;
    dowFont.setPointSize(8);
    p.setFont(dowFont);
    p.setPen(QColor(Theme::TextTertiary));
    for (int c = 0; c < 7; ++c) {
        const QRectF r(c * cw, NavH, cw, DowH);
        p.drawText(r, Qt::AlignCenter, DOW_LABELS[c]);
    }

    // ── 日期格 ───────────────────────────────────────────
    QFont dayFont;
    dayFont.setPointSize(9);
    p.setFont(dayFont);

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            const QDate  date        = dateFromCell(col, row);
            const QRectF r           = cellRect(col, row);
            const bool   isThisMonth = (date.month() == m_month.month() && date.year() == m_month.year());
            const bool   isToday     = (date == today);
            const bool   isSelected  = (date == m_selected);
            const bool   selectedToday = isSelected && isToday;
            const bool   hasEvent    = m_eventDates.contains(date);

            // 背景圆
            const double circR = qMin(r.width(), r.height()) / 2.0 - 2.0;
            if (selectedToday) {
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(Theme::Primary));
                p.drawEllipse(r.center(), circR, circR);
            } else if (isSelected) {
                QPen selectedPen(QColor(Theme::Primary), 1.5);
                p.setPen(selectedPen);
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(r.center(), circR, circR);
            }

            // 数字（稍微上移为小圆点留空）
            if (selectedToday)    p.setPen(QColor(Theme::BgPrimary));
            else if (isSelected)  p.setPen(QColor(Theme::Primary));
            else if (isToday)     p.setPen(QColor(Theme::Primary));
            else if (isThisMonth) p.setPen(QColor(Theme::TextSecondary));
            else                  p.setPen(QColor(Theme::TextTertiary));

            QFont cellFont = dayFont;
            cellFont.setBold(isToday);
            p.setFont(cellFont);
            p.setOpacity(isThisMonth ? 1.0 : 0.45);
            p.drawText(r.adjusted(0, -4, 0, -4), Qt::AlignCenter, QString::number(date.day()));
            p.setOpacity(1.0);

            // 事件/DDL 小圆点
            const bool hasDDL = m_ddlDates.contains(date);
            p.setPen(Qt::NoPen);
            if (hasEvent && hasDDL) {
                // 双点：左紫（事件）右红（DDL）
                p.setBrush(selectedToday ? QColor(Theme::BgPrimary) : QColor(Theme::PrimaryMid));
                p.drawEllipse(QPointF(r.center().x() - 4.0, r.bottom() - 4.0), 2.0, 2.0);
                p.setBrush(selectedToday ? QColor(Theme::BgPrimary) : QColor(Theme::EventCoralBar));
                p.drawEllipse(QPointF(r.center().x() + 4.0, r.bottom() - 4.0), 2.0, 2.0);
            } else if (hasDDL) {
                p.setBrush(selectedToday ? QColor(Theme::BgPrimary) : QColor(Theme::EventCoralBar));
                p.drawEllipse(QPointF(r.center().x(), r.bottom() - 4.0), 2.0, 2.0);
            } else if (hasEvent) {
                p.setBrush(selectedToday ? QColor(Theme::BgPrimary) : QColor(Theme::PrimaryMid));
                p.drawEllipse(QPointF(r.center().x(), r.bottom() - 4.0), 2.0, 2.0);
            }
        }
    }
}

void MiniCalendar::mousePressEvent(QMouseEvent *event) {
    const QPointF pos = event->position();
    if (pos.y() < NavH + DowH) return;

    const double cw  = static_cast<double>(width()) / 7.0;
    const int col    = static_cast<int>(pos.x() / cw);
    const int row    = static_cast<int>((pos.y() - NavH - DowH) / CellH);

    if (col < 0 || col >= 7 || row < 0 || row >= 6) return;

    const QDate date = dateFromCell(col, row);
    if (!date.isValid()) return;

    // 点击其他月份的日期 → 跳转到该月
    if (date.year() != m_month.year() || date.month() != m_month.month()) {
        m_month = QDate(date.year(), date.month(), 1);
        repositionNav();
    }

    m_selected = date;
    update();
    emit dateSelected(date);
}
