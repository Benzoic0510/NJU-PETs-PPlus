//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    static DatabaseManager &instance();

    bool init();
    QSqlDatabase &database();

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    QSqlDatabase m_db;
    bool m_initialized = false;
};

#endif //DATABASEMANAGER_H
