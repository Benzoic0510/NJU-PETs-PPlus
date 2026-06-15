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

namespace {

qreal clamp01(qreal value) {
    return qBound<qreal>(0.0, value, 1.0);
}

qreal easeOutCubic(qreal value) {
    const qreal t = 1.0 - clamp01(value);
    return 1.0 - t * t * t;
}

qreal easeInCubic(qreal value) {
    const qreal t = clamp01(value);
    return t * t * t;
}

qreal lerp(qreal from, qreal to, qreal progress) {
    return from + (to - from) * clamp01(progress);
}

} // namespace

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
    return yForDayMinute(totalMins);
}

int TimelineCanvas::yForDayMinute(int minute) const {
    return TopPad + minute * SlotH / 60;
}

void TimelineCanvas::setSchedules(const QDate &date, const QVector<Schedule> &schedules) {
    m_date = date;
    m_schedules = schedules;
    rebuildCards();
    update();
}

void TimelineCanvas::prepareEnter() {
    stopLayerAnimation();
    m_layerAnimMode = LayerAnimMode::Enter;
    m_layerAnimTime = 0.0;
    update();
}

void TimelineCanvas::playEnter() {
    stopLayerAnimation();
    m_layerAnimMode = LayerAnimMode::Enter;
    m_layerAnimTime = 0.0;
    update();

    const int totalMs = EnterLayerMs + (LayerCount - 1) * LayerDelayMs;
    m_layerAnim = new QVariantAnimation(this);
    m_layerAnim->setDuration(totalMs);
    m_layerAnim->setEasingCurve(QEasingCurve::Linear);
    m_layerAnim->setStartValue(0.0);
    m_layerAnim->setEndValue(static_cast<qreal>(totalMs));
    connect(m_layerAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_layerAnimTime = value.toReal();
        update();
    });
    connect(m_layerAnim, &QVariantAnimation::finished, this, [this]() {
        m_layerAnim = nullptr;
        m_layerAnimMode = LayerAnimMode::None;
        m_layerAnimTime = 0.0;
        update();
    });
    m_layerAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void TimelineCanvas::playExit(const std::function<void()> &finished) {
    stopLayerAnimation();
    m_layerAnimMode = LayerAnimMode::Exit;
    m_layerAnimTime = 0.0;
    update();

    const int totalMs = ExitFadeMs;
    m_layerAnim = new QVariantAnimation(this);
    m_layerAnim->setDuration(totalMs);
    m_layerAnim->setEasingCurve(QEasingCurve::Linear);
    m_layerAnim->setStartValue(0.0);
    m_layerAnim->setEndValue(static_cast<qreal>(totalMs));
    connect(m_layerAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_layerAnimTime = value.toReal();
        update();
    });
    connect(m_layerAnim, &QVariantAnimation::finished, this, [this, finished]() {
        m_layerAnim = nullptr;
        m_layerAnimMode = LayerAnimMode::Exit;
        m_layerAnimTime = static_cast<qreal>(ExitFadeMs);
        update();
        if (finished)
            finished();
    });
    m_layerAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void TimelineCanvas::rebuildCards() {
    m_cards.clear();
    const int eventAreaX = LabelW + 4;
    const int eventAreaW = width() - eventAreaX - 8;
    if (eventAreaW <= 0) return;

    QVector<int> ddlLabelYs;
    const QDateTime dayStart(m_date, QTime(0, 0));
    const QDateTime dayEnd(m_date.addDays(1), QTime(0, 0));

    for (int i = 0; i < m_schedules.size(); ++i) {
        const Schedule &s = m_schedules[i];

        if (s.isDDL) {
            if (s.startTime.date() != m_date) continue;
            const int lineY = yForTime(s.startTime.time());
            const int flagW = qMin(200, eventAreaW);
            int labelY = lineY - 12;
            for (int usedY : ddlLabelYs) {
                if (qAbs(labelY - usedY) < 24) labelY = usedY - 24;
            }
            ddlLabelYs.append(labelY);
            m_cards.append({
                QRect(eventAreaX + eventAreaW - flagW, labelY, flagW, 24),
                s.id,
                2,   // coral
                i,
                s.startTime,
                s.startTime
            });
        } else {
            const QDateTime visibleStart = s.startTime > dayStart ? s.startTime : dayStart;
            const QDateTime visibleEnd = s.endTime < dayEnd ? s.endTime : dayEnd;
            if (visibleEnd <= visibleStart) continue;

            const int startMinute = dayStart.secsTo(visibleStart) / 60;
            const int endMinute = dayStart.secsTo(visibleEnd) / 60;
            int y = yForDayMinute(startMinute);
            int h = yForDayMinute(endMinute) - y;
            if (h < 24) h = 24;
            m_cards.append({
                QRect(eventAreaX, y, eventAreaW, h),
                s.id,
                i % 4,
                i,
                visibleStart,
                visibleEnd
            });
        }
    }
}

void TimelineCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_layerAnimMode == LayerAnimMode::None) {
        drawTimelineContent(p);
        return;
    }

    if (m_layerAnimMode == LayerAnimMode::Exit) {
        p.setOpacity(layerOpacity(0));
        drawTimelineContent(p);
        return;
    }

    for (int layer = 0; layer < LayerCount; ++layer) {
        const qreal opacity = layerOpacity(layer);
        if (opacity <= 0.0)
            continue;

        p.save();
        p.setClipRect(layerRect(layer));
        p.setOpacity(opacity);
        p.translate(0.0, layerOffset(layer));
        drawTimelineContent(p);
        p.restore();
    }
}

void TimelineCanvas::drawTimelineContent(QPainter &p) {
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

    // ── 字体 ─────────────────────────────────────────────
    QFont titleFont;
    titleFont.setPointSize(9);
    titleFont.setBold(true);

    QFont metaFont;
    metaFont.setPointSize(8);

    // ── 普通事件卡片 ──────────────────────────────────────
    for (const CardInfo &card : m_cards) {
        const Schedule &s = m_schedules[card.scheduleIdx];
        if (s.isDDL) continue;

        const EventColor &c = EVENT_COLORS[card.colorIdx];

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(c.bg));
        p.drawRoundedRect(card.rect.adjusted(0, 1, 0, -1), 6, 6);

        p.setBrush(QColor(c.bar));
        p.drawRoundedRect(QRect(card.rect.left(), card.rect.top() + 1, 3, card.rect.height() - 2), 2, 2);

        const QRect textRect = card.rect.adjusted(10, 6, -6, -4);

        p.setFont(titleFont);
        p.setPen(QColor(c.fg));
        p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop,
                   p.fontMetrics().elidedText(s.title, Qt::ElideRight, textRect.width()));

        if (card.rect.height() >= 40) {
            p.setFont(metaFont);
            p.setPen(QColor(c.bar));
            QString meta = card.visibleStart.toString("HH:mm") + " – " + card.visibleEnd.toString("HH:mm");
            if (s.startTime.date() != m_date || s.endTime.date() != m_date)
                meta += "  ·  跨日";
            if (!s.location.isEmpty()) meta += "  ·  " + s.location;
            const QRect metaRect = textRect.adjusted(0, 16, 0, 0);
            p.drawText(metaRect, Qt::AlignLeft | Qt::AlignTop,
                       p.fontMetrics().elidedText(meta, Qt::ElideRight, metaRect.width()));
        }
    }

    // ── DDL 标注线（先画所有虚线，再画所有旗帜标签，避免后者被前者覆盖）────────
    QPen ddlPen(QColor(Theme::EventCoralBar), 1.5, Qt::DashLine);
    for (const CardInfo &card : m_cards) {
        const Schedule &s = m_schedules[card.scheduleIdx];
        if (!s.isDDL) continue;
        p.setPen(ddlPen);
        p.drawLine(LabelW + 4, yForTime(s.startTime.time()), w, yForTime(s.startTime.time()));
    }

    for (const CardInfo &card : m_cards) {
        const Schedule &s = m_schedules[card.scheduleIdx];
        if (!s.isDDL) continue;

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(Theme::EventCoralBg));
        p.drawRoundedRect(card.rect, 4, 4);

        p.setPen(QPen(QColor(Theme::EventCoralBar), 1));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(card.rect, 4, 4);

        p.setFont(titleFont);
        p.setPen(QColor(Theme::EventCoralFg));
        p.drawText(card.rect.adjusted(6, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft,
                   p.fontMetrics().elidedText("⚑ " + s.title, Qt::ElideRight, card.rect.width() - 10));
    }
}

void TimelineCanvas::stopLayerAnimation() {
    if (m_layerAnim) {
        m_layerAnim->stop();
        m_layerAnim->deleteLater();
        m_layerAnim = nullptr;
    }
}

QRect TimelineCanvas::layerRect(int layer) const {
    const int top = layer == 0
        ? 0
        : TopPad + layer * SlotH - 8;
    const int bottom = layer == LayerCount - 1
        ? height()
        : TopPad + (layer + 1) * SlotH - 8;
    return QRect(0, top, width(), qMax(0, bottom - top));
}

qreal TimelineCanvas::layerLocalProgress(int layer) const {
    const int spanMs = m_layerAnimMode == LayerAnimMode::Exit ? ExitFadeMs : EnterLayerMs;
    return clamp01((m_layerAnimTime - layer * LayerDelayMs) / spanMs);
}

qreal TimelineCanvas::layerOpacity(int layer) const {
    const qreal local = layerLocalProgress(layer);
    if (m_layerAnimMode == LayerAnimMode::Exit)
        return 1.0 - easeInCubic(local);
    return easeOutCubic(local);
}

qreal TimelineCanvas::layerOffset(int layer) const {
    const qreal local = layerLocalProgress(layer);
    if (m_layerAnimMode == LayerAnimMode::Exit)
        return 0.0;

    if (local < 0.68)
        return lerp(-12.0, 8.0, easeOutCubic(local / 0.68));
    return lerp(8.0, 0.0, easeOutCubic((local - 0.68) / 0.32));
}

void TimelineCanvas::mousePressEvent(QMouseEvent *event) {
    if (m_layerAnimMode != LayerAnimMode::None)
        return;

    const QPoint pos = event->position().toPoint();
    // DDL 渲染在最上层，优先命中
    for (const CardInfo &card : m_cards) {
        if (m_schedules[card.scheduleIdx].isDDL && card.rect.contains(pos)) {
            emit scheduleClicked(card.scheduleId, mapToGlobal(pos));
            return;
        }
    }
    for (const CardInfo &card : m_cards) {
        if (!m_schedules[card.scheduleIdx].isDDL && card.rect.contains(pos)) {
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
    setStyleSheet(
        "QScrollArea { background: transparent; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px 0 2px 0; }"
        "QScrollBar::handle:vertical { background: " + QString(Theme::Border) + "; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: " + QString(Theme::TextTertiary) + "; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }");
    viewport()->setStyleSheet("background: transparent;");

    m_canvas = new TimelineCanvas(this);
    m_canvas->setStyleSheet("background: transparent;");
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
    m_canvas->setSchedules(date, schedules);
    if (date != m_lastDate) {
        m_lastDate = date;
        const int y = m_canvas->yForHour(8) - 15;
        QTimer::singleShot(0, this, [this, y]() {
            verticalScrollBar()->setValue(y);
        });
    }
    // 同一天的增删改：保持滚动位置不动
}

void TimelineView::prepareEnter() {
    if (m_canvas)
        m_canvas->prepareEnter();
}

void TimelineView::playEnter() {
    if (m_canvas)
        m_canvas->playEnter();
}

void TimelineView::playExit(const std::function<void()> &finished) {
    if (m_canvas)
        m_canvas->playExit(finished);
    else if (finished)
        finished();
}
