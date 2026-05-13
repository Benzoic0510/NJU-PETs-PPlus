//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QDateTime>
#include <QString>

struct Schedule {
    int id = 0;
    QString title;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    int remindMins = 0;
};

#endif //SCHEDULE_H
