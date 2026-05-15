//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include "data/models/Schedule.h"

#include <QDate>
#include <QScrollArea>
#include <QVector>
#include <QWidget>

// 时间轴画布（内部使用，不对外暴露）
class TimelineCanvas : public QWidget {
    Q_OBJECT

public:
    explicit TimelineCanvas(QWidget *parent = nullptr);
    void setSchedules(const QVector<Schedule> &schedules);
    int  yForHour(int h) const { return TopPad + (h - StartH) * SlotH; }
    int  yForTime(const QTime &t) const;

signals:
    void scheduleClicked(int id, QPoint globalPos);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    struct CardInfo {
        QRect rect;
        int   scheduleId;
        int   colorIdx;
        int   scheduleIdx;  // index into m_schedules
    };

    void rebuildCards();

    QVector<Schedule> m_schedules;
    QVector<CardInfo> m_cards;

    static constexpr int LabelW  = 52;  // 时间标签列宽
    static constexpr int SlotH   = 52;  // 每小时高度
    static constexpr int StartH  = 0;   // 起始小时
    static constexpr int EndH    = 24;  // 最后显示的小时标签
    static constexpr int TopPad  = 8;
};

class TimelineView : public QScrollArea {
    Q_OBJECT

public:
    explicit TimelineView(QWidget *parent = nullptr);
    void setSchedules(const QDate &date, const QVector<Schedule> &schedules);
    void scrollToTime(const QTime &t);

signals:
    void scheduleClicked(int id, QPoint globalPos);

private:
    TimelineCanvas *m_canvas   = nullptr;
    QDate           m_lastDate;
};

#endif // TIMELINEVIEW_H
