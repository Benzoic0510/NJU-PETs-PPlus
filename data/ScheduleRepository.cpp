//
// Created by BenzoicAcid on 2026/5/12.
//

#include "data/ScheduleRepository.h"
#include "data/DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDateTime>

int ScheduleRepository::add(const Schedule &s) {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(
        "INSERT INTO schedules (title, startTime, endTime, location, remindMins) "
        "VALUES (:title, :start, :end, :loc, :remind)"
    );
    q.bindValue(":title",   s.title);
    q.bindValue(":start",   s.startTime.toString(Qt::ISODate));
    q.bindValue(":end",     s.endTime.toString(Qt::ISODate));
    q.bindValue(":loc",     s.location);
    q.bindValue(":remind",  s.remindMins);

    if (!q.exec()) {
        qWarning() << "add failed:" << q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toInt();
}

bool ScheduleRepository::update(const Schedule &s) {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(
        "UPDATE schedules SET title=:title, startTime=:start, endTime=:end, "
        "location=:loc, remindMins=:remind WHERE id=:id"
    );
    q.bindValue(":title",   s.title);
    q.bindValue(":start",   s.startTime.toString(Qt::ISODate));
    q.bindValue(":end",     s.endTime.toString(Qt::ISODate));
    q.bindValue(":loc",     s.location);
    q.bindValue(":remind",  s.remindMins);
    q.bindValue(":id",      s.id);

    if (!q.exec()) {
        qWarning() << "update failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool ScheduleRepository::remove(int id) {
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("DELETE FROM schedules WHERE id=:id");
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "remove failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

Schedule ScheduleRepository::getById(int id) const {
    Schedule s;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("SELECT * FROM schedules WHERE id=:id");
    q.bindValue(":id", id);

    if (!q.exec() || !q.next()) return s;

    s.id         = q.value("id").toInt();
    s.title      = q.value("title").toString();
    s.startTime  = QDateTime::fromString(q.value("startTime").toString(), Qt::ISODate);
    s.endTime    = QDateTime::fromString(q.value("endTime").toString(), Qt::ISODate);
    s.location   = q.value("location").toString();
    s.remindMins = q.value("remindMins").toInt();
    return s;
}

QVector<Schedule> ScheduleRepository::getAll() const {
    QVector<Schedule> results;
    QSqlQuery q(DatabaseManager::instance().database());
    q.exec("SELECT * FROM schedules ORDER BY startTime ASC");

    while (q.next()) {
        Schedule s;
        s.id         = q.value("id").toInt();
        s.title      = q.value("title").toString();
        s.startTime  = QDateTime::fromString(q.value("startTime").toString(), Qt::ISODate);
        s.endTime    = QDateTime::fromString(q.value("endTime").toString(), Qt::ISODate);
        s.location   = q.value("location").toString();
        s.remindMins = q.value("remindMins").toInt();
        results.append(s);
    }
    return results;
}

QVector<Schedule> ScheduleRepository::getByDateRange(const QDateTime &from, const QDateTime &to) const {
    QVector<Schedule> results;
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare(
        "SELECT * FROM schedules WHERE startTime <= :end AND endTime >= :start "
        "ORDER BY startTime ASC"
    );
    q.bindValue(":start", from.toString(Qt::ISODate));
    q.bindValue(":end",   to.toString(Qt::ISODate));

    if (!q.exec()) return results;

    while (q.next()) {
        Schedule s;
        s.id         = q.value("id").toInt();
        s.title      = q.value("title").toString();
        s.startTime  = QDateTime::fromString(q.value("startTime").toString(), Qt::ISODate);
        s.endTime    = QDateTime::fromString(q.value("endTime").toString(), Qt::ISODate);
        s.location   = q.value("location").toString();
        s.remindMins = q.value("remindMins").toInt();
        results.append(s);
    }
    return results;
}
