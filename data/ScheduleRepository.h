//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef SCHEDULEREPOSITORY_H
#define SCHEDULEREPOSITORY_H

#include "data/models/Schedule.h"

#include <QVector>

class ScheduleRepository {
public:
    int         add(const Schedule &s);
    bool        update(const Schedule &s);
    bool        remove(int id);

    Schedule    getById(int id);
    QVector<Schedule> getAll();
    QVector<Schedule> getByDateRange(const QDateTime &from, const QDateTime &to);
};

#endif //SCHEDULEREPOSITORY_H
