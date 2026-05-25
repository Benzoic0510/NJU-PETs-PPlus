//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef CALENDARPANEL_H
#define CALENDARPANEL_H

#include "business/NLPService.h"
#include "business/ScheduleService.h"
#include "presentation/calendar/MiniCalendar.h"
#include "presentation/calendar/ScheduleEditPanel.h"
#include "presentation/calendar/TimelineView.h"

#include <QDate>
#include <QEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class CalendarPanel : public QWidget {
    Q_OBJECT

public:
    explicit CalendarPanel(ScheduleService *svc, NLPService *nlp, QWidget *parent = nullptr, bool embedContextPanel = true);
    QWidget *contextPanel() const { return m_contextPanel; }

public slots:
    void refresh();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *e) override;

private slots:
    void onDateSelected(const QDate &date);
    void onScheduleClicked(int id, QPoint globalPos);
    void onAddClicked();

private:
    void setupUi(NLPService *nlp, bool embedContextPanel);
    void repositionEditPanel();
    void loadDay(const QDate &date);
    void updateUpcoming();

    ScheduleService    *m_svc      = nullptr;
    QDate               m_selDate;

    QWidget            *m_contextPanel = nullptr;
    MiniCalendar       *m_miniCal      = nullptr;
    TimelineView       *m_timeline     = nullptr;
    QLabel             *m_dateLabel    = nullptr;
    QWidget            *m_upcomingList = nullptr;
    QWidget            *m_rightPanel   = nullptr;
    ScheduleEditPanel  *m_editPanel    = nullptr;
};

#endif // CALENDARPANEL_H
