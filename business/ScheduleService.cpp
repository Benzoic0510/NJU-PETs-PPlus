//
// Created by BenzoicAcid on 2026/5/12.
//

#include "business/ScheduleService.h"

ScheduleService::ScheduleService(QObject *parent)
    : QObject(parent)
{
}

int ScheduleService::addSchedule(const Schedule &s) {
    if (hasConflict(s)) return -1;
    int id = m_repo.add(s);
    if (id > 0) emit scheduleAdded(id);
    return id;
}

bool ScheduleService::updateSchedule(const Schedule &s) {
    if (hasConflict(s)) return false;
    bool ok = m_repo.update(s);
    if (ok) emit scheduleUpdated(s.id);
    return ok;
}

bool ScheduleService::removeSchedule(int id) {
    bool ok = m_repo.remove(id);
    if (ok) emit scheduleRemoved(id);
    return ok;
}

Schedule ScheduleService::getSchedule(int id) {
    return m_repo.getById(id);
}

QVector<Schedule> ScheduleService::getAll() {
    return m_repo.getAll();
}

QVector<Schedule> ScheduleService::getByDateRange(const QDateTime &from, const QDateTime &to) {
    return m_repo.getByDateRange(from, to);
}

bool ScheduleService::hasConflict(const Schedule &s) const {
    if (s.isDDL) return false;  // DDL 不占时间片，不参与冲突检测
    const auto existing = m_repo.getAll();
    for (const auto &e : existing) {
        if (e.id == s.id) continue;
        if (e.isDDL) continue;
        if (e.startTime < s.endTime && e.endTime > s.startTime) return true;
    }
    return false;
}
