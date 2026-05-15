//
// Created by BenzoicAcid on 2026/5/12.
//

#include "presentation/calendar/TimelineView.h"
#include "presentation/common/Theme.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

// ── 事件颜色表 ────────────────────────────────────────────────────────────────

struct EventColor {
    const char *fg, *bg, *bar;
};
static const EventColor EVENT_COLORS[4] = {
    {Theme::EventPurpleFg, Theme::EventPurpleBg, Theme::EventPurpleBar},
    {Theme::EventTealFg,   Theme::EventTealBg,   Theme::EventTealBar  },
    {Theme::EventCoralFg,  Theme::EventCoralBg,  Theme::EventCoralBar },
    {Theme::EventAmberFg,  Theme::EventAmberBg,  Theme::EventAmberBar },
};

// ── TimelineCanvas ────────────────────────────────────────────────────────────

TimelineCanvas::TimelineCanvas(QWidget *parent)
    : QWidget(parent)
{
    const int canvasH = TopPad + (EndH - StartH) * SlotH + 24;  // 24px 为 20:00 标签下方留白
    setMinimumHeight(canvasH);
    setFixedHeight(canvasH);
    setMouseTracking(true);
}

int TimelineCanvas::yForTime(const QTime &t) const {
    const int totalMins = (t.hour() - StartH) * 60 + t.minute();
    return TopPad + totalMins * SlotH / 60;
}

void TimelineCanvas::setSchedules(const QVector<Schedule> &schedules) {
    m_schedules = schedules;
    rebuildCards();
    update();
}

void TimelineCanvas::rebuildCards() {
    m_cards.clear();
    const int eventAreaX = LabelW + 4;
    const int eventAreaW = width() - eventAreaX - 8;
    if (eventAreaW <= 0) return;

    for (int i = 0; i < m_schedules.size(); ++i) {
        const Schedule &s = m_schedules[i];
        const QTime st = s.startTime.time();
        const QTime et = s.endTime.time();

        int y = yForTime(st);
        int h = yForTime(et) - y;
        if (h < 24) h = 24;  // 最小高度

        m_cards.append({
            QRect(eventAreaX, y, eventAreaW, h),
            s.id,
            i % 4,
            i
        });
    }
}

void TimelineCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();

    // ── 小时线 + 时间标签 ─────────────────────────────────
    QFont labelFont;
    labelFont.setPointSize(8);
    p.setFont(labelFont);

    for (int h = StartH; h <= EndH; ++h) {
        const int y = TopPad + (h - StartH) * SlotH;
        p.setPen(QColor(Theme::Border));
        p.drawLine(LabelW, y, w, y);
        p.setPen(QColor(Theme::TextTertiary));
        p.drawText(
            QRect(0, y - 8, LabelW - 8, 16),
            Qt::AlignRight | Qt::AlignVCenter,
            QString::number(h).rightJustified(2, '0') + ":00"
        );
    }

    // ── 事件卡片 ──────────────────────────────────────────
    QFont titleFont;
    titleFont.setPointSize(9);
    titleFont.setBold(true);
    
    QFont metaFont;
    metaFont.setPointSize(8);

    for (const CardInfo &card : m_cards) {
        const Schedule &s   = m_schedules[card.scheduleIdx];
        const EventColor &c = EVENT_COLORS[card.colorIdx];

        // 卡片背景
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(c.bg));
        p.drawRoundedRect(card.rect.adjusted(0, 1, 0, -1), 6, 6);

        // 左侧彩色竖条
        p.setBrush(QColor(c.bar));
        p.drawRoundedRect(QRect(card.rect.left(), card.rect.top() + 1, 3, card.rect.height() - 2), 2, 2);

        // 文字区域
        const QRect textRect = card.rect.adjusted(10, 6, -6, -4);

        // 标题
        p.setFont(titleFont);
        p.setPen(QColor(c.fg));
        const QString title = p.fontMetrics().elidedText(s.title, Qt::ElideRight, textRect.width());
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, title);

        // 元信息（时间 · 地点）
        if (card.rect.height() >= 40) {
            p.setFont(metaFont);
            p.setPen(QColor(c.bar));
            QString meta = s.startTime.toString("HH:mm") + " – " + s.endTime.toString("HH:mm");
            if (!s.location.isEmpty()) meta += "  ·  " + s.location;
            const QRect metaRect = textRect.adjusted(0, 16, 0, 0);
            p.drawText(metaRect, Qt::AlignLeft | Qt::AlignTop,
                       p.fontMetrics().elidedText(meta, Qt::ElideRight, metaRect.width()));
        }
    }
}

void TimelineCanvas::mousePressEvent(QMouseEvent *event) {
    const QPoint pos = event->position().toPoint();
    for (const CardInfo &card : m_cards) {
        if (card.rect.contains(pos)) {
            emit scheduleClicked(card.scheduleId, mapToGlobal(pos));
            return;
        }
    }
}

// ── TimelineView ──────────────────────────────────────────────────────────────

TimelineView::TimelineView(QWidget *parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_canvas = new TimelineCanvas(this);
    setWidget(m_canvas);
    setWidgetResizable(true);

    connect(m_canvas, &TimelineCanvas::scheduleClicked,
            this,     &TimelineView::scheduleClicked);
}

void TimelineView::scrollToTime(const QTime &t) {
    const int y = qMax(0, m_canvas->yForTime(t) - 15);
    QTimer::singleShot(0, this, [this, y]() {
        verticalScrollBar()->setValue(y);
    });
}

void TimelineView::setSchedules(const QDate &date, const QVector<Schedule> &schedules) {
    m_canvas->setSchedules(schedules);
    if (date != m_lastDate) {
        m_lastDate = date;
        const int y = m_canvas->yForHour(8) - 15;
        QTimer::singleShot(0, this, [this, y]() {
            verticalScrollBar()->setValue(y);
        });
    }
    // 同一天的增删改：保持滚动位置不动
}
