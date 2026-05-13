//
// Created by BenzoicAcid on 2026/5/12.
//

#include "data/DatabaseManager.h"

#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::init() {
    if (m_initialized) return true;

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    const QString dbPath = dataDir + "/desktopPet.db";
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Database open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS schedules ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title      TEXT    NOT NULL,"
        "  startTime  TEXT    NOT NULL,"
        "  endTime    TEXT    NOT NULL,"
        "  location   TEXT,"
        "  remindMins INTEGER DEFAULT 0"
        ")"
    );

    if (!ok) {
        qWarning() << "CREATE TABLE failed:" << query.lastError().text();
        return false;
    }

    m_initialized = true;
    return true;
}

QSqlDatabase &DatabaseManager::database() {
    return m_db;
}
