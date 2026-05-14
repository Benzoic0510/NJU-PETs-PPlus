//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef REMINDERSERVICE_H
#define REMINDERSERVICE_H

#include "data/ScheduleRepository.h"
#include "data/models/Schedule.h"

#include <QObject>
#include <QSet>
#include <QTimer>

class ReminderService : public QObject {
    Q_OBJECT

public:
    explicit ReminderService(QObject *parent = nullptr);

    void start(int intervalMs = 60000);
    void stop();

signals:
    void remind(const Schedule &s);

private slots:
    void scan();

private:
    ScheduleRepository m_repo;
    QTimer             m_timer;
    QSet<int>          m_notified;
};

#endif //REMINDERSERVICE_H
