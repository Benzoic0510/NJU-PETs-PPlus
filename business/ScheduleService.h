//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef SCHEDULESERVICE_H
#define SCHEDULESERVICE_H

#include "data/models/Schedule.h"
#include "data/ScheduleRepository.h"

#include <QObject>
#include <QVector>

class ScheduleService : public QObject {
    Q_OBJECT

public:
    explicit ScheduleService(QObject *parent = nullptr);

    int  addSchedule(const Schedule &s);
    bool updateSchedule(const Schedule &s);
    bool removeSchedule(int id);

    Schedule          getSchedule(int id) const;
    QVector<Schedule> getAll() const;
    QVector<Schedule> getByDateRange(const QDateTime &from, const QDateTime &to) const;
    QVector<Schedule> getUpcoming(int limit = 4) const;

signals:
    void scheduleAdded(int id);
    void scheduleUpdated(int id);
    void scheduleRemoved(int id);

private:
    bool hasConflict(const Schedule &s) const;

    ScheduleRepository m_repo;
};

#endif //SCHEDULESERVICE_H
