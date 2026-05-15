//
// Created by BenzoicAcid on 2026/5/12.
//

#include "data/AppConfig.h"

#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>

AppConfig &AppConfig::instance() {
    static AppConfig inst;
    return inst;
}

QString AppConfig::configPath() const {
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return dataDir + "/config.json";
}

void AppConfig::load() {
    QFile f(configPath());
    if (!f.open(QIODevice::ReadOnly)) return;

    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    m_petId            = obj.value("petId").toString("whale");
    m_reminderEnabled  = obj.value("reminderEnabled").toBool(true);
    m_volume           = obj.value("volume").toInt(80);
    m_apiKey           = obj.value("apiKey").toString();
}

void AppConfig::save() {
    QJsonObject obj;
    obj["petId"]            = m_petId;
    obj["reminderEnabled"]  = m_reminderEnabled;
    obj["volume"]           = m_volume;
    obj["apiKey"]           = m_apiKey;

    QFile f(configPath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson());
    } else {
        qWarning() << "AppConfig::save failed";
    }
}

QString AppConfig::petId() const            { return m_petId; }
void AppConfig::setPetId(const QString &id) { m_petId = id; }

bool AppConfig::reminderEnabled() const         { return m_reminderEnabled; }
void AppConfig::setReminderEnabled(bool on)     { m_reminderEnabled = on; }

int AppConfig::volume() const               { return m_volume; }
void AppConfig::setVolume(int vol)           { m_volume = vol; }

QString AppConfig::apiKey() const            { return m_apiKey; }
void AppConfig::setApiKey(const QString &key) { m_apiKey = key; }
