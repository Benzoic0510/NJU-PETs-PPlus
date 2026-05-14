//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef CALENDARPANEL_H
#define CALENDARPANEL_H

#include "business/ScheduleService.h"
#include "presentation/calendar/MiniCalendar.h"
#include "presentation/calendar/TimelineView.h"

#include <QDate>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class CalendarPanel : public QWidget {
    Q_OBJECT

public:
    explicit CalendarPanel(ScheduleService *svc, QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onDateSelected(const QDate &date);
    void onScheduleClicked(int id, QPoint globalPos);
    void onAddClicked();

private:
    void setupUi();
    void loadDay(const QDate &date);
    void updateUpcoming();

    ScheduleService *m_svc      = nullptr;
    QDate            m_selDate;

    MiniCalendar    *m_miniCal      = nullptr;
    TimelineView    *m_timeline     = nullptr;
    QLabel          *m_dateLabel    = nullptr;
    QWidget         *m_upcomingList = nullptr;
};

#endif // CALENDARPANEL_H
