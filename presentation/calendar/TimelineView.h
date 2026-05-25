//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include "data/models/Schedule.h"

#include <QDate>
#include <QScrollArea>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

#include <functional>

class QPainter;

// 时间轴画布（内部使用，不对外暴露）
class TimelineCanvas : public QWidget {
    Q_OBJECT

public:
    explicit TimelineCanvas(QWidget *parent = nullptr);
    void setSchedules(const QDate &date, const QVector<Schedule> &schedules);
    int  yForHour(int h) const { return TopPad + (h - StartH) * SlotH; }
    int  yForTime(const QTime &t) const;
    void prepareEnter();
    void playEnter();
    void playExit(const std::function<void()> &finished);

signals:
    void scheduleClicked(int id, QPoint globalPos);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    enum class LayerAnimMode {
        None,
        Enter,
        Exit
    };

    struct CardInfo {
        QRect rect;
        int   scheduleId;
        int   colorIdx;
        int   scheduleIdx;  // index into m_schedules
        QDateTime visibleStart;
        QDateTime visibleEnd;
    };

    void rebuildCards();
    void drawTimelineContent(QPainter &p);
    void stopLayerAnimation();
    QRect layerRect(int layer) const;
    qreal layerLocalProgress(int layer) const;
    qreal layerOpacity(int layer) const;
    qreal layerOffset(int layer) const;
    int  yForDayMinute(int minute) const;

    QDate m_date;
    QVector<Schedule> m_schedules;
    QVector<CardInfo> m_cards;
    QVariantAnimation *m_layerAnim = nullptr;
    LayerAnimMode m_layerAnimMode = LayerAnimMode::None;
    qreal m_layerAnimTime = 0.0;

    static constexpr int LabelW  = 52;  // 时间标签列宽
    static constexpr int SlotH   = 52;  // 每小时高度
    static constexpr int StartH  = 0;   // 起始小时
    static constexpr int EndH    = 24;  // 最后显示的小时标签
    static constexpr int TopPad  = 8;
    static constexpr int LayerCount = EndH - StartH;
    static constexpr int LayerDelayMs = 10;
    static constexpr int EnterLayerMs = 240;
    static constexpr int ExitFadeMs = 120;
};

class TimelineView : public QScrollArea {
    Q_OBJECT

public:
    explicit TimelineView(QWidget *parent = nullptr);
    void setSchedules(const QDate &date, const QVector<Schedule> &schedules);
    void scrollToTime(const QTime &t);
    void prepareEnter();
    void playEnter();
    void playExit(const std::function<void()> &finished);

signals:
    void scheduleClicked(int id, QPoint globalPos);

private:
    TimelineCanvas *m_canvas   = nullptr;
    QDate           m_lastDate;
};

#endif // TIMELINEVIEW_H
